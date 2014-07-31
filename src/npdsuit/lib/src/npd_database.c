
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_database.c
*
*
* CREATOR:
*		chenjun@autelan.com
*
* DESCRIPTION:
*		Distributed, in-memory database.
*
* DATE:
*		04/12/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.40 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "util/npd_list.h"
#include "npd_database.h"


int npd_db_debug = 0;
#define TRUE 1
#define FALSE 0

struct list_head cfg_db =
{
    &cfg_db,
    &cfg_db
};

struct list_head db_show_list =
{
    &db_show_list,
    &db_show_list
};

pthread_mutex_t cfg_db_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

pthread_mutex_t oir_lock = PTHREAD_MUTEX_INITIALIZER;

int (*platform_db_sync)(char *buffer, int len, unsigned int sync_flag, int slot_index) = NULL;
int (*platform_db_sync_complete_handler)() = NULL;
int platform_relay_flag = FALSE;

int db_table_lock_all();

int db_table_read_sync_lock_all();

int db_table_unlock_all();

int db_table_read_sync_unlock_all();

int db_table_init()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    /* the original version without _NP */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&cfg_db_lock, &attr);
    return 0;
}

/* 
* app_db_init is for the application using db to init some basic features
*     db_sync:             a function is use for sync db_table content, 
*                            buffer: the packet buffer
*                            len:    the packet len
*                            sync_flag: DB_SYNC_ALL, DB_SYNC_MCU, DB_SYNC_SERVICE
*                          if the function return code is 0, then it successs, others 
*                          the function is failed.
*     db_get_running_state: a function to get platform running state, if the return code 
*                           < LOCAL_SLAVE_READY, than db will discard all packet. and if
*                           the return cod is LOCAL_SLAVE_READY than db can only handle sync packet 
*                           type of DB_ITEM_TABLE_ADD.
*                           and will discard all other type sync packet.
*     platform_relay_flag:  if it is TRUE, then db will sync other db when recv the DB sync packet.
*/

int app_db_init(
    int (*db_sync)(char *buffer, int len, unsigned int sync_flag, int slot_index),
    int (*db_sync_complete_handler)(),
    int db_relay_flag
    )
{
    platform_db_sync = db_sync;
	platform_db_sync_complete_handler = db_sync_complete_handler;
    platform_relay_flag = db_relay_flag;
    return 0;
}

int register_db(db_table_t *db)
{
    pthread_mutex_lock(&cfg_db_lock);
    list_add_tail(&db->list, &cfg_db);
    pthread_mutex_unlock(&cfg_db_lock);
    return 0;
}

int unregister_db(db_table_t *db)
{
    pthread_mutex_lock(&cfg_db_lock);
    list_del(&db->list);
    pthread_mutex_unlock(&cfg_db_lock);
    return 0;
}

int dbtable_oir_lock()
{
    return pthread_mutex_lock(&oir_lock);
}

int dbtable_oir_unlock()
{
    return pthread_mutex_unlock(&oir_lock);
}


int wait_for_dbtable_sync()
{
    return dbtable_oir_lock();
}

long db_get(char *name, db_table_t **db)
{
    struct list_head *pos;
    *db = NULL;

    pthread_mutex_lock(&cfg_db_lock);
    list_for_each(pos, &cfg_db)   
    {
        *db = list_entry(pos, db_table_t, list);

        if (0 == strcmp(name, (*db)->name))
        {
            pthread_mutex_unlock(&cfg_db_lock);
            return DB_TABLE_RETURN_CODE_OK;
        }
    }
    pthread_mutex_unlock(&cfg_db_lock);
    *db = NULL;
    return DB_TABLE_RETURN_CODE_DB_NOT_EXIST;
}

int db_table_traversal(int(*table_handler)(db_table_t *db_t, int slot_index), int slot_index)
{
    db_table_t *db = NULL;
    struct list_head *pos;  
    db_table_read_sync_lock_all();
    list_for_each(pos, &cfg_db)   
    {
        db = list_entry(pos, db_table_t, list);
        if(table_handler)
         {
            table_handler(db, slot_index);
         }
         else
         {
            printf("need table_handler \r\n");
         }
    }
    db_table_read_sync_unlock_all();
    db = NULL;
    return 0;
}

/*非HASH index也可以通过此函数得到*/
int dbtable_index_get_by_name(char *db_name, char *table_name, char *index_name, hash_table_index_t **hash)
{
	int ret = -1;
    struct list_head *pos;
    db_index_common_t *db_index;
	db_table_t *db = NULL;
	if(db_name == NULL || table_name == NULL || index_name == NULL)
	{
		return DB_TABLE_RETURN_CODE_BAD_INPUT;
	}
	if(strlen(db_name) >= MAX_DB_NAME_LEN)
	{
		return DB_TABLE_RETURN_CODE_TABLE_NAME_LEN_ERR;
	}
	if(strlen(table_name) >= MAX_TABLE_NAME_LEN)
	{
		return DB_TABLE_RETURN_CODE_TABLE_NAME_LEN_ERR;
	}
	if(strlen(index_name) >= MAX_INDEX_NAME_LEN)
	{
		return DB_TABLE_RETURN_CODE_INDEX_NAME_LEN_ERR;
	}
	ret = db_get(table_name, &db);
	if(ret != DB_TABLE_RETURN_CODE_OK || db == NULL)
	{
		return ret;
	}
    for (pos = db->index_list.next; pos != &(db->index_list); \
            pos = pos->next)
    {
        db_index = list_entry(pos, db_index_common_t, list);
        if(db_index->index_name)
        {
            if(strcmp(index_name, db_index->index_name) == 0)
            {
				*hash = (hash_table_index_t *)db_index;
				if((*hash)->common.db != db)
				{
					return DB_TABLE_RETURN_CODE_BAD_INDEX;
				}
				else
				{
				    return DB_TABLE_RETURN_CODE_OK;
				}
            }
        }
    }
	return DB_TABLE_RETURN_CODE_INDEX_NOT_EXIST;
}

int osal_table_entry_alloc_list_init(db_table_t *table_ctrl)
{
	int i = 0;
	ENTRY_ALLOC_LIST *entry_alloc_list = NULL;
	for(i = 0; i < table_ctrl->entry_num; i++)
	{
		entry_alloc_list = malloc(sizeof(ENTRY_ALLOC_LIST));
		if(entry_alloc_list==NULL)
		{
			/*不释放了*/
			printf("%s %d. Error when allocing memory.\r\n", __func__, __LINE__);
			return i+1;
		}
		entry_alloc_list->entry_index = (table_ctrl->entry_num - i - 1);
		list_add(&(entry_alloc_list->list),&(table_ctrl->entry_alloc_list));
	}
    return 0;
}

int osal_table_entry_id_alloc(db_table_t *table_ctrl)
{
	int ret = -1;
	ENTRY_ALLOC_LIST * entry_alloc_list = NULL;
	if(list_empty(&(table_ctrl->entry_alloc_list)))
	{
		return -1;
	}

    if(table_ctrl->entry_use_num >= table_ctrl->entry_use_num_limit)
        return -1;
    table_ctrl->entry_use_num++;
	entry_alloc_list = list_entry(table_ctrl->entry_alloc_list.next, ENTRY_ALLOC_LIST, list);
	ret = entry_alloc_list->entry_index;
	list_del((struct list_head*)entry_alloc_list);
	free(entry_alloc_list);
	return ret;
}

int osal_table_entry_id_free(db_table_t *table_ctrl, int index)
{
	ENTRY_ALLOC_LIST * entry_alloc_list = NULL;
	entry_alloc_list = malloc(sizeof(ENTRY_ALLOC_LIST));
	if(entry_alloc_list == NULL)
	{
		return -1;
	}
    table_ctrl->entry_use_num--;
	entry_alloc_list->entry_index = index;
	list_add(&(entry_alloc_list->list),&(table_ctrl->entry_alloc_list));
	return 0;
}

int osal_table_entry_alloc_list_destroy(db_table_t *table_ctrl)
{
	int ret = -1;
	ENTRY_ALLOC_LIST * entry_alloc_list = NULL;
    struct list_head *pos, *n;

    
    list_for_each_safe(pos, n, &table_ctrl->entry_alloc_list)
    {
	    entry_alloc_list = list_entry(pos, ENTRY_ALLOC_LIST, list);
	    list_del(&entry_alloc_list->list);
        free(entry_alloc_list);
    }
	return ret;
}


int create_dbtable(
    char name[MAX_TABLE_NAME_LEN],
    unsigned int entry_num,
    unsigned int entry_size,
    long(*handle_update)(void *, void*),
    long(*app_handle_update)(void*, void*),    
    long(*handle_insert)(void *),
    long(*handle_delete)(void *),
    long(*app_handle_delete)(void*),
    long(*alloc_entry)(struct db_table_s *, char **, int *),
    long(*free_entry)(struct db_table_s *, char *entry),
    int (*entry_ntoh)(void *),
    int (*entry_hton)(void*),
    int sync_flag,
    db_table_t **dbtable
)
{
    pthread_mutexattr_t attr;

    (*dbtable) = malloc(sizeof(db_table_t));

    if (NULL == (*dbtable))
    {
        perror("Memory is not enough");
        exit(1);
    }

    memset(*dbtable, 0, sizeof(db_table_t));
    strncpy((*dbtable)->name, name, MAX_TABLE_NAME_LEN);
	
	if(entry_num)
	{
	    (*dbtable)->entries = malloc(entry_num*(sizeof(dbentry_common_t)));

	    if (NULL == (*dbtable)->entries)
	    {
	        perror("Memory is not enough");
	        exit(1);
	    }

	    memset((*dbtable)->entries, 0, (entry_num * sizeof(dbentry_common_t)));
	}
	
    (*dbtable)->entry_num = entry_num;
    (*dbtable)->entry_size = entry_size;
    (*dbtable)->entry_use_num_limit = entry_num;
    (*dbtable)->sync_size = entry_size;

	(*dbtable)->entry_alloc_list.next = &((*dbtable)->entry_alloc_list);
	(*dbtable)->entry_alloc_list.prev = &((*dbtable)->entry_alloc_list);
	if(osal_table_entry_alloc_list_init((*dbtable)) != 0)
	{
		free((*dbtable));
		exit(1);
	}
#ifdef _DB_MUTEX_
    pthread_mutexattr_init(&attr);
    /* the original version without _NP */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&(*dbtable)->lock, &attr);
#endif
#ifdef _DB_RWLOCK_
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&((*dbtable)->lock.wait), &attr);
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&((*dbtable)->lock.access), &attr);
    pthread_mutex_init(&((*dbtable)->lock.atomic), NULL);
	(*dbtable)->lock.readcnt = 0;
#endif
    (*dbtable)->handle_update = handle_update;
    (*dbtable)->handle_insert = handle_insert;
    (*dbtable)->handle_delete = handle_delete;
    (*dbtable)->app_handle_update = app_handle_update;
	(*dbtable)->app_handle_delete = app_handle_delete;
    (*dbtable)->entry_ntoh = entry_ntoh;
    (*dbtable)->entry_hton = entry_hton;
    (*dbtable)->sync_mode = sync_flag;
    (*dbtable)->index_list.next = &(*dbtable)->index_list;
    (*dbtable)->index_list.prev = &(*dbtable)->index_list;
    (*dbtable)->element_list.next = &(*dbtable)->element_list;
    (*dbtable)->element_list.prev = &(*dbtable)->element_list;
    register_db(*dbtable);
    return 0;
}

int create_partsync_dbtable(
    char name[MAX_TABLE_NAME_LEN],
    unsigned int entry_num,
    unsigned int entry_size,
    unsigned int sync_size,
    long(*handle_update)(void *, void*),
    long(*app_handle_update)(void*, void*),    
    long(*handle_insert)(void *),
    long(*handle_delete)(void *),
    long(*app_handle_delete)(void*),
    long(*alloc_entry)(struct db_table_s *, char **, int *),
    long(*free_entry)(struct db_table_s *, char *entry),
    int (*entry_ntoh)(void*),
    int (*entry_hton)(void*),
    int sync_flag,
    db_table_t **dbtable
)
{
    pthread_mutexattr_t attr;

    (*dbtable) = malloc(sizeof(db_table_t));

    if (NULL == (*dbtable))
    {
        perror("Memory is not enough");
        exit(1);
    }

    memset(*dbtable, 0, sizeof(db_table_t));
    strncpy((*dbtable)->name, name, MAX_TABLE_NAME_LEN);
	if(entry_num)
	{
    	(*dbtable)->entries = malloc(entry_num*(sizeof(dbentry_common_t)));

	    if (NULL == (*dbtable)->entries)
	    {
	        perror("Memory is not enough");
	        exit(1);
	    }

	    memset((*dbtable)->entries, 0, (entry_num * sizeof(dbentry_common_t)));
	}
    (*dbtable)->entry_num = entry_num;
	(*dbtable)->entry_use_num_limit = entry_num;
    (*dbtable)->entry_size = entry_size;
    (*dbtable)->sync_size = sync_size;

	(*dbtable)->entry_alloc_list.next = &((*dbtable)->entry_alloc_list);
	(*dbtable)->entry_alloc_list.prev = &((*dbtable)->entry_alloc_list);
	if(osal_table_entry_alloc_list_init((*dbtable)) != 0)
	{
		free((*dbtable));
		exit(1);
	}
#ifdef _DB_MUTEX_
    pthread_mutexattr_init(&attr);
    /* the original version without _NP */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&(*dbtable)->lock, &attr);
#endif
#ifdef _DB_RWLOCK_
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&((*dbtable)->lock.wait), &attr);
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&((*dbtable)->lock.access), &attr);
	(*dbtable)->lock.readcnt = 0;
#endif
    (*dbtable)->handle_update = handle_update;
    (*dbtable)->handle_insert = handle_insert;
    (*dbtable)->handle_delete = handle_delete;
    (*dbtable)->app_handle_update = app_handle_update;
	(*dbtable)->app_handle_delete = app_handle_delete;
    (*dbtable)->entry_ntoh = entry_ntoh;
    (*dbtable)->entry_hton = entry_hton;
    (*dbtable)->sync_mode = sync_flag;
    (*dbtable)->index_list.next = &(*dbtable)->index_list;
    (*dbtable)->index_list.prev = &(*dbtable)->index_list;
    (*dbtable)->element_list.next = &(*dbtable)->element_list;
    (*dbtable)->element_list.prev = &(*dbtable)->element_list;
    register_db(*dbtable);
    return 0;
}

int destroy_dbtable(
    db_table_t *dbtable
)
{
    int i;
    struct list_head *pos;
    db_index_common_t *db_index;

    unregister_db(dbtable);
    db_table_lock(dbtable);
    for(i = 0; i < dbtable->entry_num; i++)
    {
    	db_table_free_entry_index(dbtable, i, NULL, NULL, FALSE);
        db_table_delete_entry(dbtable, i, NULL, NULL, FALSE);
    }
    list_for_each(pos, &(dbtable->index_list))
    {
        db_index = list_entry(pos, db_index_common_t, list);
        db_index->db = NULL;
        //free(db_index);
    }
    osal_table_entry_alloc_list_destroy(dbtable);

	if( dbtable->entries )
	    free(dbtable->entries);
    db_table_unlock(dbtable);
#ifdef _DB_MUTEX_
    pthread_mutex_destroy(&dbtable->lock);
#endif
#ifdef _DB_RWLOCK_
    pthread_mutex_destroy(&(dbtable->lock.wait));
    pthread_mutex_destroy(&(dbtable->lock.access));
    pthread_mutex_destroy(&(dbtable->lock.atomic));
#endif
    free(dbtable);
    return 0;
}


#if 0
void dbtable_dump(db_table_t *db)
{
    struct list_head *pos;
	ENTRY_ALLOC_LIST * entry_alloc_list = NULL;
    printf("db name: %s\r\n", db->name);
	printf("db entry size %d, num: %d\r\n", db->entry_size, db->entry_num);
	printf("db entry memory: %x\r\n", db->entries);
	
    for (pos = db->entry_alloc_list.next; pos != &db->entry_alloc_list; \
            pos = pos->next)
        /*
            list_for_each(pos, &cfg_db)
        */
    {
        entry_alloc_list = list_entry(pos, ENTRY_ALLOC_LIST, list);

        printf("entry id:%d\r\n", entry_alloc_list->entry_index);
    }
}
int specify_db_element(
    db_table_t *db,
    unsigned int start_offset,
    unsigned int len,
    int sync_flag
)
{
    struct list_head *pos;
    db_element_t *test;
    db_element_t *element = malloc(sizeof(db_element_t));

    if (element == NULL)
        return -1;

    element->start_offset = start_offset;
    element->len = len;
    element->sync_flag = sync_flag;
    /*
    list_for_each(pos, &db->element_list)
    {
    test = list_entry(pos, db_element_t, list);
    if((test->start_offset >= start_offset)
        &&(test->len < test->start_offset-start_offset))
        goto error;
    if((test->start_offset + test->len >= start_offset)
        &&(test->len < test->end_offset-start_offset))
        goto error;
    if((test->start_offset >= start_offset)
        &&(test->len + test->start_offset <= start_offset + len))
        goto error;
    if((start_offset >= test->start_offset)
        &&(len + start_offset <= test->len + test->start_offset))
        goto error;

    }
        */
    list_add(&element->list, &db->element_list);
    return 0;
error:
    free(element);
    return -1;
}
#endif

int db_table_internal_alloc(db_table_t *db, unsigned int input_id, unsigned int *out_id)
{
    struct list_head *pos;
	ENTRY_ALLOC_LIST * entry_alloc_list = NULL;

    list_for_each(pos, &db->entry_alloc_list)
    {
        entry_alloc_list = list_entry(pos, ENTRY_ALLOC_LIST, list);
		if(entry_alloc_list->entry_index == input_id)
		{		
			db->entry_use_num++;
			*out_id = entry_alloc_list->entry_index;
			list_del((struct list_head*)entry_alloc_list);
	        free(entry_alloc_list);
	        return 0;
		}
    }
	return -1;
}

int db_table_internal_alloc_after(db_table_t *db, unsigned int input_id, unsigned int *out_id)
{
    struct list_head *pos;
	ENTRY_ALLOC_LIST * entry_alloc_list = NULL;

    list_for_each(pos, &db->entry_alloc_list)
    {
        entry_alloc_list = list_entry(pos, ENTRY_ALLOC_LIST, list);
		if(entry_alloc_list->entry_index >= input_id)
		{		
			db->entry_use_num++;
			*out_id = entry_alloc_list->entry_index;
			list_del((struct list_head*)entry_alloc_list);
	        free(entry_alloc_list);
	        return 0;
		}
    }
	return -1;
}

int db_table_internal_free_order(db_table_t *db, unsigned int index)
{	
	struct list_head *pos;
	ENTRY_ALLOC_LIST * entry_alloc_list = NULL, *entry_list = NULL;

	entry_alloc_list = malloc(sizeof(ENTRY_ALLOC_LIST));
	if(entry_alloc_list == NULL)
	{
		return -1;
	}

	list_for_each(pos, &db->entry_alloc_list)
    {
        entry_list = list_entry(pos, ENTRY_ALLOC_LIST, list);
		if(entry_list->entry_index > index)
		{	
			break;
		}
	}
		
	db->entry_use_num--;
	entry_alloc_list->entry_index = index;		
	if(pos == &db->entry_alloc_list)
		list_add_tail(&(entry_alloc_list->list), &(db->entry_alloc_list));
	else if(pos->prev == &db->entry_alloc_list)
		list_add(&(entry_alloc_list->list),&(db->entry_alloc_list));
	else
		list_add(&(entry_alloc_list->list),(pos->prev));
		
	return 0;
}

int db_table_entry_num_limit(db_table_t *db, unsigned int num)
{
	return 0;
}

int db_table_sync_get_entry(
    db_table_t *db,
    unsigned int entry_id,
    void *data)
{
    dbentry_common_t *entry;

    if (data == NULL)
        return -1;

    if (entry_id >= db->entry_num)
        return -1;
    entry = &(db->entries[entry_id]);

    if (entry->flags&DB_ENTRY_EXIST && entry->real_data)
        memcpy(data, entry->real_data, db->sync_size);
    else
        return -1;

    return 0;
}


int db_table_get_entry(
    db_table_t *db,
    unsigned int entry_id,
    void *data)
{
    dbentry_common_t *entry;

    if (data == NULL)
        return -1;

    if (entry_id >= db->entry_num)
        return -1;
    entry = &(db->entries[entry_id]);

    if (entry->flags&DB_ENTRY_EXIST && entry->real_data)
        memcpy(data, entry->real_data, db->entry_size);
    else
        return -1;

    return 0;
}

int db_table_entry(
    db_table_t *db,
    unsigned int entry_id,
    void *data)
{
    dbentry_common_t *entry;

    if (data == NULL)
        return -1;

    if (entry_id >= db->entry_num)
        return -1;
    entry = &(db->entries[entry_id]);

    if (entry->flags&DB_ENTRY_EXIST && entry->real_data)
        data = entry->real_data;
    else
        return -1;

    return 0;
}

int db_table_lock(
    db_table_t *db
)
{
#ifdef _DB_MUTEX_
    pthread_mutex_lock(&db->lock);
#endif
#ifdef _DB_RWLOCK_
    pthread_mutex_lock(&(db->lock.wait));
    pthread_mutex_lock(&(db->lock.access));
#endif
	return 0;
}

int db_table_unlock(
    db_table_t *db
)
{
#ifdef _DB_MUTEX_
    pthread_mutex_unlock(&db->lock);
#endif
#ifdef _DB_RWLOCK_
    pthread_mutex_unlock(&(db->lock.access));
    pthread_mutex_unlock(&(db->lock.wait));
#endif
	return 0;
}

int db_table_read_lock(
    db_table_t *db
)
{
#ifdef _DB_MUTEX_
    pthread_mutex_lock(&db->lock);
#endif
#ifdef _DB_RWLOCK_
    int readcounter = 0;
    pthread_mutex_lock(&(db->lock.wait));
	pthread_mutex_lock(&(db->lock.atomic));
	readcounter = db->lock.readcnt;
	db->lock.readcnt++;
	pthread_mutex_unlock(&(db->lock.atomic));
    if (readcounter == 0)
    {
        pthread_mutex_lock(&(db->lock.access));
    }
    pthread_mutex_unlock(&(db->lock.wait));
#endif
	return 0;
}

int db_table_read_unlock(
    db_table_t *db
)
{
#ifdef _DB_MUTEX_
    pthread_mutex_unlock(&db->lock);
#endif
#ifdef _DB_RWLOCK_
    int readcounter = 0;
	pthread_mutex_lock(&(db->lock.atomic));
	readcounter = db->lock.readcnt;
	db->lock.readcnt--;
	pthread_mutex_unlock(&(db->lock.atomic));
    if (readcounter == 1)
	{
        pthread_mutex_unlock(&(db->lock.access));
    }
#endif
	return 0;
}

int dbtable_hash_lock(hash_table_index_t *hash)
{
    if(NULL == hash)
        return -1;
    if(!hash->common.db)
        return -1;
    db_table_lock(hash->common.db);
	return 0;
}

int dbtable_hash_unlock(hash_table_index_t *hash)
{
    if(NULL == hash)
        return -1;
     if(!hash->common.db)
        return -1;
    db_table_unlock(hash->common.db);
	return 0;
}

int dbtable_sequence_lock(sequence_table_index_t *sequence)
{
    if(NULL == sequence)
        return -1;
    if(!sequence->common.db)
        return -1;
    db_table_lock(sequence->common.db);
	return 0;
}

int dbtable_sequence_unlock(sequence_table_index_t *sequence)
{
    if(NULL == sequence)
        return -1;
    if(!sequence->common.db)
        return -1;
    db_table_unlock(sequence->common.db);
	return 0;
}

int dbtable_array_lock(array_table_index_t *array)
{
    if(NULL == array)
        return -1;
    if(!array->db)
        return -1;
    db_table_lock(array->db);
	return 0;
}

int dbtable_array_unlock(array_table_index_t *array)
{
    if(NULL == array)
        return -1;
    if(!array->db)
        return -1;
    db_table_unlock(array->db);
	return 0;
}

int db_table_lock_all()
{
    db_table_t *db = NULL;
    struct list_head *pos;  
    list_for_each(pos, &cfg_db)   
    {
        db = list_entry(pos, db_table_t, list);
        db_table_lock(db);
    }
	return 0;
}

int db_table_read_sync_lock_all()
{
    db_table_t *db = NULL;
    struct list_head *pos;  
    list_for_each(pos, &cfg_db)   
    {
        db = list_entry(pos, db_table_t, list);
		if(db->sync_mode != DB_SYNC_NONE)
		{
            db_table_read_lock(db);
		}
    }
	return 0;
}

int db_table_unlock_all()
{
    db_table_t *db = NULL;
    struct list_head *pos;  
    list_for_each_prev(pos, &cfg_db)   
    {
        db = list_entry(pos, db_table_t, list);
        db_table_unlock(db);
    }
	return 0;
}

int db_table_read_sync_unlock_all()
{
    db_table_t *db = NULL;
    struct list_head *pos;  
    list_for_each(pos, &cfg_db)   
    {
        db = list_entry(pos, db_table_t, list);
		if(db->sync_mode != DB_SYNC_NONE)
		{
            db_table_read_unlock(db);
		}
    }
	return 0;
}

long dbtable_sync_data_insert_piece(db_table_t *table, unsigned int start_id, unsigned int num, int slot_index)
{
    npd_sync_msg_header_t *header;
    int op_ret = -1;
    int i;
    char *buffer = malloc(sizeof(*header)+table->entry_size*num);
    void *data;

    if (NULL == buffer)
        return op_ret;

    memset(buffer, 0, sizeof(*header)+table->entry_size*num);
    header = (npd_sync_msg_header_t*)buffer;
    header->type = NPD_SYNC_TYPE_ENTRY;
    header->op = NPD_SYNC_OP_ITEM_ADD;
    header->entry_num = num;
    header->start_index = start_id;
    header->total_len = sizeof(*header)+table->entry_size*num;
    header->entry_len = table->entry_size;
    header->sync_len = table->sync_size;
    strncpy(header->name, table->name, MAX_TABLE_NAME_LEN);
    data = (void*)(header+1);

    for (i = 0; i < num; i++)
    {
        db_table_sync_get_entry(table, start_id+i, data);
		if(table->entry_hton)
			table->entry_hton(data);
        data = (void*)((char*)data + table->entry_size);
    }
    
    if(NULL != platform_db_sync)
        op_ret = (*platform_db_sync)(buffer, header->total_len, table->sync_mode, slot_index);

    if (0 == op_ret)
    {
        for (i = 0; i < num; i++)
            db_table_synced_entry(table, start_id+i, DB_MASTER_SLAVE_SYNCED);
    }
    free(buffer);
    return 0;
}


long dbtable_sync_data_insert(db_table_t *table, unsigned int start_id, unsigned int num, int slot_index)
{
    int i;
    int s_id = start_id;
    int send_num = (MAX_SEND_SIZE-sizeof(npd_sync_msg_header_t))/table->entry_size;
    int send_count = (num/send_num)+1;
    //printf("num:%d,send_num:%d,send_count:%d \r\n",num,send_num,send_count);
    for( i=0;i< send_count;i++)
    {
        if(i == send_count-1)
        {
            send_num = num - (send_count-1)*send_num;
        }
        //printf("the send_num is:%d",send_num);
        dbtable_sync_data_insert_piece(table, s_id, send_num, slot_index);
        s_id += send_num;
    }
    return 0;
 
}


long dbtable_sync_data_delete(db_table_t *table, unsigned int start_id, unsigned int num, int slot_index)
{
    npd_sync_msg_header_t *header;
    int op_ret = -1;
    int i;
    char *buffer = malloc(sizeof(*header)+table->entry_size*num);
    void *data;

    if (NULL == buffer)
        return op_ret;

    memset(buffer, 0, sizeof(*header)+table->entry_size*num);
    header = (npd_sync_msg_header_t*)buffer;
    header->type = NPD_SYNC_TYPE_ENTRY;
    header->op = NPD_SYNC_OP_ITEM_DEL;
    header->entry_num = num;
    header->start_index = start_id;
    header->total_len = sizeof(*header)+table->entry_size*num;
    header->entry_len = table->entry_size;
    header->sync_len = table->sync_size;
    strncpy(header->name, table->name, MAX_TABLE_NAME_LEN);
    data = (void*)(header+1);

    for (i = 0; i < num; i++)
    {
        db_table_sync_get_entry(table, start_id+i, data);		
		if(table->entry_hton)
			table->entry_hton(data);
        data = (void*)((char*)data + table->entry_size);
    }

    if(NULL != platform_db_sync)
        op_ret = (*platform_db_sync)(buffer, header->total_len, table->sync_mode, slot_index);

    if (0 == op_ret)
    {
        for (i = 0; i < num; i++)
            db_table_synced_entry(table, start_id+i, DB_MASTER_SLAVE_SYNCED);
    }
    free(buffer);
    return 0;
}

long dbtable_sync_data_update(db_table_t *table, unsigned int start_id, unsigned int num, int slot_index)
{
    npd_sync_msg_header_t *header;
    int op_ret = -1;
    int i;
    char *buffer = malloc(sizeof(*header)+table->entry_size*num);
    void *data;

    if (NULL == buffer)
        return op_ret;

    memset(buffer, 0, sizeof(*header)+table->entry_size*num);
    header = (npd_sync_msg_header_t*)buffer;
    header->type = NPD_SYNC_TYPE_ENTRY;
    header->op = NPD_SYNC_OP_ITEM_UPDATE;
    header->entry_num = num;
    header->start_index = start_id;
    header->total_len = sizeof(*header)+table->entry_size*num;
    header->entry_len = table->entry_size;
    header->sync_len = table->sync_size;
    strncpy(header->name, table->name, MAX_TABLE_NAME_LEN);
    data = (void*)(header+1);

    for (i = 0; i < num; i++)
    {
        db_table_sync_get_entry(table, start_id+i, data);		
		if(table->entry_hton)
			table->entry_hton(data);
        data = (void*)((char*)data + table->entry_size);
    }

    if(NULL != platform_db_sync)
        op_ret = (*platform_db_sync)(buffer, header->total_len, table->sync_mode, slot_index);

    if (0 == op_ret)
    {
        for (i = 0; i < num; i++)
            db_table_synced_entry(table, start_id+i, DB_MASTER_SLAVE_SYNCED);
    }
    free(buffer);
    return 0;
}

int dbtable_sync_data_all(db_table_t *db, int slot_index)
{
    int start_id = -1;
    int num = 0;
    int i;
    dbentry_common_t *entry;
	if((db->sync_mode & DB_SYNC_NONE))
	{
	    return 0;
	}
    for (i = 0; i < db->entry_num; i++)
    {
        entry = db->entries + i;
        if (entry->flags & DB_ENTRY_EXIST)
        {
            if (start_id == -1)
                start_id = i;
            num++;               
            if(start_id+num == db->entry_num)
            {
                dbtable_sync_data_insert(db, start_id, num, slot_index);
                break;
            }
            
        }
        else
        {
            if (start_id != -1)
            {
                dbtable_sync_data_insert(db, start_id, num, slot_index);
                start_id = -1;
                num = 0;
            }
        }
        
    }

    return 0;
}

int dbtable_slot_online_insert_complete(int slot_index)
{
    npd_sync_msg_header_t *header;
    int op_ret = -1;
    char *buffer = malloc(sizeof(npd_sync_msg_header_t));

    if (NULL == buffer)
        return op_ret;
    memset(buffer, 0, sizeof(npd_sync_msg_header_t));
    header = (npd_sync_msg_header_t*)buffer;
    header->type = NPD_SYNC_TYPE_TBL;
    header->op = NPD_SYNC_OP_COMPLETE;
    header->entry_num = 0;
    header->start_index = 0;
    header->total_len = sizeof(npd_sync_msg_header_t);
    strncpy(header->name, "COMPLETED", MAX_TABLE_NAME_LEN);

    if(NULL != platform_db_sync)
        op_ret = (*platform_db_sync)(buffer, header->total_len, DB_SYNC_ALL, slot_index);

    free(buffer);
    return 0;
}

int dbtable_slot_online_insert(int slot_index)
{
    if (platform_relay_flag)
    {
        db_table_traversal(dbtable_sync_data_all, slot_index);
		dbtable_slot_online_insert_complete(slot_index);
    }

    return 0;
}

int db_table_insert_entry(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
)
{
    dbentry_common_t *entry;
    struct list_head *pos;
    db_index_common_t *db_index;
	if(entry_id >= db->entry_num)
	{
		return -1;
	}
    entry = &(db->entries[entry_id]);
	if(entry->real_data == NULL)
	{
		entry->real_data = malloc(db->entry_size);
        if(entry->real_data == NULL)
    	{
    		printf("%s %d. Error when allocing memory.\r\n", __func__, __LINE__);
    		return -1;
        }
        memset(entry->real_data, 0, db->entry_size);
	}
    memcpy(entry->real_data, data, db->entry_size);
    entry->flags |= DB_ENTRY_EXIST;
	entry->modify_counter++;
    list_for_each(pos, &(db->index_list))
    {
        db_index = list_entry(pos, db_index_common_t, list);
        if (db_index != orig_index && db_index->index_insert)
        {
            (*db_index->index_insert)(db_index, data, entry_id);            
        }
    }
#ifdef HAVE_CHASSIS_SUPPORT
    if(db->sync_mode != DB_SYNC_NONE)
    {
    	if (local_flag || platform_relay_flag)
        	dbtable_sync_data_insert(db, entry_id, 1, -1);
    }
#endif
    return 0;
}

/*only delete all index relating entry*/
int db_table_free_entry_index(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
)
{
    dbentry_common_t *entry;
    struct list_head *pos;
    db_index_common_t *db_index;
	
    if (entry_id >= db->entry_num)
        return -1;
	
    entry = &(db->entries[entry_id]);

    if(!(entry->flags & DB_ENTRY_EXIST))
        return 0;
    list_for_each(pos, &db->index_list)
    {
        db_index = list_entry(pos, db_index_common_t, list);

        if (db_index != orig_index && db_index->index_delete)
        {
            (*db_index->index_delete)(db_index, entry->real_data, entry_id);
        }
    }
    return 0;
}


int db_table_delete_entry(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
)
{
    dbentry_common_t *entry;
	
    if (entry_id >= db->entry_num)
        return -1;
	
    entry = &(db->entries[entry_id]);

    if(!(entry->flags & DB_ENTRY_EXIST))
        return 0;
#ifdef HAVE_CHASSIS_SUPPORT
    if(db->sync_mode != DB_SYNC_NONE)
    {
        if (local_flag || platform_relay_flag)
            dbtable_sync_data_delete(db, entry_id, 1, -1);
    }
#endif
    if(entry->flags & DB_DELETE_NOT_SYNCED)
        return 0;
    
    entry->flags &= ~DB_ENTRY_EXIST;

	if(entry->real_data != NULL)
	{
		free(entry->real_data);
		entry->real_data = NULL;
	}

	db_table_internal_free_order(db, entry_id);
    //osal_table_entry_id_free(db, entry_id);
    return 0;
}

int db_table_update_entry(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
)
{
    dbentry_common_t *entry;
    struct list_head *pos;
    db_index_common_t *db_index;
    db_element_t *element;
    int sync_remote = TRUE;

	if(entry_id >= db->entry_num)
	{
		return -1;
	}
    entry = &(db->entries[entry_id]);
	if(entry->real_data == NULL)
	{
		return -1;
	}
	/*(有待实现)
	在此加入基于entry的互斥判断(数据一致性保证)
	判断规则:
	1)是否有远程操作此ENTRY(由于DB有互斥锁，所以这项可以忽略)
	2)entry 写操作(包括insert, delete, update)次数的计数(每次写操作加1，计数大，优先)
	3)写操作发起者的优先级
	*/
    for (pos = db->index_list.next; pos != &(db->index_list); \
            pos = pos->next)
    {
        db_index = list_entry(pos, db_index_common_t, list);

        if (db_index != orig_index && db_index->index_delete)
        {
            (*db_index->index_delete)(db_index, entry->real_data, entry_id);
        }
    }

    if (list_empty(&db->element_list)||local_flag)
    {
#ifdef HAVE_CHASSIS_SUPPORT		
        if(0 == memcmp(data, entry->real_data, db->sync_size))
            sync_remote = FALSE;
#endif		
        memcpy(entry->real_data, data, db->entry_size);
    }
    else
    {
        list_for_each(pos, &db->element_list)
        {
            element = list_entry(pos, db_element_t, list);

            if (element->sync_flag != DB_SYNC_NONE)
            {
                memcpy((char*)entry->real_data+element->start_offset,
                       (char*)data+element->start_offset,
                       element->len);
            }
        }
    }

    list_for_each(pos, &db->index_list)
    {
        db_index = list_entry(pos, db_index_common_t, list);

        if (db_index != orig_index && db_index->index_insert)
        {
            (*db_index->index_insert)(db_index, data, entry_id);
        }
    }
#ifdef HAVE_CHASSIS_SUPPORT
    if(db->sync_mode != DB_SYNC_NONE)
    {
	    if (sync_remote && (local_flag || platform_relay_flag))
	        dbtable_sync_data_update(db, entry_id, 1, -1);	
    }
#endif
    return 0;
}

int db_table_synced_entry(
    db_table_t *db,
    unsigned int entry_id,
    unsigned int flag
)
{
    dbentry_common_t *entry;
	
    if (entry_id >= db->entry_num)
        return -1;
	
    entry = &(db->entries[entry_id]);
    entry->flags |= flag;
    return 0;
}

int db_table_unsynced_entry(
    db_table_t *db,
    unsigned int entry_id,
    unsigned int flag
)
{
    dbentry_common_t *entry;
	
    if (entry_id >= db->entry_num)
        return -1;
	
    entry = &(db->entries[entry_id]);
    entry->flags &= ~flag;
    return 0;
}

int dbtable_index_entry_size(
    db_index_common_t *db_index,
    int *size
    )
{
    if(NULL == db_index)
    {
        *size = 0;
        return -1;
    }
            
    *size = db_index->db->entry_size;
    return 0;
}

int dbtable_index_entry_num(
    db_index_common_t *db_index,
    int *num
    )
{
    if(NULL == db_index)
    {
        *num = 0;
        return -1;
    }
    *num = db_index->db->entry_num;
    return 0;
}
int dbtable_hash_internal_insert(
    db_index_common_t *common,
    void *data,
    unsigned int entry_id
);
int dbtable_hash_internal_delete(
    db_index_common_t *common,
    void *data,
    unsigned int entry_id
);

int dbtable_create_hash_index(
	char *index_name,
    db_table_t *db,
    unsigned int hash_bucket_size,
    unsigned int (*hash_key)(void *),
    unsigned int (*hash_cmp)(void *, void *),
    hash_table_index_t **hash
)
{
    *hash = malloc(sizeof(hash_table_index_t));

    if (NULL == *hash)
    {
        *hash = NULL;
        return -1;
    }
    memset(*hash, 0, sizeof(hash_table_index_t));
    (*hash)->common.db = db;
	strncpy((*hash)->common.index_name, index_name, 31);
	(*hash)->common.index_type = DB_TABLE_INDEX_HASH;
    (*hash)->common.index_insert = dbtable_hash_internal_insert;
    (*hash)->common.index_delete = dbtable_hash_internal_delete;
    list_add(&((*hash)->common.list), &db->index_list);
    (*hash)->hash_index.size = hash_bucket_size;
    (*hash)->hash_index.count = 0;
    (*hash)->hash_index.index = malloc(hash_bucket_size*(sizeof(dbhash_bucket_t*)));

    if ((*hash)->hash_index.index == NULL)
    {
        free(*hash);
        *hash = NULL;
        return -1;
    }

    memset((*hash)->hash_index.index, 0, hash_bucket_size*sizeof(dbhash_bucket_t*));
    (*hash)->hash_index.hash_key = hash_key;
    (*hash)->hash_index.hash_cmp = hash_cmp;
    return 0;
}
/* allocate new hash bucket */
struct dbhash_bucket * dbhash_bucket_new
(
    unsigned int index
)
{
    struct dbhash_bucket * bucket = NULL;

    bucket = malloc(sizeof(struct dbhash_bucket));

    if (!bucket)
    {
        return NULL;
    }

    bucket->index = index;
    bucket->next = NULL;
    bucket->pre = NULL;

    return bucket;
}

int dbtable_hash_search
(
    hash_table_index_t *hash,
    void *data,
    unsigned int (*compFunc)(void*,void*),
    void *retData
)
{
    unsigned int    key1;
    struct dbhash_bucket **mp = NULL;
    struct dbhash_bucket **index1 = NULL;

    if ((NULL == hash)||(NULL == data))
    {
        return DB_TABLE_RETURN_CODE_BAD_INPUT;
    }
    if(!hash->common.db)
        return -1;
    char *cmpdata = malloc(hash->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
    }

    db_table_read_lock(hash->common.db);

    if (NULL != hash->hash_index.hash_key)
    {
        key1 = (*hash->hash_index.hash_key)(data);
        index1 = hash->hash_index.index;

        if (key1 >= hash->hash_index.size)
        {
            db_table_read_unlock(hash->common.db);
            free(cmpdata);
            return DB_TABLE_RETURN_CODE_KEY_OVERLAP;
        }

        {
            for (mp = &(index1[key1]); *mp != NULL; mp = &((*mp)->next))
            {
                db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);

                if (((*hash->hash_index.hash_cmp)(data, cmpdata) == TRUE))
                {
                    memcpy(retData, cmpdata, hash->common.db->entry_size);
                    /*unlock mutex*/
                    db_table_read_unlock(hash->common.db);
                    free(cmpdata);
                    return 0;
                }
				else
				{
					if(compFunc != NULL)
					{
                        if (((*compFunc)(data, cmpdata) == TRUE))
                        {
                            memcpy(retData, cmpdata, hash->common.db->entry_size);
                            /*unlock mutex*/
                            db_table_read_unlock(hash->common.db);
                            free(cmpdata);
                            return 0;
                        }
					}
				}
            } /* FOR (...) */
        }
    }
    db_table_read_unlock(hash->common.db);
    free(cmpdata);
    return DB_TABLE_RETURN_CODE_ENTRY_NOT_EXIST;
}

int dbtable_hash_insert(
    hash_table_index_t *hash,
    void *data
)
{
    int ret = -1;
    unsigned int    key1 = 0;
    struct dbhash_bucket *bucket = NULL,**mp = NULL;
    struct dbhash_bucket ** index1 = NULL;
    int entry_id;
    int *count = NULL;
    char *cmpdata = NULL;

    if ((NULL == hash)||(NULL == data))
    {
        return -1;
    }

    if(!hash->common.db)
        return -1;
	
    cmpdata = malloc(hash->common.db->entry_size);;
    if (NULL == cmpdata)
    {
        return -1;
    }

    /*lock mutex*/
    db_table_lock(hash->common.db);
    count = &(hash->hash_index.count);
    (*count)++;

    if (NULL != hash->hash_index.hash_key)
    {
        key1 = (*hash->hash_index.hash_key)(data);
        index1 = hash->hash_index.index;
        if (key1 >= hash->hash_index.size)
        {
            db_table_unlock(hash->common.db);
            (*count)--;
            free(cmpdata);
            return -1;
        }

        {
            for (mp = &(index1[ key1 ]); *mp != NULL; mp = &((*mp)->next))
            {
                db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);

                if ((*hash->hash_index.hash_cmp)(data, cmpdata) == TRUE)
                {
                    db_table_update_entry(hash->common.db, (*mp)->index, data,
                                          (db_index_common_t*)hash, TRUE);
                    (*count)--;
					ret = 0;
					if(hash->common.db->handle_update)
					{
                        ret = (*hash->common.db->handle_update)(data, cmpdata);
					}

                    if (0 == ret)
                    {
                        db_table_synced_entry(hash->common.db, (*mp)->index, DB_HW_SW_SYNCED);
                        db_table_unlock(hash->common.db);
                    }
                    else
                    {
                        printf("Hash %x TABLE %s update unsuccess\r\n", (unsigned int)hash, hash->common.db->name);
                        db_table_unsynced_entry(hash->common.db, (*mp)->index, DB_HW_SW_SYNCED);
                        db_table_unlock(hash->common.db);
                    }
                    free(cmpdata);
                    return 0;
                }
            } /* FOR (...) */

            entry_id = osal_table_entry_id_alloc(hash->common.db);
        	if(entry_id == -1)
        	{
        		db_table_unlock(hash->common.db);
				(*count)--;
                free(cmpdata);
        		return -1;
        	}
            bucket = dbhash_bucket_new(entry_id);
        	if(bucket == NULL)
        	{
        		osal_table_entry_id_free(hash->common.db, entry_id);
        		db_table_unlock(hash->common.db);
				(*count)--;
                free(cmpdata);
        		return -1;
        	}
            (*mp) = bucket;
        }
    }

    if (!(hash->hash_index.hash_key))
    {
        (*count)--;
        db_table_unlock(hash->common.db);
        free(cmpdata);
        return -1;
    }

    db_table_insert_entry(hash->common.db, entry_id, data,
                          (db_index_common_t*)hash, TRUE);
    /*unlock mutex*/
    free(cmpdata);
	ret = 0;
	if(hash->common.db->handle_insert)
	{
        ret = (*hash->common.db->handle_insert)(data);
	}

    if (0 == ret)
    {
        db_table_synced_entry(hash->common.db, entry_id, DB_HW_SW_SYNCED);
    }
    else
    {
        printf("Hash %x TABLE %s insert unsuccess\r\n", (unsigned int)hash, hash->common.db->name);
    }
    db_table_unlock(hash->common.db);

    return 0;
}

int dbtable_hash_insert_seq(
    hash_table_index_t *hash,
    void *data,
    int (*list_prev)(void*, void*)
)
{
    int ret = -1;
    unsigned int    key1 = 0;
    struct dbhash_bucket *bucket = NULL,**mp = NULL;
    struct dbhash_bucket ** index1 = NULL;
    int entry_id;
    int * count = NULL;
    char *cmpdata = malloc(hash->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }

    if ((NULL == hash)||(NULL == data))
    {
        free(cmpdata);
        return -1;
    }
    if(!hash->common.db)
        return -1;

    /*lock mutex*/
    db_table_lock(hash->common.db);
    count = &(hash->hash_index.count);
    (*count)++;

    if (NULL != hash->hash_index.hash_key)
    {
        key1 = (*hash->hash_index.hash_key)(data);
        index1 = hash->hash_index.index;
        if (key1 >= hash->hash_index.size)
        {
            db_table_unlock(hash->common.db);
            (*count)--;
            free(cmpdata);
            return -1;
        }

        {
            for (mp = &(index1[ key1 ]); *mp != NULL; mp = &((*mp)->next))
            {
                db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);

                if ((*hash->hash_index.hash_cmp)(data, cmpdata) == TRUE)
                {
                    db_table_update_entry(hash->common.db, (*mp)->index, data,
                                          (db_index_common_t*)hash, TRUE);
                    (*count)--;
					ret = 0;
					if(hash->common.db->handle_update)
					{
                        ret = (*hash->common.db->handle_update)(data, cmpdata);
					}

                    if (0 == ret)
                    {
                        db_table_synced_entry(hash->common.db, (*mp)->index, DB_HW_SW_SYNCED);
                        db_table_unlock(hash->common.db);
                    }
                    else
                    {
                        printf("Hash %x TABLE %s update unsuccess\r\n", (unsigned int)hash, hash->common.db->name);
                        db_table_unsynced_entry(hash->common.db, (*mp)->index, DB_HW_SW_SYNCED);
                        db_table_unlock(hash->common.db);
                    }

                    free(cmpdata);
                    return 0;
                }
                if(list_prev)
                {
                    if(TRUE == (*list_prev)(data, cmpdata))
                    {
                        break;
                    }
                }
            } /* FOR (...) */

            entry_id = osal_table_entry_id_alloc(hash->common.db);
        	if(entry_id == -1)
        	{
        		db_table_unlock(hash->common.db);
                (*count)--;
                free(cmpdata);
        		return -1;
        	}
            bucket = dbhash_bucket_new(entry_id);
        	if(bucket == NULL)
        	{
        		osal_table_entry_id_free(hash->common.db, entry_id);
        		db_table_unlock(hash->common.db);
                (*count)--;
                free(cmpdata);
        		return -1;
        	}
            bucket->next = (*mp);
            (*mp) = bucket;
        }
    }

    if (!(hash->hash_index.hash_key))
    {
        (*count)--;
        db_table_unlock(hash->common.db);
        free(cmpdata);
        return -1;
    }

    db_table_insert_entry(hash->common.db, entry_id, data,
                          (db_index_common_t*)hash, TRUE);
    /*unlock mutex*/
    free(cmpdata);
	ret = 0;
	if(hash->common.db->handle_insert)
	{
        ret = (*hash->common.db->handle_insert)(data);
	}

    if (0 == ret)
    {
        db_table_synced_entry(hash->common.db, entry_id, DB_HW_SW_SYNCED);
    }
    else
    {
        printf("Hash %x TABLE %s insert unsuccess\r\n", (unsigned int)hash, hash->common.db->name);
    }
    db_table_unlock(hash->common.db);

    return 0;
}



int dbtable_hash_internal_insert(
    db_index_common_t *common,
    void *data,
    unsigned int entry_id
)
{
    unsigned int    key1 = 0;
    hash_table_index_t *hash = (hash_table_index_t*)common;
    struct dbhash_bucket *bucket = NULL,**mp = NULL;
    struct dbhash_bucket ** index1 = NULL;
    char *cmpdata = malloc(hash->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }

    if (NULL == hash)
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;
    bucket = dbhash_bucket_new(entry_id);

    if (NULL != hash->hash_index.hash_key)
    {
        key1 = (*hash->hash_index.hash_key)(data);
        index1 = hash->hash_index.index;

        if (key1 >= hash->hash_index.size)
        {
            free(bucket);
            free(cmpdata);
            return -1;
        }

        {
            for (mp = &(index1[ key1 ]); *mp != NULL; mp = &((*mp)->next))
            {
                db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);

                if ((*hash->hash_index.hash_cmp)(data, cmpdata) == TRUE)
                {
                    free(bucket);
					free(cmpdata);
					return 0;
                }
            } /* FOR (...) */

            *mp = bucket;
        }
    }

    if (!(hash->hash_index.hash_key))
    {
        free(bucket);
    }

    free(cmpdata);
    return 0;
}

int dbtable_hash_delete(
    hash_table_index_t *hash,
    void *data,
    void *ret_data
)
{
    int ret = -1;
    unsigned int    key1 = 0;
    struct dbhash_bucket **mp  = NULL;
    struct dbhash_bucket *mpp = NULL;
    struct dbhash_bucket ** index1 = NULL;
    unsigned int entry_id;
    int * count = NULL;
    char *cmpdata = NULL;

    if ((NULL == hash)||(NULL == data))
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;
    cmpdata = malloc(hash->common.db->entry_size);
    if (NULL == cmpdata)
    {
        return -1;
    }

    index1 = hash->hash_index.index;
    db_table_lock(hash->common.db);
    count = &(hash->hash_index.count);

    if (hash->hash_index.hash_key) /* pull for key1 if not null*/
    {
        key1 = (*hash->hash_index.hash_key)(data);

        if (key1 >= hash->hash_index.size)
        {
			db_table_unlock(hash->common.db);
            free(cmpdata);
            return -1;
        }

        mp = &(index1[key1]);

        while (*mp)
        {
            db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);

            if ((*hash->hash_index.hash_cmp)(cmpdata,data) == TRUE)
            {
                entry_id = (*mp)->index;
                mpp = *mp;
                *mp = (*mp)->next;
                db_table_free_entry_index(hash->common.db, entry_id, NULL,
                                  (db_index_common_t*)hash, TRUE);                
                break;
            }

            mp = &((*mp)->next);
        } /* WHILE (...) */
    }

    if ((NULL != mpp))
    {
        entry_id = mpp->index;
        db_table_get_entry(hash->common.db, entry_id, cmpdata);
		ret = 0;
		if(hash->common.db->handle_delete)
		{
            ret = (*hash->common.db->handle_delete)(cmpdata);
		}
        if(ret_data)
            memcpy(ret_data, cmpdata, hash->common.db->entry_size);
/*
        if (0 == ret)
        {
*/
            db_table_delete_entry(hash->common.db, entry_id, NULL,
                                  (db_index_common_t*)hash, TRUE);
/*

        }
        else
        {
            dbtable_hash_internal_insert(&hash->common, ret_data, entry_id);
            db_table_synced_entry(hash->common.db, entry_id, DB_DELETE_NOT_SYNCED);
        }
*/
        free(mpp);
        (*count)--;
        mpp = NULL;
    }

    db_table_unlock(hash->common.db);
    free(cmpdata);
    return 0;
}

int dbtable_hash_internal_delete(
    db_index_common_t *common,
    void *data,
    unsigned int entry_id
)
{
    unsigned int    key1 = 0;
    hash_table_index_t *hash = (hash_table_index_t*)common;
    struct dbhash_bucket **mp  = NULL;
    struct dbhash_bucket *mpp = NULL;
    struct dbhash_bucket ** index1 = NULL;
    int * count = NULL;
    char *cmpdata = NULL;

    if (NULL == hash)
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;
	
    cmpdata = malloc(hash->common.db->entry_size);
    if (NULL == cmpdata)
    {
        return -1;
    }

    index1 = hash->hash_index.index;
    count = &(hash->hash_index.count);

    if (hash->hash_index.hash_key) /* pull for key1 if not null*/
    {
        key1 = (*hash->hash_index.hash_key)(data);

        if (key1 >= hash->hash_index.size)
        {
            free(cmpdata);
            return -1;
        }

        mp = &(index1[key1]);

        while (*mp)
        {
            db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);

            if ((*hash->hash_index.hash_cmp)(cmpdata,data) == TRUE)
            {
                mpp = *mp;
                *mp = (*mp)->next;
                break;
            }

            mp = &((*mp)->next);
        } /* WHILE (...) */
    }

    if ((NULL != mpp))
    {
        free(mpp);
        (*count)--;
        mpp = NULL;
    }

    free(cmpdata);
    return 0;
}

int dbtable_hash_update(
    hash_table_index_t *hash,
    void *predata,
    void *data
)
{
    char *cmpdata = NULL;
    int ret;
    if(NULL == hash)
    {
		return -1;
    }
    if(!hash->common.db)
        return -1;
    cmpdata = malloc(hash->common.db->entry_size);
    if (NULL == cmpdata)
    {
        return -1;
    }
    db_table_read_lock(hash->common.db);
    ret = dbtable_hash_search(hash, data, NULL, cmpdata);
    if(0 != ret)
    {
        free(cmpdata);
        db_table_unlock(hash->common.db);
        return -1;
    }
    
    ret = dbtable_hash_insert(hash, data);

    free(cmpdata);
    db_table_unlock(hash->common.db);
    return ret;
}


int dbtable_hash_count(
    hash_table_index_t *hash
)
{
    if (NULL == hash)
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;
    return hash->common.db->entry_use_num;
}


int dbtable_hash_next
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
)
{
    unsigned int    key1;
    struct dbhash_bucket **mp = NULL;
    struct dbhash_bucket **index1 = NULL;
    int first_flag = 0;

    if (NULL == hash)
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;

    char *cmpdata = malloc(hash->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }

    db_table_read_lock(hash->common.db);

    if (NULL != hash->hash_index.hash_key)
    {
        if (NULL == data)
        {
            /*get first valid entry*/
            key1 = 0;
            first_flag = 1;
        }
        else
        {
            key1 = (*hash->hash_index.hash_key)(data);
        }
        index1 = hash->hash_index.index;

        if (key1 >= hash->hash_index.size)
        {
            key1 = 0;
            first_flag = 1;
        }
        
        while(1)
        {
            if (key1 >= hash->hash_index.size)
            {
				break;
            }
        
            for (mp = &(index1[key1]); ; mp = &((*mp)->next))
            {
                if(NULL == (*mp))
                {
                    first_flag = 1;
                    key1++;
                    break;
                }
                db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);
                if(first_flag)
                {
                    if(filter)
                    {
                        int filter_flag;
                        filter_flag = (*filter)(data, cmpdata);
                        if(FALSE == filter_flag)
                        {
                            continue;
                        }
                    }
                    memcpy(outdata, cmpdata, hash->common.db->entry_size);
                    db_table_read_unlock(hash->common.db);
                    free(cmpdata);
                    return 0;
                }
                if (((*hash->hash_index.hash_cmp)(data, cmpdata) == TRUE))
                {
                    dbhash_bucket_t *bucket;
                    bucket = (*mp)->next;
                    while(bucket)
                    {
                        db_table_get_entry(hash->common.db, bucket->index, cmpdata);
                        if(filter)
                        {
                            int filter_flag;
                            filter_flag = (*filter)(data, cmpdata);
                            if(FALSE == filter_flag)
                            {
                                bucket = bucket->next;
                                continue;
                            }
                        }
                        memcpy(outdata, cmpdata, hash->common.db->entry_size);
                        db_table_read_unlock(hash->common.db);
                        free(cmpdata);
                        return 0;
                    }
                    key1++;
                    first_flag = 1;
                    break;
                }
            } /* FOR (...) */
        }
    }
    db_table_read_unlock(hash->common.db);
    free(cmpdata);
    return -1;
}


int dbtable_hash_head
(
    hash_table_index_t *hash,
    void * indata,
    void * outdata,
    unsigned int (*filter)(void *,void *)
)
{
    unsigned int i;
	struct dbhash_bucket *backet = NULL, *next = NULL;

    if ((NULL == hash)||(NULL == outdata))
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;

    char *cmpdata = malloc(hash->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }

    db_table_read_lock(hash->common.db);

    if(0 == dbtable_hash_count(hash))
	{
		db_table_read_unlock(hash->common.db);
		free(cmpdata);
		return -1;
	}
	
	for (i=0; i<hash->hash_index.size; i++)
	{
		for (backet = hash->hash_index.index[i] ; backet; )
		{
			next = backet->next;

			db_table_get_entry(hash->common.db, backet->index, cmpdata);
			{
                if(filter && (NULL != indata))
                {
                    int filter_flag;
                    filter_flag = (*filter)(indata, cmpdata);
                    if(FALSE == filter_flag)
                    {
						backet = next;
                        continue;
                    }
                }
                {
                    db_table_get_entry(hash->common.db, backet->index, outdata);
                    db_table_read_unlock(hash->common.db);
                    free(cmpdata);
                    return 0;
                }
            }
		}
	}
    
    db_table_read_unlock(hash->common.db);
    free(cmpdata);
    return -1;
}

int dbtable_hash_head_key
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
)
{
    unsigned int    key1;
    struct dbhash_bucket **mp = NULL;
    struct dbhash_bucket **index1 = NULL;

    if ((NULL == hash)||(NULL == data)||(NULL == outdata))
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;

    char *cmpdata = malloc(hash->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }

    db_table_read_lock(hash->common.db);

    if (NULL != hash->hash_index.hash_key)
    {
        key1 = (*hash->hash_index.hash_key)(data);
        index1 = hash->hash_index.index;

        
        mp = &(index1[key1]);
        while(*mp)
        {
            db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);
            {
                if(filter)
                {
                    int filter_flag;
                    filter_flag = (*filter)(data, cmpdata);
                    if(FALSE == filter_flag)
                    {
                        mp = &((*mp)->next);
                        continue;
                    }
                }
                {
                    db_table_get_entry(hash->common.db, (*mp)->index, outdata);
                    db_table_read_unlock(hash->common.db);
                    free(cmpdata);
                    return 0;
                }
            }
        } /* FOR (...) */
    }
    db_table_read_unlock(hash->common.db);
    free(cmpdata);
    return -1;
}


int dbtable_hash_next_key
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
)
{
    unsigned int    key1;
    struct dbhash_bucket **mp = NULL;
    struct dbhash_bucket **index1 = NULL;

    if ((NULL == hash)||(NULL == data))
    {
        return -1;
    }
    if(!hash->common.db)
        return -1;

    char *cmpdata = malloc(hash->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }

    db_table_read_lock(hash->common.db);

    if (NULL != hash->hash_index.hash_key)
    {
        key1 = (*hash->hash_index.hash_key)(data);
        index1 = hash->hash_index.index;

        
        for (mp = &(index1[key1]); NULL != (*mp); mp = &((*mp)->next))
        {
            db_table_get_entry(hash->common.db, (*mp)->index, cmpdata);
            if (((*hash->hash_index.hash_cmp)(data, cmpdata) == TRUE))
            {
                dbhash_bucket_t *bucket;
                bucket = (*mp)->next;
                while(bucket)
                {
                    db_table_get_entry(hash->common.db, bucket->index, cmpdata);
                    if(filter)
                    {
                        int filter_flag;
                        filter_flag = (*filter)(data, cmpdata);
                        if(FALSE == filter_flag)
                        {
                            bucket = bucket->next;
                            continue;
                        }
                    }
                    memcpy(outdata, cmpdata, hash->common.db->entry_size);
                    db_table_read_unlock(hash->common.db);
                    free(cmpdata);
                    return 0;
                }
                break;
            }
        } /* FOR (...) */
    }
    db_table_read_unlock(hash->common.db);
    free(cmpdata);
    return -1;
}


int dbtable_hash_return
(
    hash_table_index_t *hash,
    void * indata,
    void * outdata,
    unsigned int len,
    unsigned int (*filter)(void *,void *)
)
{
    int i = 0,count = 0;
    struct dbhash_bucket *backet = NULL, *next = NULL;
    unsigned int filterMatch = 0;
    char* proc_data;

    if (NULL == hash)
    {
        return 0;
    }
    if(!hash->common.db)
        return -1;

	proc_data = malloc(hash->common.db->entry_size);
	
	if( proc_data == NULL )
		return 0;
	
    db_table_read_lock(hash->common.db);
    for (i=0; i<hash->hash_index.size; i++)
    {
        for (backet = hash->hash_index.index[i]; backet != NULL; backet= next )
        {
        	next = backet->next;
			
        	db_table_get_entry(hash->common.db, backet->index, (void*)proc_data);
			
            if (NULL != filter)  /* specific filter given*/
            {                                
                filterMatch = (*filter)(proc_data,indata);
            }
            else   /* no filter specified*/
            {
                filterMatch = TRUE;
            }

            if (TRUE == filterMatch)
            {
				if(outdata)
				{
            	    memcpy( ((char*)outdata+count*hash->common.db->entry_size), proc_data, hash->common.db->entry_size);
				}
                count++;
                if(len && outdata)
                {
                    if (count >= len)
                        goto full;
                }
            }
        }
    }
full:	
    db_table_read_unlock(hash->common.db);

	free(proc_data);

    return count;
}

int dbtable_hash_traversal(
    hash_table_index_t *hash,
    unsigned int flag,
    void * data,
    unsigned int (*filter)(void *,void *),
    int (*processor)(hash_table_index_t*, void *,unsigned int)
)
{
    int i = 0,count = 0;
    unsigned int filterMatch = 0;
    struct dbhash_bucket *backet = NULL, *next = NULL;
    if (NULL == hash)
    {
        return 0;
    }
    if(!hash->common.db)
    {
        return -1;
    }
    void *proc_data = malloc(hash->common.db->entry_size);

    if (NULL == proc_data)
    {
        return 0;
    }
    if(processor)
    {
        db_table_lock(hash->common.db);
    }
	else
	{
        db_table_read_lock(hash->common.db);
	}
    for (i=0; i<hash->hash_index.size; i++)
    {
        for (backet = hash->hash_index.index[i]	; backet; )
        {
            next = backet->next;

			db_table_get_entry(hash->common.db, backet->index, proc_data);
			
            if (NULL != filter)  /* specific filter given*/
            {                
                filterMatch = (*filter)(proc_data,data);
            }
            else   /* no filter specified*/
            {
                filterMatch = TRUE;
            }

            if (TRUE == filterMatch) count++;

            if ((TRUE == filterMatch) && (NULL != processor))
            {
                (*processor)(hash, proc_data,flag);
            }
            backet = next;
        }
    }
	if(processor)
	{
        db_table_unlock(hash->common.db);
	}
	else
	{
        db_table_read_unlock(hash->common.db);
	}
    free(proc_data);
	
    return count;
}

int dbtable_hash_traversal_key(
    hash_table_index_t *hash,
    unsigned int flag,
    void * data,
    unsigned int (*filter)(void *,void *),
    int (*processor)(hash_table_index_t*, void*, unsigned int)
)
{
    int count = 0;
    unsigned int filterMatch = 0;
    struct dbhash_bucket *backet = NULL, *next = NULL;
	void *proc_data = NULL;
    int key;

    if (NULL == hash)
    {
        return 0;
    }
    if(!hash->common.db)
        return -1;
    proc_data = malloc(hash->common.db->entry_size);
    if (NULL == proc_data)
    {
        return 0;
    }

    key = hash->hash_index.hash_key(data);
    db_table_read_lock(hash->common.db);
    
    {
        for (backet = hash->hash_index.index[key]	; backet; )
        {
            next = backet->next;

			db_table_get_entry(hash->common.db, backet->index, proc_data);

            if (NULL != filter)  /* specific filter given*/
            {                
                filterMatch = (*filter)(proc_data,data);
            }
            else   /* no filter specified*/
            {
                filterMatch = TRUE;
            }

            if (TRUE == filterMatch) count++;

            if ((TRUE == filterMatch) && (NULL != processor))
            {
                (*processor)(hash, proc_data,flag);
                
            }
            backet = next;
        }
    }
    db_table_read_unlock(hash->common.db);
    free(proc_data);
	
    return count;
}



void dbtable_hash_show
(
	hash_table_index_t *hash,
	char* string,
	void (*showFunc)(void *, char*)
)
{
	int i = 0,count = 0;
	struct dbhash_bucket *backet = NULL, *next = NULL;
	void *proc_data = NULL; 
	
    if(NULL == hash){
		return;
    }
    if(!hash->common.db)
        return ;

    if (NULL == showFunc) return;

    proc_data = malloc(hash->common.db->entry_size);

    if (NULL == proc_data)
    {
        return;
    }

    for (i=0; i<hash->hash_index.size; i++)
    {
        for (backet = hash->hash_index.index[i]	; backet;)
        {
            next = backet->next;

            if (NULL != showFunc)
            {
                db_table_get_entry(hash->common.db, backet->index, proc_data);
                (*showFunc)(proc_data,string);
            }

            count++;
        }
        backet = next;        
    }
    free(proc_data);
    return;
}

int dbtable_sequence_internal_insert(
    db_index_common_t *common,
    void *data,
    unsigned int entry_id
);
int dbtable_sequence_internal_delete(
    db_index_common_t *common,
    void *data,
    unsigned int entry_id
);

int dbtable_create_sequence_index(
	char *index_name,
    db_table_t *db,
    unsigned int sequence_size,
    unsigned int (*sequence_index)(unsigned int),
    unsigned int (*sequence_key)(void*),
    int (*sequence_cmp)(void *, void*),
    sequence_table_index_t **sequence
)
{
    *sequence = malloc(sizeof(sequence_table_index_t));

    if (NULL == *sequence)
    {
        *sequence = NULL;
        return -1;
    }
    memset(*sequence, 0, sizeof(sequence_table_index_t));

    (*sequence)->common.db = db;
	strncpy((*sequence)->common.index_name, index_name, 31);
	(*sequence)->common.index_type = DB_TABLE_INDEX_SEQ;
    (*sequence)->common.index_insert = dbtable_sequence_internal_insert;
    (*sequence)->common.index_delete = dbtable_sequence_internal_delete;
    list_add(&(*sequence)->common.list, &db->index_list);
    (*sequence)->sequence.size = sequence_size;
    (*sequence)->sequence.index = malloc(sequence_size*(sizeof(dbsequence_bucket_t*)));

    if ((*sequence)->sequence.index == NULL)
    {
        free(*sequence);
        *sequence = NULL;
        return -1;
    }

    memset((*sequence)->sequence.index, 0, sequence_size*sizeof(dbsequence_bucket_t*));
    (*sequence)->sequence.sequence_index = sequence_index;
    (*sequence)->sequence.sequence_cmp = sequence_cmp;
    (*sequence)->sequence.sequence_key = sequence_key;
    return 0;
}

struct dbsequence_bucket * dbsequence_bucket_new
(
    unsigned int index
)
{
    struct dbsequence_bucket * bucket = NULL;

    bucket = malloc(sizeof(struct dbsequence_bucket));

    if (!bucket)
    {
        return NULL;
    }

    bucket->index = index;
    bucket->next = NULL;
    bucket->pre = NULL;

    return bucket;
}


int dbtable_sequence_insert(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data
)
{
    unsigned int index = 0;
    dbsequence_bucket_t *bucket_new,  **mp;
    unsigned int entry_id;
    int ret;
    char *cmpdata = NULL;

    if(NULL == sequence)
    {
		printf("%s:Sequence index is not been created.\r\n", __func__);
		return -1;
    }
    if(!sequence->common.db)
        return -1;
	cmpdata = malloc(sequence->common.db->entry_size);
    if (NULL == cmpdata)
    {
        return -1;
    }

    /*lock mutex*/
    db_table_lock(sequence->common.db);

    if (*sequence->sequence.sequence_index != NULL)
        index = (*sequence->sequence.sequence_index)(sequence_index);

    if (index >= sequence->sequence.size)
    {
        db_table_unlock(sequence->common.db);
        free(cmpdata);
        return -1;
    }

    mp = &(sequence->sequence.index[index]);
    {
        for (; (*mp) != NULL; mp = &((*mp)->next))
        {
            db_table_get_entry(sequence->common.db, (*mp)->index, cmpdata);

            if ((*sequence->sequence.sequence_cmp)(data, cmpdata) == TRUE)
            {
                db_table_update_entry(sequence->common.db,(*mp)->index, data, 
                    (db_index_common_t*)sequence, TRUE);
                /*unlock mutex*/
				ret = 0;
				if(sequence->common.db->handle_update)
				{
                    ret = (*sequence->common.db->handle_update)(data, cmpdata);
				}

                if (0 == ret)
                {
                    db_table_synced_entry(sequence->common.db, (*mp)->index, DB_HW_SW_SYNCED);
                }
                else
                {
                    printf("Sequence %x for TABLE %s update unsuccess\r\n", (unsigned int)sequence, sequence->common.db->name);
                    db_table_unsynced_entry(sequence->common.db, (*mp)->index, DB_HW_SW_SYNCED);
                }
                db_table_unlock(sequence->common.db);

                free(cmpdata);
                return 0;
            }
        } /* FOR (...) */

        entry_id = osal_table_entry_id_alloc(sequence->common.db);
    
        if (-1 == entry_id)
        {
            db_table_unlock(sequence->common.db);
            free(cmpdata);
            return -1;
        }
    
        bucket_new = dbsequence_bucket_new(entry_id);
    
        if (NULL == bucket_new)
        {
            osal_table_entry_id_free(sequence->common.db, entry_id);
            db_table_unlock(sequence->common.db);
            free(cmpdata);
            return -1;
        }

        *mp = bucket_new;
    }
    db_table_insert_entry(sequence->common.db, entry_id, data, 
           (db_index_common_t*)sequence, TRUE);
	ret = 0;
	if(sequence->common.db->handle_insert)
	{
        ret = (*sequence->common.db->handle_insert)(data);
	}

    if (0 == ret)
    {
        db_table_synced_entry(sequence->common.db, entry_id, DB_HW_SW_SYNCED);
    }
    else
    {
        printf("Sequence %x for TABLE %s insert unsuccess\r\n", (unsigned int)sequence, sequence->common.db->name);
    }
    db_table_unlock(sequence->common.db);

    free(cmpdata);
    return 0;
}

int dbtable_sequence_internal_insert(
    db_index_common_t *common,
    void *data,
    unsigned int entry_id
)
{
    unsigned int index = 0;
    sequence_table_index_t *sequence = (sequence_table_index_t*)common;
    dbsequence_bucket_t *bucket_new, **mp;
    char *cmpdata = malloc(sequence->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }
    if(!sequence->common.db)
        return -1;

    if (*sequence->sequence.sequence_key != NULL)
        index = (*sequence->sequence.sequence_key)(data);

    if (*sequence->sequence.sequence_index != NULL)
        index = (*sequence->sequence.sequence_index)(index);

    if (index >= sequence->sequence.size)
    {
        free(cmpdata);
        return -1;
    }

    bucket_new = dbsequence_bucket_new(entry_id);

    if (NULL == bucket_new)
    {
        osal_table_entry_id_free(sequence->common.db, entry_id);
        free(cmpdata);
        return -1;
    }

    mp = &(sequence->sequence.index[index]);
    {
        for (; (*mp) != NULL; mp = &((*mp)->next))
        {
            db_table_get_entry(sequence->common.db, (*mp)->index, cmpdata);

            if ((*sequence->sequence.sequence_cmp)(data, cmpdata) == TRUE)
            {
                free(bucket_new);
                free(cmpdata);
                return 0;
            }
        } /* FOR (...) */

        *mp = bucket_new;
    }
    free(cmpdata);
    return 0;
}

int dbtable_sequence_search(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data
)
{
    unsigned int key;
    unsigned int entry_id;
    int ret;
    dbsequence_bucket_t **mp;
    char *cmpdata = NULL;
    if(NULL == sequence)
    {
		printf("%s:Sequence index is not been created.\r\n", __func__);
		return -1;
    }
    if(!sequence->common.db)
        return -1;
    cmpdata = malloc(sequence->common.db->entry_size);
    if (NULL == cmpdata)
    {
        return -1;
    }
    db_table_read_lock(sequence->common.db);
	
    key = (*sequence->sequence.sequence_index)(sequence_index);
	if (key >= sequence->sequence.size)
    {
        db_table_read_unlock(sequence->common.db);
        free(cmpdata);
        return -1;
    }
	
    mp = &(sequence->sequence.index[key]);


    for (; (*mp) != NULL; mp = &(*mp)->next)
    {
        entry_id = (*mp)->index;
        ret = db_table_get_entry(sequence->common.db, entry_id, cmpdata);

        if (ret == -1)
            continue;

        if (TRUE == (*sequence->sequence.sequence_cmp)(cmpdata, data))
        {
            break;
        }
    }

    if (NULL == *mp)
    {
        db_table_read_unlock(sequence->common.db);
        free(cmpdata);
        return -1;
    }

    ret = db_table_get_entry(sequence->common.db, entry_id, data);
    db_table_read_unlock(sequence->common.db);
    free(cmpdata);
    return ret;
}

int dbtable_sequence_head(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data
)
{
    unsigned int key;
    unsigned int entry_id;
    dbsequence_bucket_t *bucket = NULL;
    int ret;
    if(NULL == sequence)
    {
		printf("%s:Sequence index is not been created.\r\n", __func__);
		return -1;
    }
    if(!sequence->common.db)
        return -1;
    db_table_read_lock(sequence->common.db);
    key = (*sequence->sequence.sequence_index)(sequence_index);
    bucket = sequence->sequence.index[key];

    while(bucket)
    {
        entry_id = bucket->index;
        ret = db_table_get_entry(sequence->common.db, entry_id, data);
        if((*sequence->sequence.sequence_key)(data) == sequence_index)
        {
            db_table_read_unlock(sequence->common.db);
            return ret;
                
        }
        bucket = bucket->next;
    }

    db_table_read_unlock(sequence->common.db);
    return -1;
}


int dbtable_sequence_next(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data,
    void *ret_data,
    unsigned int (*filter)(void *,void *)
)
{
    unsigned int key;
    unsigned int entry_id;
    int ret = -1;
    dbsequence_bucket_t *bucket = NULL;
    void *cmpdata;

    
    if(NULL == sequence)
    {
		printf("%s:Sequence index is not been created.\r\n", __func__);
		return -1;
    }
    if(!sequence->common.db)
        return -1;
    
    cmpdata = malloc(sequence->common.db->entry_size);
    if(NULL == cmpdata)
    {
        return -1;
    }
    
    db_table_read_lock(sequence->common.db);

    key = (*sequence->sequence.sequence_index)(sequence_index);
    bucket = sequence->sequence.index[key];

    while(bucket)
    {
        entry_id = bucket->index;
        ret = db_table_get_entry(sequence->common.db, entry_id, cmpdata);
        if(0 == ret)
        {
            if((*sequence->sequence.sequence_key)(cmpdata) == sequence_index)
            {
                bucket = bucket->next;
                while(bucket)
                {
                    entry_id = bucket->index;
                    ret = db_table_get_entry(sequence->common.db, entry_id, cmpdata);
                    if(0 != ret)
                    {
                        goto error;
                    }
                    if(filter)
                    {
                        int filter_flag;
                        filter_flag = (*filter)(data, cmpdata);
                        if(FALSE == filter_flag)
                        {
                            bucket = bucket->next;
                            continue;
                        }
                    }
                    goto success;
                }
                goto error;
            }
        }
        else
            goto error;
        
        bucket = bucket->next;
    }
    ret = -1;
    goto error;
success:
    memcpy(ret_data, cmpdata, sequence->common.db->entry_size);
    free(cmpdata);
    db_table_read_unlock(sequence->common.db);
    return 0;
error:
    free(cmpdata);
    db_table_read_unlock(sequence->common.db);
    return -1;
}


int dbtable_sequence_traverse_next(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *ret_data
)
{
    unsigned int key;
    unsigned int entry_id;
    int ret = -1;
    dbsequence_bucket_t *bucket = NULL;
    int first_flag = FALSE;
    void *cmpdata;
    
    if(NULL == sequence)
    {
		printf("%s:Sequence index is not been created.\r\n", __func__);
		return -1;
    }
    if(!sequence->common.db)
        return -1;
    cmpdata = malloc(sequence->common.db->entry_size);
    if(NULL == cmpdata)
    {
        return -1;
    }
    
    db_table_read_lock(sequence->common.db);
    if(-1 == sequence_index)
    {
        key = 0;
        first_flag = TRUE;
    }
    else
    {
        key = (*sequence->sequence.sequence_index)(sequence_index);
        if(key > sequence->sequence.size)
        {
            key = 0;
            first_flag = TRUE;
        }
    }
    bucket = sequence->sequence.index[key];

    while(!bucket)
    {
        key++;
        first_flag = TRUE;
        if(key >= sequence->sequence.size)
        {
            ret = -1;
            goto error; 
        }
        bucket = sequence->sequence.index[key];
    }

    while(bucket)
    {
        entry_id = bucket->index;
        ret = db_table_get_entry(sequence->common.db, entry_id, cmpdata);
        if(0 == ret)
        {
            if(first_flag)
                goto success;
            
            if((*sequence->sequence.sequence_key)(cmpdata) == sequence_index)
            {
                bucket = bucket->next;
                if(bucket)
                {
                    entry_id = bucket->index;
                    ret = db_table_get_entry(sequence->common.db, entry_id, cmpdata);
                    if(0 != ret)
                    {
                        goto error;
                    }
                    goto success;
                }
                else
                {
                    while(!bucket)
                    {
                        key++;
                        if(key >= sequence->sequence.size)
                        {
                            ret = -1;
                            goto error; 
                        }
                        bucket = sequence->sequence.index[key];
                    }
                    first_flag = TRUE;
                    continue;
                }
            }
        }
        else
            goto error;
        
        bucket = bucket->next;
    }    
    ret = -1;
    goto error;
success:
    memcpy(ret_data, cmpdata, sequence->common.db->entry_size);
    free(cmpdata);
    db_table_read_unlock(sequence->common.db);
    return 0;
error:
    free(cmpdata);
    db_table_read_unlock(sequence->common.db);
    return ret;
}


int dbtable_sequence_delete(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data,
    void *ret_data
)
{
    unsigned int    key1 = 0;
    struct dbsequence_bucket **mp  = NULL;
    struct dbsequence_bucket *mpp = NULL;
    struct dbsequence_bucket ** index1 = NULL;
    unsigned int entry_id;

    if (NULL == sequence)
    {
        return -1;
    }
    if(!sequence->common.db)
        return -1;

    index1 = sequence->sequence.index;
    db_table_lock(sequence->common.db);
    {
        key1 = (*sequence->sequence.sequence_index)(sequence_index);

        if (key1 >= sequence->sequence.size)
        {
            db_table_unlock(sequence->common.db);
            return -1;
        }

        mp =  &(index1[key1]);

        while (*mp)
        {
            entry_id = (*mp)->index;
            db_table_get_entry(sequence->common.db, entry_id, ret_data);

            if ((*sequence->sequence.sequence_cmp)(data,ret_data) == TRUE)
            {
                mpp = *mp;
                *mp = (*mp)->next;
                db_table_free_entry_index(sequence->common.db, entry_id, 
                    NULL, (db_index_common_t*) sequence, TRUE);                
                
                break;
            }
			mp = &((*mp)->next);
        } /* WHILE (...) */
    }

    if ((NULL != mpp))
    {
        entry_id = mpp->index;
        db_table_get_entry(sequence->common.db, entry_id, ret_data);
		if(sequence->common.db->handle_delete)
		{
            (*sequence->common.db->handle_delete)(ret_data);
		}
#if 0
        if (0 == ret)
        {
#endif
            db_table_delete_entry(sequence->common.db, entry_id, 
                NULL, (db_index_common_t*)sequence, TRUE);
#if 0
        }
        else
        {
            dbtable_sequence_internal_insert(&sequence->common, ret_data, entry_id);
            db_table_synced_entry(sequence->common.db, entry_id, DB_DELETE_NOT_SYNCED);
        }
#endif
        free(mpp);
        mpp = NULL;
    }

    db_table_unlock(sequence->common.db);
    return 0;
}


int dbtable_sequence_internal_delete(
    db_index_common_t *common,
    void *data,
    unsigned int id
)
{
    unsigned int    key1 = 0;
    sequence_table_index_t *sequence = (sequence_table_index_t*)common;
    dbsequence_bucket_t **mp  = NULL;
    dbsequence_bucket_t *mpp = NULL;
    dbsequence_bucket_t ** index1 = NULL;
    char *cmpdata = NULL;
    unsigned int entry_id;

    if(NULL == sequence)
    {
		printf("%s:Sequence index is not been created.\r\n", __func__);
		return -1;
    }
    if(!sequence->common.db)
        return -1;
	cmpdata = malloc(sequence->common.db->entry_size);
    if (NULL == cmpdata)
    {
        return -1;
    }

    index1 = sequence->sequence.index;
    {
        if (sequence->sequence.sequence_key != NULL)
            key1 = (*sequence->sequence.sequence_key)(data);

        if (sequence->sequence.sequence_index != NULL)
            key1 = (*sequence->sequence.sequence_index)(key1);

        if (key1 >= sequence->sequence.size)
        {
            free(cmpdata);
            return -1;
        }

        mp =  &(index1[key1]);

        while (*mp)
        {
            entry_id = (*mp)->index;
            db_table_get_entry(sequence->common.db, entry_id, cmpdata);
            if(sequence->sequence.sequence_cmp)
            {
                if ((*sequence->sequence.sequence_cmp)(data,cmpdata) == TRUE)
                {
                    mpp = *mp;
                    *mp = (*mp)->next;
                    break;
                }

                mp = &((*mp)->next);
            }
        } /* WHILE (...) */
    }

    if ((NULL != mpp))
    {
        free(mpp);
        mpp = NULL;
    }
    free(cmpdata);
    return 0;
}

/*
int dbtable_sequence_count(
    sequence_table_index_t *sequence,
    unsigned int sequence_index
)
{
    return dbtable_hash_count(sequence);
}
*/

int dbtable_sequence_traversal(
	sequence_table_index_t *sequence,
	unsigned int flag,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*processor)(sequence_table_index_t*, void *,unsigned int)
)
{
    int i = 0,count = 0;
    unsigned int filterMatch = 0;
    struct dbsequence_bucket *backet = NULL, *next = NULL;
    void *proc_data = NULL;

    if (NULL == sequence)
    {
        return 0;
    }
    if(!sequence->common.db)
        return -1;
	proc_data = malloc(sequence->common.db->entry_size);
    if (NULL == proc_data)
    {
        return 0;
    }

    if(processor)
    {
        db_table_lock(sequence->common.db);
    }
	else
	{
        db_table_read_lock(sequence->common.db);
	}
	for(i=0;i<sequence->sequence.size;i++) 
    {
        for (backet = sequence->sequence.index[i]; backet;)
        {
            next = backet->next;

            db_table_get_entry(sequence->common.db, backet->index, proc_data);

            if (NULL != filter)  /* specific filter given*/
            {
                filterMatch = (*filter)(proc_data,data);
            }
            else   /* no filter specified*/
            {
                filterMatch = TRUE;
            }

            if (TRUE == filterMatch) count++;

            if ((TRUE == filterMatch) && (NULL != processor))
            {
                (*processor)(sequence, proc_data,flag);
                
            }
            backet = next;
        }
    }
    if(processor)
    {
        db_table_unlock(sequence->common.db);
    }
	else
	{
        db_table_read_unlock(sequence->common.db);
	}
    free(proc_data);
    return count;
}


int dbtable_sequence_show(
	sequence_table_index_t *sequence,
	char * string,
	int* string_size,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*show)(void *, char*, int*)
)
{
	int i = 0,count = 0;
	unsigned int filterMatch = 0;
    struct dbsequence_bucket *backet = NULL, *next = NULL;
    void *proc_data = NULL;

	if(NULL == sequence){
		return 0;
	}
    if(!sequence->common.db)
        return -1;
	proc_data = malloc(sequence->common.db->entry_size);
    if(NULL == proc_data){        
        return 0;
    }
	for(i=0;i<sequence->sequence.size;i++) 
    {
		for(backet = sequence->sequence.index[i];backet;backet=next) {
            int ret = db_table_get_entry(sequence->common.db, backet->index, proc_data);
			next = backet->next;
			if(NULL != filter) { /* specific filter given*/ 
				filterMatch = (*filter)(proc_data,data);				
			}
			else { /* no filter specified*/
         
				filterMatch = TRUE;
			}

			if(TRUE == filterMatch) count++;
			
			if((TRUE == filterMatch) && (NULL != show) && ret == 0) {
                
				(*show)(proc_data,string, string_size);
			}
		}
	}

    free(proc_data);

	return count;
}


int dbtable_sequence_update(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *predata,
    void *data
)
{
    char *cmpdata = NULL;
    int ret;
    if(NULL == sequence)
    {
		printf("%s:Sequence index is not been created.\r\n", __func__);
		return -1;
    }
    if(!sequence->common.db)
        return -1;
    cmpdata = malloc(sequence->common.db->entry_size);
    if (NULL == cmpdata)
    {
        return -1;
    }
    memcpy(cmpdata, data, sequence->common.db->entry_size);
    /*db_table_read_lock(sequence->common.db);*/
    ret = dbtable_sequence_search(sequence, sequence_index, cmpdata);
    if(0 != ret)
    {
        free(cmpdata);
        /*db_table_unlock(sequence->common.db);*/
        return -1;
    }
    ret =  dbtable_sequence_insert(sequence, sequence_index, data);
    free(cmpdata);
    /*db_table_unlock(sequence->common.db);*/
    return ret;
}

int dbtable_array_internal_insert(
    array_table_index_t *array,
    void *data,
    unsigned int entry_id
);
int dbtable_array_internal_delete(
    array_table_index_t *array,
    void *data,
    unsigned int entry_id
);

int dbtable_create_array_index(
	char *index_name,
    db_table_t *db,
    array_table_index_t **array
)
{
    *array = malloc(sizeof(array_table_index_t));

    if (NULL == *array)
    {
        *array = NULL;
        return -1;
    }
    memset(*array, 0, sizeof(array_table_index_t));
    
    (*array)->db = db;
	sprintf((*array)->index_name, index_name, 31);
	(*array)->index_type = DB_TABLE_INDEX_ARRAY;
    (*array)->index_insert = dbtable_array_internal_insert;
    (*array)->index_delete = dbtable_array_internal_delete;
    list_add(&(*array)->list, &db->index_list);
    return 0;
}

int dbtable_array_insert_byid(
    array_table_index_t *array,
    unsigned int array_index,
    void *data
)
{
    unsigned int entry_id;
    int ret  = 0;

    /*lock mutex*/
    if(!array->db)
        return -1;
    db_table_lock(array->db);

    ret = db_table_internal_alloc(array->db, array_index, &entry_id);
    if (-1 == ret)
    {
        db_table_unlock(array->db);
        array_index = -1;
        return -1;
    }

    db_table_insert_entry(array->db, entry_id, data, array, TRUE);
	if(array->db->handle_insert)
	{
        ret = (*array->db->handle_insert)(data);
	}
    if (0 == ret)
    {
        db_table_synced_entry(array->db, array_index, DB_HW_SW_SYNCED);
    }
    else
    {
        printf("Array %x TABLE %s insert unsuccess\r\n", (unsigned int)array, array->db->name);
        db_table_unsynced_entry(array->db, entry_id, DB_HW_SW_SYNCED);
    }
    db_table_unlock(array->db);
    return 0;    
}

int dbtable_array_insert(
    array_table_index_t *array,
    unsigned int *array_index,
    void *data
)
{
    unsigned int entry_id;
    int ret  = 0;
    /*lock mutex*/
    if(!array->db)
        return -1;
    db_table_lock(array->db);
    entry_id = osal_table_entry_id_alloc(array->db);

    if (-1 == entry_id)
    {
        db_table_unlock(array->db);
        *array_index = -1;
        return -1;
    }

    db_table_insert_entry(array->db, entry_id, data, array, TRUE);
     
    *array_index = entry_id;
	if(array->db->handle_insert)
	{
        ret = (*array->db->handle_insert)(data);
	}
    if (0 == ret)
    {
        db_table_synced_entry(array->db, *array_index, DB_HW_SW_SYNCED);
    }
    else
    {
        printf("Array %x TABLE %s insert unsuccess\r\n", (unsigned int)array, array->db->name);
        db_table_unsynced_entry(array->db, entry_id, DB_HW_SW_SYNCED);
    }
    db_table_unlock(array->db);
    return 0;
}

int dbtable_array_insert_after(
    array_table_index_t *array,
    unsigned int *array_index,
    void *data,
    unsigned int start_id
)
{
    unsigned int entry_id;
    int ret  = 0;
    /*lock mutex*/
    if(!array->db)
        return -1;
    db_table_lock(array->db);
    ret = db_table_internal_alloc_after(array->db, start_id, &entry_id);

    if (0 != ret)
    {
        db_table_unlock(array->db);
        *array_index = -1;
        return -1;
    }

    db_table_insert_entry(array->db, entry_id, data, array, TRUE);
     
    *array_index = entry_id;
	if(array->db->handle_insert)
	{
        ret = (*array->db->handle_insert)(data);
	}
    if (0 == ret)
    {
        db_table_synced_entry(array->db, *array_index, DB_HW_SW_SYNCED);
    }
    else
    {
        printf("Array %x TABLE %s insert unsuccess\r\n", (unsigned int)array, array->db->name);
        db_table_unsynced_entry(array->db, entry_id, DB_HW_SW_SYNCED);
    }
    db_table_unlock(array->db);
    return 0;
}

int dbtable_array_internal_insert(
    array_table_index_t *array,
    void *data,
    unsigned int entry_id
)
{
    return 0;
}


int dbtable_array_get(
    array_table_index_t *array,
    unsigned int array_index,
    void *data
)
{
    unsigned int entry_id;
    int ret;
	if(array == NULL)
	{
		return -1;
	}
    if(!array->db)
        return -1;
    db_table_read_lock(array->db);
    entry_id = array_index;
    ret = db_table_get_entry(array->db, entry_id, data);
    db_table_read_unlock(array->db);
    return ret;
}

int dbtable_array_totalcount(
    array_table_index_t *array
    )
{
	if(array == NULL)
	{
		return 0;
	}
    if(!array->db)
        return 0;
    return array->db->entry_num;
}

int dbtable_array_delete(
    array_table_index_t *array,
    unsigned int array_index,
    void *ret_data
)
{
    int ret = 0;
	if(array == NULL)
	{
		return -1;
	}
    if(!array->db)
        return -1;
    db_table_lock(array->db);
    ret = db_table_get_entry(array->db, array_index, ret_data);
    if(0 != ret)
    {
        db_table_unlock(array->db);
        return 0;
    }
        
    db_table_free_entry_index(array->db, array_index, NULL, array, TRUE);
	if(array->db->handle_delete)
	{
        ret = (*array->db->handle_delete)(ret_data);
	}
#if 0
    if (0 == ret)
    {
#endif
        db_table_delete_entry(array->db, array_index, NULL, array, TRUE);
#if 0
    }
    else
    {
        db_table_synced_entry(array->db, array_index, DB_DELETE_NOT_SYNCED);
    }
#endif
    
    db_table_unlock(array->db);
    return 0;
}


int dbtable_array_internal_delete(
    array_table_index_t *sequence,
    void *data,
    unsigned int entry_id
)
{
    return 0;
}

int dbtable_array_update(
    array_table_index_t *array,
    unsigned int array_index,
    void *predata,
    void *data
)
{
    unsigned int entry_id;
    int ret = 0;
    void *old_data = NULL;

	if(array == NULL)
	{
		return -1;
	}
    if(!array->db)
        return -1;
	old_data = malloc(array->db->entry_size);
    if (NULL == old_data)
        return -1;

    db_table_lock(array->db);
    entry_id = array_index;
    ret = db_table_get_entry(array->db, entry_id, old_data);

    if (ret == -1)
        goto error;

    ret = db_table_update_entry(array->db, entry_id, data, array, 1);
	if(array->db->handle_update)
	{
        ret = (*array->db->handle_update)(data, old_data);
	}
    if (0 == ret)
    {
        db_table_synced_entry(array->db, entry_id, DB_HW_SW_SYNCED);
    }
    else
    {
        printf("Array %x TABLE %s update unsuccess\r\n", (unsigned int)array, array->db->name);
        db_table_unsynced_entry(array->db, entry_id, DB_HW_SW_SYNCED);
    }
    db_table_unlock(array->db);
    free(old_data);
    return 0;
error:
    db_table_unlock(array->db);
    if(old_data)
        free(old_data);
    return -1;
}

int dbtable_array_show(
	array_table_index_t *array,
	char * string,
	int* string_size,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*show)(void *, char*, int*)
)
{
	unsigned int entry_id = 0,count = 0;
	unsigned int filterMatch = 0;
    void *proc_data = NULL;

	if(NULL == array){
		return 0;
	}
    if(!array->db)
        return -1;
	proc_data = malloc(array->db->entry_size);
    if(NULL == proc_data){        
        return 0;
    }
  
	for(entry_id=0;entry_id<array->db->entry_num;entry_id++) 
    {
        int ret = db_table_get_entry(array->db, entry_id, proc_data);
		if(NULL != filter) { /* specific filter given*/ 
			filterMatch = (*filter)(proc_data,data);				
		}
		else { /* no filter specified*/
     
			filterMatch = TRUE;
		}

		if(TRUE == filterMatch) count++;
		
		if((TRUE == filterMatch) && (NULL != show) && ret == 0) {
            
			(*show)(proc_data,string, string_size);
		}
	}

    free(proc_data);

	return count;
}

#if 0
btree_bucket_t * dbbtree_bucket_new
(
    unsigned int index
)
{
    btree_bucket_t * bucket = NULL;

    bucket = malloc(sizeof(btree_bucket_t));

    if (!bucket)
    {
        return NULL;
    }

    bucket->index = index;
    bucket->l_son_node = NULL;
    bucket->r_son_node = NULL;
    bucket->father = NULL;
    bucket->next = NULL;
    bucket->empty = FALSE;

    return bucket;
}

int dbtable_btree_insert(
    btree_table_index_t *btree,
    void *data
)
{
    unsigned int entry_id = -1, entry_father_id = -1;
    btree_bucket_t *bucket_new = NULL, *bucket_new_father = NULL, **mp = NULL;
    int judge_dir = 0, judge_level = 0;
    char *cmpdata = malloc(btree->common.db->entry_size);
    char *father = malloc(btree->common.db->entry_size);
    int ret = -1;

    if ((NULL == cmpdata) || (NULL == father))
    {
        if(cmpdata)
            free(cmpdata);
        if(father)
            free(father);
        return -1;
    }
    db_table_lock(btree->common.db);
    
    entry_id = osal_table_entry_id_alloc(btree->common.db);
    if (-1 == entry_id)
        goto error;
    
    bucket_new = dbbtree_bucket_new(entry_id);
    if(NULL == bucket_new)
        goto error;
    
    
    mp = &btree->btree.top;

    while(*mp != NULL)
    {
        db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
        judge_level = (*btree->btree.btree_level_cmp)(
            (*btree->btree.btree_key)(data), 
            (*btree->btree.btree_key)(cmpdata));
        judge_dir = (*btree->btree.btree_key_cmp)(
            (*btree->btree.btree_key)(data), 
            (*btree->btree.btree_key)(cmpdata));
        if((judge_level == DB_BTREE_LEVEL_LESS)
            || (judge_level == DB_BTREE_LEVEL_EQ))
        {
            switch(judge_dir)
            {
            case DB_BTREE_BROTHER:
                {
                    int judge_new_dir;
                    
                    entry_father_id = osal_table_entry_id_alloc(btree->common.db);
                    if (-1 == entry_id)
                        goto error;
                    
                    bucket_new_father = dbbtree_bucket_new(entry_id);
                   
                    if(NULL == bucket_new_father)
                        goto error;

                    bucket_new_father->empty = TRUE;
                    (*btree->btree.btree_new_father)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata), 
                            (*btree->btree.btree_key)(father));
                    ret = db_table_insert_entry(btree->common.db,
                        entry_father_id, father, &btree->common, TRUE);
                    if(0 != ret)
                        goto error;
                    ret = db_table_insert_entry(btree->common.db, 
                        entry_id, bucket_new, &btree->common, TRUE);
                    if(0 != ret)
                    {
                        db_table_delete_entry(btree->common.db,
                            entry_father_id, father, &btree->common, TRUE);
                        goto error;
                    }

                    judge_new_dir = (*btree->btree.btree_key_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(father) 
                            );
                    
                    if(judge_new_dir == DB_BTREE_LEFT)
                    {
                        bucket_new_father->l_son_node = bucket_new;
                        bucket_new_father->r_son_node = *mp;
                    }
                    else if(judge_new_dir == DB_BTREE_RIGHT)
                    {
                        bucket_new_father->r_son_node = bucket_new;
                        bucket_new_father->l_son_node = *mp;
                    }
                    else
                        assert(0);
                    bucket_new_father->father = (*mp)->father;
                    bucket_new->father = bucket_new_father;
                    (*mp)->father = bucket_new_father;
                    *mp = bucket_new_father;
                    goto success;
                }
                break;
            case DB_BTREE_LEFT:
                bucket_new->father = (*mp)->father;
                bucket_new->r_son_node = (*mp);
                *mp = bucket_new;
                ret = db_table_insert_entry(btree->common.db,
                    entry_id, bucket_new, &btree->common, TRUE);
                goto success;
                break;
            case DB_BTREE_RIGHT:
                bucket_new->father = (*mp)->father;
                bucket_new->l_son_node = (*mp);
                *mp = bucket_new;
                ret = db_table_insert_entry(btree->common.db,
                    entry_id, bucket_new, &btree->common, TRUE);
                goto success;
                break;
            case DB_BTREE_SELF:
                while(*mp)
                {
                    ret = db_table_get_entry(btree->common.db, (*mp)->index, cmpdata);
                    if(0 != ret)
                        goto error;
                    if((*btree->btree.btree_data_cmp)(data, cmpdata))
                    {
                        ret = db_table_update_entry(btree->common.db,
                            (*mp)->index, data, &btree->common, TRUE);
                        if(0 != ret)
                            goto error;
                        osal_table_entry_id_free(btree->common.db, entry_id);
                        free(bucket_new);
                        bucket_new = NULL;
                        if(btree->common.db->handle_update)
                        {
                            int ret;
                            db_table_unlock(btree->common.db);
                            ret = (*btree->common.db->handle_update)(data, cmpdata);
                            if(0 == ret)
                            {
                                db_table_lock(btree->common.db);
                                db_table_synced_entry(btree->common.db, (*mp)->index, DB_HW_SW_SYNCED);
                                db_table_unlock(btree->common.db);
                            }
                        }
        
                        if(father)
                            free(father);
                        if(cmpdata)
                            free(cmpdata);
                        return 0;
                    }
                    if(btree->btree.btree_data_list_cmp)
                    {
                        if((*btree->btree.btree_data_list_cmp)(data, cmpdata)
                             == DB_BTREE_PREV)
                        {
                            break;
                        }
                        else
                        {
                            mp = &((*mp)->next);
                        }
                            
                    }
                }
                bucket_new->father = (*mp)->father;
                bucket_new->l_son_node = (*mp)->l_son_node;
                bucket_new->r_son_node = (*mp)->r_son_node;
                bucket_new->next = *mp;
                *mp = bucket_new;
                ret = db_table_insert_entry(btree->common.db,
                    entry_id, data, &btree->common, TRUE);
                
                if(ret != 0)
                    goto error;
                goto success;
            default:
                assert(0);
                goto error;
            }  
        }

        switch(judge_dir)
        {
            case DB_BTREE_LEFT:
                bucket_new->father = *mp;
                mp = &((*mp)->l_son_node);
                break;
            case DB_BTREE_RIGHT:
                bucket_new->father = *mp;
                mp = &((*mp)->r_son_node);
                break;
            case DB_BTREE_SELF:
            default:
                assert(0);
                goto error;
        }  

    }
    bucket_new->father = *mp;
    *mp = bucket_new;
    ret = db_table_insert_entry(btree->common.db, entry_id, data, 
        &btree->common, TRUE);

success:
    db_table_unlock(btree->common.db);
    if(btree->common.db->handle_insert)
    {
        int ret;
        ret = (*btree->common.db->handle_insert)(data);
        if(0 == ret)
        {
            db_table_lock(btree->common.db);
            db_table_synced_entry(btree->common.db, entry_id, DB_HW_SW_SYNCED);
            db_table_unlock(btree->common.db);
        }
    }
    
    if(father)
        free(father);
    if(cmpdata)
        free(cmpdata);
    return 0;
error:
    if(entry_id != -1)
        osal_table_entry_id_free(btree->common.db, entry_id);
    if(bucket_new)
        free(bucket_new);
    if(entry_father_id != -1)
        osal_table_entry_id_free(btree->common.db, entry_father_id);
    if(bucket_new_father)
        free(bucket_new_father);
    db_table_unlock(btree->common.db);
    if(father)
        free(father);
    if(cmpdata)
        free(cmpdata);
    return ret;
}

int dbtable_btree_update(
    btree_table_index_t *btree,
    void *predata,
    void *data
)
{
    return dbtable_btree_insert(btree, data);
}

int dbtable_btree_search(
    btree_table_index_t *btree,
    void *data,
    void *ret_data
    )
{
    unsigned int entry_id = -1;
    btree_bucket_t *bucket_exist = NULL, **mp = NULL;
    int judge_dir = 0, judge_level = 0;
    char *cmpdata = malloc(btree->common.db->entry_size);
    int ret = -1;

    if (NULL == cmpdata)
    {
        if(cmpdata)
            free(cmpdata);
        return -1;
    }

    db_table_lock(btree->common.db);
    
    mp = &btree->btree.top;

    while(*mp != NULL)
    {
        ret = db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
        if(0 != ret)
            goto retcode;
        judge_level = (*btree->btree.btree_level_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        judge_dir = (*btree->btree.btree_key_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        if(judge_level == DB_BTREE_LEVEL_GREAT)
        {
            ret = -1;
            goto retcode;
        }
        if(judge_level == DB_BTREE_LEVEL_EQ)
        {
            switch(judge_dir)
            {
            case DB_BTREE_BROTHER:
            case DB_BTREE_LEFT:
            case DB_BTREE_RIGHT:
                ret = -1;
                goto retcode;
            case DB_BTREE_SELF:
                if((*btree->btree.btree_data_cmp)(data, cmpdata))
                {
                    ret = db_table_get_entry(btree->common.db, 
                        (*mp)->index, ret_data);
                    goto retcode;

                }
                else
                {
                    *mp = (*mp)->next;
                    while(*mp)
                    {
                        ret = db_table_get_entry(btree->common.db, 
                            (*mp)->index, ret_data); 
                        if(0 != ret)
                            goto retcode;
                        if((*btree->btree.btree_data_cmp)(data, cmpdata))
                        {
                            goto retcode;
                        }
                        *mp = (*mp)->next;
                    }
                    goto retcode;
                }
                break;
            default:
                ret = -1;
                goto retcode;
            }  
        }

        switch(judge_dir)
        {
            case DB_BTREE_LEFT:
                mp = &((*mp)->l_son_node);
                break;
            case DB_BTREE_RIGHT:
                mp = &((*mp)->r_son_node);
                break;
            case DB_BTREE_SELF:
            default:
                ret = -1;
                goto retcode;
        }  
    }

retcode:   
    db_table_unlock(btree->common.db);
    if(cmpdata)
        free(cmpdata);
    return ret;
}

enum
{
    BTREE_TRAV_FINISH_NONE,
    BTREE_TRAV_FINISH_LEFT,
    BTREE_TRAV_FINISH_ALL
};

/*only do left traversal*/
int dbtable_btree_traversal(
    btree_table_index_t *btree,
	void * pri,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*processor)(btree_table_index_t*, void *, void*)
    )
{
    unsigned int entry_id = -1;
    btree_bucket_t *bucket_exist = NULL, *mp = NULL;
    char *cmpdata = malloc(btree->common.db->entry_size);
    int state = BTREE_TRAV_FINISH_NONE;
    int ret = -1;
    int filterMatch = FALSE;
    int count = 0;

    if (NULL == cmpdata)
    {
        if(cmpdata)
            free(cmpdata);
        return -1;
    }

    db_table_lock(btree->common.db);
    
    mp = btree->btree.top;

    while(mp != NULL)
    {
        switch(state)
        {
        case BTREE_TRAV_FINISH_NONE:
            if(mp->l_son_node)
            {
                mp = mp->l_son_node;
                continue;
            }
            else if(mp->r_son_node)
            {
                state = BTREE_TRAV_FINISH_LEFT;
                mp = mp->r_son_node;
                continue;
            }
            else
            {
                /*only set state, processing will be next loop*/
                state = BTREE_TRAV_FINISH_ALL;
                continue;
            }
            break;
        case BTREE_TRAV_FINISH_LEFT:
            if(mp->r_son_node)
            {
                mp = mp->r_son_node;
                state = BTREE_TRAV_FINISH_NONE;
                continue;
            }
            else
            { 
                /*only set state, processing will be next loop*/
                state = BTREE_TRAV_FINISH_ALL;
                continue;
            }
            break;
        case BTREE_TRAV_FINISH_ALL:

            bucket_exist = mp;
            if(mp->father->l_son_node == mp)
                state = BTREE_TRAV_FINISH_LEFT;
            else if(mp->father->r_son_node == mp)
                state = BTREE_TRAV_FINISH_ALL;
            mp = mp->father;
            while(bucket_exist)
            {
                btree_bucket_t *next = bucket_exist->next;

                db_table_get_entry(btree->common.db, bucket_exist->index, cmpdata);
                if (NULL != filter)  /* specific filter given*/
                {
                    filterMatch = (*filter)(cmpdata,data);
                }
                else   /* no filter specified*/
                {
                    filterMatch = TRUE;
                }

                if (TRUE == filterMatch) count++;

                if ((TRUE == filterMatch) && (NULL != processor))
                {
                    (*processor)(btree, cmpdata,pri);
                    
                }
                bucket_exist = next;
            }
            break;
        default:
            assert(0);
            break;
            
        }
    }

    return count;    
}


int dbtable_btree_top(
    btree_table_index_t *btree,
    void *data
    )
{
    btree_bucket_t **mp;
    int ret = -1;
    
    db_table_lock(btree->common.db);
    mp = &btree->btree.top;

    if(*mp)
    {
        ret = db_table_get_entry(btree->common.db, 
              (*mp)->index, data);
    }

    db_table_unlock(btree->common.db);

    return ret;
    
}

int dbtable_btree_next_node(
    btree_table_index_t *btree,
    void *data,
    void *ret_data
    )
{
    unsigned int entry_id = -1;
    btree_bucket_t *bucket_exist = NULL, **mp = NULL;
    int judge_dir = 0, judge_level = 0;
    char *cmpdata = malloc(btree->common.db->entry_size);
    int ret = -1;

    if (NULL == cmpdata)
    {
        if(cmpdata)
            free(cmpdata);
        return -1;
    }

    db_table_lock(btree->common.db);
    
    mp = &btree->btree.top;

    while(*mp != NULL)
    {
        ret = db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
        if(0 != ret)
            goto error;
        judge_level = (*btree->btree.btree_level_cmp)(
                             (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        judge_dir = (*btree->btree.btree_key_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        if(judge_level == DB_BTREE_LEVEL_GREAT)
        {
            goto error;
        }
        if(judge_level == DB_BTREE_LEVEL_EQ)
        {
            switch(judge_dir)
            {
            case DB_BTREE_BROTHER:
            case DB_BTREE_LEFT:
            case DB_BTREE_RIGHT:
                goto error;
            case DB_BTREE_SELF:
                {
                    if((*mp)->l_son_node)
                    {
                        ret = db_table_get_entry(btree->common.db, 
                             (*mp)->l_son_node->index, ret_data);
                    }
                    else if((*mp)->r_son_node)
                    {
                        ret = db_table_get_entry(btree->common.db, 
                             (*mp)->r_son_node->index, ret_data);
                    }
                    else
                    {
                        while((*mp)->father)
                        {
                            if((*mp)->father->l_son_node == (*mp)
                                && (*mp)->father->r_son_node)
                            {
                                ret = db_table_get_entry(btree->common.db,
                                    (*mp)->father->r_son_node->index, ret_data);
                                break;
                            }
                            mp = &((*mp)->father);
                        }
                    }
                    if(0 != ret)
                        goto error;
                    goto success;

                }
                break;
            default:
                assert(0);
                goto error;
            }  
        }

        switch(judge_dir)
        {
            case DB_BTREE_LEFT:
                mp = &((*mp)->l_son_node);
                break;
            case DB_BTREE_RIGHT:
                mp = &((*mp)->r_son_node);
                break;
            case DB_BTREE_SELF:
            default:
                assert(0);
                goto error;
        }  

    }
    goto error;
success:   
    db_table_unlock(btree->common.db);
    if(cmpdata)
        free(cmpdata);
    return 0;
error:
    db_table_unlock(btree->common.db);
    if(cmpdata)
        free(cmpdata);
    return ret;    
    
}

int dbtable_btree_next_entry(
    btree_table_index_t *btree,
    void *data,
    void *ret_data
    )
{
    unsigned int entry_id = -1;
    btree_bucket_t *bucket_exist = NULL, **mp = NULL;
    int judge_dir = 0, judge_level = 0;
    char *cmpdata = malloc(btree->common.db->entry_size);
    int ret = -1;

    if (NULL == cmpdata)
    {
        if(cmpdata)
            free(cmpdata);
        return -1;
    }

    db_table_lock(btree->common.db);
    
    mp = &btree->btree.top;

    while(*mp != NULL)
    {
        ret = db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
        if(0 != ret)
            goto error;
        judge_level = (*btree->btree.btree_level_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        judge_dir = (*btree->btree.btree_key_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        if(judge_level == DB_BTREE_LEVEL_GREAT)
        {
            goto error;
        }
        if(judge_level == DB_BTREE_LEVEL_EQ)
        {
            switch(judge_dir)
            {
            case DB_BTREE_BROTHER:
            case DB_BTREE_LEFT:
            case DB_BTREE_RIGHT:
                goto error;
            case DB_BTREE_SELF:
                bucket_exist = (*mp);
                while(bucket_exist)
                {
                    ret = db_table_get_entry(btree->common.db, 
                        bucket_exist->index, cmpdata); 
                    if(0 != ret)
                        goto error;
                    if((*btree->btree.btree_data_cmp)(data, cmpdata))
                    {
                        if(bucket_exist->next)
                        {
                            ret = db_table_get_entry(btree->common.db,
                              bucket_exist->next->index, ret_data);
                            if(0 == ret)
                                goto success;
                            else
                                goto error;
                        }
                        else
                            goto error;
                    }
                    bucket_exist = bucket_exist->next;
                }
                if(!bucket_exist)
                     goto error;


                break;
            default:
                assert(0);
                goto error;
            }  
        }

        switch(judge_dir)
        {
            case DB_BTREE_LEFT:
                mp = &((*mp)->l_son_node);
                break;
            case DB_BTREE_RIGHT:
                mp = &((*mp)->r_son_node);
                break;
            case DB_BTREE_SELF:
            default:
                assert(0);
                goto error;
        }  

    }

success:   
    db_table_unlock(btree->common.db);
    if(cmpdata)
        free(cmpdata);
    return 0;
error:
    db_table_unlock(btree->common.db);
    if(cmpdata)
        free(cmpdata);
    return ret;    
}


int dbtable_btree_delete(
    btree_table_index_t *btree,
    void *data
)
{
    unsigned int entry_id = -1;
    btree_bucket_t *bucket_exist = NULL, **mp = NULL;
    int judge_dir = 0, judge_level = 0;
    char *cmpdata = malloc(btree->common.db->entry_size);
    int ret = -1;

    if (NULL == cmpdata)
    {
        if(cmpdata)
            free(cmpdata);
        return -1;
    }
    db_table_lock(btree->common.db);
    
    mp = &btree->btree.top;

    while(*mp != NULL)
    {
        db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
        judge_level = (*btree->btree.btree_level_cmp)(
                         (*btree->btree.btree_key)(data), 
                         (*btree->btree.btree_key)(cmpdata) 
                         );
        judge_dir = (*btree->btree.btree_key_cmp)(
                         (*btree->btree.btree_key)(data), 
                         (*btree->btree.btree_key)(cmpdata) 
                         );
        if(judge_level == DB_BTREE_LEVEL_GREAT)
        {
            goto success;
        }
        if(judge_level == DB_BTREE_LEVEL_EQ)
        {
            switch(judge_dir)
            {
            case DB_BTREE_BROTHER:
            case DB_BTREE_LEFT:
            case DB_BTREE_RIGHT:
                goto error;
            case DB_BTREE_SELF:
                if((*btree->btree.btree_data_cmp)(data, cmpdata))
                {
                    if((*mp)->next)
                    {
                        bucket_exist = *mp;
                        (*mp)->next->l_son_node = (*mp)->l_son_node;
                        (*mp)->next->r_son_node = (*mp)->r_son_node;
                        (*mp)->next->father = (*mp)->father;
                        *mp = (*mp)->next;
                        db_table_free_entry_index(btree->common.db, bucket_exist->index, 
                              cmpdata, &btree->common, TRUE);
                        goto success;
                    }
                    else
                    {
                        int judge_father_dir = -1, judge_father_level = -1;
                        int entry_lson_id,entry_rson_id;

                        bucket_exist = *mp;
                        if((!(*mp)->l_son_node) && (!(*mp)->r_son_node))
                        {
                            /*also delete empty father, only one father can be deleted*/
                            btree_bucket_t **del_father = NULL, *del_grandpa=NULL;
                            if((*mp)->father)
                                del_grandpa = (*mp)->father->father;

                            if(del_grandpa)
                            {
                                if((*mp)->father == del_grandpa->l_son_node)
                                    del_father = &(del_grandpa->l_son_node);
                                else if((*mp)->father == del_grandpa->r_son_node)
                                    del_father = &(del_grandpa->r_son_node);
                                else
                                    assert(0);
                            }
                            
                            *mp = NULL;

                            if((del_father) && (*del_father)->empty)
                            {
                                btree_bucket_t *bucket_father = *del_father;
                                if(!(*del_father)->l_son_node)
                                {
                                    (*del_father)->r_son_node->father 
                                                = (*del_father)->father;
                                    *del_father = (*del_father)->r_son_node;
                                    db_table_delete_entry(btree->common.db, bucket_father->index, 
                                        cmpdata, &btree->common, TRUE);
                                    free(bucket_father);
                                }
                                else if(!(*del_father)->r_son_node)
                                {
                                    (*del_father)->l_son_node->father 
                                                = (*del_father)->father;
                                    *del_father = (*del_father)->l_son_node;
                                    db_table_delete_entry(btree->common.db, bucket_father->index, 
                                        cmpdata, &btree->common, TRUE);
                                    free(bucket_father);
                                    
                                }
                            }
                            db_table_free_entry_index(btree->common.db, bucket_exist->index, 
                              cmpdata, &btree->common, TRUE);
                                
                            goto success;
                        }
                        else if (!(*mp)->l_son_node)
                        {
                            (*mp)->r_son_node->father = (*mp)->father;
                            *mp = (*mp)->r_son_node;
                            db_table_free_entry_index(btree->common.db, bucket_exist->index, 
                                  cmpdata, &btree->common, TRUE);
                            goto success;
                        }
                        else if(!(*mp)->r_son_node)
                        {
                            (*mp)->l_son_node->father = (*mp)->father;
                            *mp = (*mp)->l_son_node;
                            db_table_free_entry_index(btree->common.db, bucket_exist->index, 
                              cmpdata, &btree->common, TRUE);
                            goto success;
                        }
                        else
                        {
                            (*btree->btree.btree_empty_father)(cmpdata);
                            db_table_update_entry(btree->common.db, (*mp)->index, cmpdata, 
                                &btree->common, TRUE);
                            (*mp)->empty = TRUE;
                            if(btree->common.db->handle_update)
                            {
                                int ret;
                                db_table_unlock(btree->common.db);
                                ret = (*btree->common.db->handle_update)(cmpdata, data);
                                if(0 == ret)
                                {
                                    db_table_lock(btree->common.db);
                                    db_table_synced_entry(btree->common.db, 
                                        (*mp)->index, DB_HW_SW_SYNCED);
                                    db_table_unlock(btree->common.db);
                                }
                            }
                            if(cmpdata)
                                free(cmpdata);
                            return 0;
                                
                        }
                        
                    }
                }
                else
                {
                    *mp = (*mp)->next;
                    while(*mp)
                    {
                        db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
                        if((*btree->btree.btree_data_cmp)(data, cmpdata))
                        {
                            bucket_exist = *mp;
                            *mp = (*mp)->next;
                            db_table_free_entry_index(btree->common.db, bucket_exist->index, 
                              cmpdata, &btree->common, TRUE);
                            goto success;
                        }
                        *mp = (*mp)->next;
                    }
                    goto success;
                }
                break;
            default:
                assert(0);
                goto error;
            }  
        }

        switch(judge_dir)
        {
            case DB_BTREE_LEFT:
                mp = &((*mp)->l_son_node);
                break;
            case DB_BTREE_RIGHT:
                mp = &((*mp)->r_son_node);
                break;
            case DB_BTREE_SELF:
            default:
                assert(0);
                goto error;
        }  

    }

success: 
    if(bucket_exist)
    {
        if(btree->common.db->handle_delete)
        {
            int ret;
            db_table_unlock(btree->common.db);
            ret = (*btree->common.db->handle_delete)(data);
            db_table_lock(btree->common.db);
            if(0 != ret)
            {
                db_table_synced_entry(btree->common.db, entry_id, DB_DELETE_NOT_SYNCED);
            }
            else
            {
                db_table_delete_entry(btree->common.db, bucket_exist->index, 
                    cmpdata, &btree->common, TRUE);
            }
        }
        free(bucket_exist);
    }
        
    db_table_unlock(btree->common.db);
    if(cmpdata)
        free(cmpdata);
    return 0;
error:
    db_table_unlock(btree->common.db);
    if(cmpdata)
        free(cmpdata);
    return ret;
}


int dbtable_btree_internal_insert(
    btree_table_index_t *btree,
    void *data,
    unsigned int entry_id
)
{
    unsigned int index;
    btree_bucket_t *bucket_new, **bucket_exist, **mp;
    int judge_dir = 0, judge_level = 0;
    char *cmpdata = malloc(btree->common.db->entry_size);

    if (NULL == cmpdata)
    {
        return -1;
    }

    bucket_new = dbbtree_bucket_new(entry_id);
    if(NULL == bucket_new)
    {
        osal_table_entry_id_free(btree->common.db, entry_id);
        free(cmpdata);
        return -1;
    }

    mp = &btree->btree.top;

    while(*mp != NULL)
    {
        int index;
        int inserted = FALSE;
        
        db_table_get_entry(btree->common.db, (*mp)->index, cmpdata);
        judge_level = (*btree->btree.btree_level_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        judge_dir = (*btree->btree.btree_key_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        if(judge_level == DB_BTREE_LEVEL_LESS)
        {
            switch(judge_dir)
            {
                case DB_BTREE_LEFT:
                    bucket_new->father = (*mp)->father;
                    bucket_new->r_son_node = (*mp);
                    break;
                case DB_BTREE_RIGHT:
                    bucket_new->father = (*mp)->father;
                    bucket_new->r_son_node = (*mp);
                    break;
                case DB_BTREE_SELF:
                    bucket_new->father = (*mp)->father;
                    bucket_new->l_son_node = (*mp)->l_son_node;
                    bucket_new->r_son_node = (*mp)->l_son_node;
                    bucket_new->next = (*mp);
                    break;
                default:
                    goto error;
            }  
            *mp = bucket_new;
            break;
        }
        switch(judge_dir)
        {
            case DB_BTREE_LEFT:
                bucket_new->father = *mp;
                mp = &((*mp)->l_son_node);
                break;
            case DB_BTREE_RIGHT:
                bucket_new->father = *mp;
                mp = &((*mp)->r_son_node);
                break;
            case DB_BTREE_SELF:
                bucket_new->father = (*mp)->father;
                bucket_new->l_son_node = (*mp)->l_son_node;
                bucket_new->r_son_node = (*mp)->l_son_node;
                bucket_new->next = (*mp);
                break;
            default:
                goto error;
        }  

    }

    *mp = bucket_new;
    if(cmpdata)
        free(cmpdata);
    return 0;
error:
    if(cmpdata)
        free(cmpdata);
    return -1;
}


int dbtable_btree_internal_delete(
    btree_table_index_t *btree,
    void *data,
    unsigned int id
)
{
    unsigned int entry_id = -1;
    btree_bucket_t *bucket_exist = NULL, **mp = NULL;
    int judge_dir = 0, judge_level = 0;
    char *cmpdata = malloc(btree->common.db->entry_size);
    int ret = -1;

    if (NULL == cmpdata)
    {
        if(cmpdata)
            free(cmpdata);
        return -1;
    }

    mp = &btree->btree.top;

    while(*mp != NULL)
    {
        db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
        judge_level = (*btree->btree.btree_level_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        judge_dir = (*btree->btree.btree_key_cmp)(
                            (*btree->btree.btree_key)(data), 
                            (*btree->btree.btree_key)(cmpdata) 
                            );
        if(judge_level == DB_BTREE_LEVEL_GREAT)
        {
            goto success;
        }
        if(judge_level == DB_BTREE_LEVEL_EQ)
        {
            switch(judge_dir)
            {
            case DB_BTREE_BROTHER:
            case DB_BTREE_LEFT:
            case DB_BTREE_RIGHT:
                goto error;
            case DB_BTREE_SELF:
                if((*btree->btree.btree_data_cmp)(data, cmpdata))
                {
                    if((*mp)->next)
                    {
                        bucket_exist = *mp;
                        (*mp)->next->l_son_node = (*mp)->l_son_node;
                        (*mp)->next->r_son_node = (*mp)->r_son_node;
                        (*mp)->next->father = (*mp)->father;
                        *mp = (*mp)->next;
                        goto success;
                    }
                    else
                    {
                        int judge_father_dir = -1, judge_father_level = -1;
                        int entry_lson_id,entry_rson_id;

                        bucket_exist = *mp;
                        if((!(*mp)->l_son_node) && (!(*mp)->r_son_node))
                        {
                            *mp = NULL;
                            goto success;
                        }
                        else if (!(*mp)->l_son_node)
                        {
                            (*mp)->r_son_node->father = (*mp)->father;
                            *mp = (*mp)->r_son_node;
                            goto success;
                        }
                        else if(!(*mp)->r_son_node)
                        {
                            (*mp)->l_son_node->father = (*mp)->father;
                            *mp = (*mp)->l_son_node;
                            goto success;
                        }
                        else
                        {
                            goto success;
                        }
                        
                    }
                }
                else
                {
                    *mp = (*mp)->next;
                    while(*mp)
                    {
                        db_table_get_entry(btree->common.db, (*mp)->index, cmpdata); 
                        if((*btree->btree.btree_data_cmp)(data, cmpdata))
                        {
                            bucket_exist = *mp;
                            *mp = (*mp)->next;
                            goto success;
                        }
                        *mp = (*mp)->next;
                    }
                    goto success;
                }
                break;
            default:
                assert(0);
                goto error;
            }  
        }

        switch(judge_dir)
        {
            case DB_BTREE_LEFT:
                mp = &((*mp)->l_son_node);
                break;
            case DB_BTREE_RIGHT:
                mp = &((*mp)->r_son_node);
                break;
            case DB_BTREE_SELF:
            default:
                assert(0);
                goto error;
        }  

    }

success:    
    if(cmpdata)
        free(cmpdata);
    return 0;
error:
    if(cmpdata)
        free(cmpdata);
    return ret;
    
}

int dbtable_create_btree_index(
	char *index_name,
    db_table_t *db,
    void *(*btree_key)(void*),
    int (*btree_key_cmp)(void *, void*),
    int (*btree_data_cmp)(void*, void*),
    int (*btree_level_cmp)(void*, void*),
    int (*btree_new_father)(void*, void*, void*),
    int (*btree_empty_father)(void*),
    int (*btree_data_list_cmp)(void*, void*),
    btree_table_index_t **btree
)
{
    *btree = malloc(sizeof(btree_table_index_t));

    if (NULL == *btree)
    {
        *btree = NULL;
        return -1;
    }
    memset(*btree, 0, sizeof(btree_table_index_t));

    (*btree)->common.db = db;
	strncpy((*btree)->common.index_name, index_name, 31);
	(*btree)->common.index_type = DB_TABLE_INDEX_BTREE;
    (*btree)->common.index_insert = dbtable_btree_internal_insert;
    (*btree)->common.index_delete = dbtable_btree_internal_delete;
    list_add(&((*btree)->common.list), &db->index_list);
    (*btree)->btree.count = 0;
    (*btree)->btree.top = NULL;
    (*btree)->btree.btree_key = btree_key;
    (*btree)->btree.btree_key_cmp = btree_key_cmp;
    (*btree)->btree.btree_data_cmp = btree_data_cmp;
    (*btree)->btree.btree_level_cmp = btree_level_cmp;
    (*btree)->btree.btree_new_father = btree_new_father;
    (*btree)->btree.btree_empty_father = btree_empty_father;
    (*btree)->btree.btree_data_list_cmp = btree_data_list_cmp;
    return 0;
}
#endif

int npd_dbtable_header_ntoh(npd_sync_msg_header_t *header)
{
	if( header == NULL )
		return 0;

	header->version = ntohl(header->version);
	header->type = ntohl(header->type);
	header->op = ntohl(header->op);
	header->tbl_id = ntohl(header->tbl_id);
	header->total_len = ntohl(header->total_len);
	header->entry_len = ntohl(header->entry_len);
	header->sync_len = ntohl(header->sync_len);
	header->start_index = ntohl(header->start_index);
	header->entry_num = ntohl(header->entry_num);

	return 0;
}

int npd_dbtable_header_hton(npd_sync_msg_header_t *header)
{
	if( header == NULL )
		return 0;

	header->version = htonl(header->version);
	header->type = htonl(header->type);
	header->op = htonl(header->op);
	header->tbl_id = htonl(header->tbl_id);
	header->total_len = htonl(header->total_len);
	header->entry_len = htonl(header->entry_len);
	header->sync_len = htonl(header->sync_len);
	header->start_index = htonl(header->start_index);
	header->entry_num = htonl(header->entry_num);

	return 0;
}


int dbtable_recv(int fd, char* buf, int len, void *private_data)
{
    void *data = NULL;
    db_table_t *db;
    npd_sync_msg_header_t *header;
    int op_ret = -1;
    int i;
    int handle_len = 0;
    char *buffer = buf;
    void *predata = NULL;
    unsigned int entry_id;
    unsigned int new_id;

    if (len<0)
    {
        return op_ret;
    }

    if (buffer == 0 || len == 0)
    {
        return op_ret;
    }

    if (len < sizeof(npd_sync_msg_header_t))
        return op_ret;

    while(handle_len < len)
    {
        header = (npd_sync_msg_header_t*)buffer;
		npd_dbtable_header_ntoh(header);
        if (npd_db_debug)
        {
            printf("dttable_recv:%d> dbname:%s, op:%d, len:%d, start entry id:%d, num:%d\r\n",
				header->total_len,header->name, header->op, header->total_len, header->start_index, header->entry_num);
        }
        
        if(header->op == NPD_SYNC_OP_COMPLETE)
        {
			if(platform_db_sync_complete_handler)
			{
				(*platform_db_sync_complete_handler)();
			}
			return 0;
        }
		
        handle_len += header->total_len;
        db_get(header->name, &db);

        if (NULL == db)
        {
            return op_ret;
        }

        data = (void*)(header+1);
        
        if (header->total_len < header->entry_len*header->entry_num+sizeof(*header))
            return op_ret;
        db_table_lock(db);
        for (i = 0; i < header->entry_num; i++)
        {
            entry_id = header->start_index+i;
            if(i != 0)
            {
                data = (void*)((char*)data + header->entry_len);				
            }
			if(entry_id >= db->entry_num)
			{
				db_table_unlock(db);
				return -1;
			}
			if(db->entry_ntoh)
				db->entry_ntoh(data);
             
            switch (header->op)
            {
                case NPD_SYNC_OP_ITEM_ADD:
                case NPD_SYNC_OP_TABLE_ADD:
                    /* the allocated one must be deleted */
                    op_ret = db_table_internal_alloc(db, entry_id, &new_id);
                    if (-1 == op_ret)
                    {
						/*update*/
                        predata = malloc(db->entry_size);
                        if (NULL == predata)
                        {
    						db_table_unlock(db);
                            return -1;
                        }
                        memset(predata, 0, db->entry_size);
    
                        op_ret = db_table_get_entry(db, entry_id, predata);
						if(op_ret == -1)
						{
    						db_table_unlock(db);
                            return -1;
                        }
                        /*only synced data need sync*/
                        memcpy((char*)data+db->sync_size, (char*)predata+db->sync_size, 
                                         db->entry_size-db->sync_size);
						#if 0
						db_table_unsynced_entry(db, entry_id, DB_DELETE_NOT_SYNCED);
						#endif
                        db_table_update_entry(db, entry_id, data, NULL, FALSE);
                        db_table_unlock(db);
    					if(db->handle_update)
    					{
                            op_ret = (*db->handle_update)(data, predata);
    					}
    					if(db->app_handle_update)
    					{
                            op_ret = (*db->app_handle_update)(data, predata);
    					}
                        db_table_lock(db);
                        if (0 == op_ret)
                        {
                            db_table_synced_entry(db, entry_id, DB_HW_SW_SYNCED);
                        }
                        if(predata)
                            free(predata);
    					/*db_table_unlock(db);*/
                        continue;
                    }
                    db_table_insert_entry(db, entry_id, data, NULL, FALSE);
                    db_table_unlock(db);
					if(db->handle_insert)
					{
                        op_ret = (*db->handle_insert)(data);
					}
					if(db->app_handle_update)
					{
                        op_ret = (*db->app_handle_update)(data, data);
					}
                    db_table_lock(db);
                    if (0 == op_ret)
                    {
                        db_table_synced_entry(db, entry_id, DB_HW_SW_SYNCED);
                    }
                    else
                    {
                        db_table_unsynced_entry(db, entry_id, DB_HW_SW_SYNCED);
                    }
                    break;
                case NPD_SYNC_OP_ITEM_DEL:
					op_ret = 0;
                    db_table_unlock(db);
					if(db->handle_delete)
					{
                        op_ret = (*db->handle_delete)(data);
					}
					if(db->app_handle_delete)
					{
                        op_ret = (*db->app_handle_delete)(data);
					}
                    db_table_lock(db);
					#if 0
                    if (0 == op_ret)
                    {
					#endif
                        db_table_synced_entry(db, entry_id, DB_HW_SW_SYNCED);
					#if 0
                    }
                    else
                    {
                        db_table_synced_entry(db, entry_id, DB_DELETE_NOT_SYNCED);
                    }
					#endif
					db_table_free_entry_index(db, entry_id, NULL, NULL, FALSE);
                    db_table_delete_entry(db, entry_id, NULL, NULL, FALSE);
                    break;
                case NPD_SYNC_OP_ITEM_UPDATE:
                    predata = malloc(db->entry_size);
                    memset(predata, 0, db->entry_size);
                    if (NULL == predata)
                    {
						db_table_unlock(db);
                        return -1;
                    }

                    op_ret = db_table_get_entry(db, entry_id, predata);
                    if(0 != op_ret)
                    {
                        db_table_unlock(db);
                        free(predata);
                        return -1;
                    }
                    /*only synced data need sync*/
                    if(db->sync_size != db->entry_size)
                        memcpy((char*)data+db->sync_size, (char*)predata+db->sync_size, 
                                     db->entry_size-db->sync_size);
                    db_table_update_entry(db, entry_id, data, NULL, FALSE);
                    db_table_unlock(db);
					if(db->handle_update)
					{
                        op_ret = (*db->handle_update)(data, predata);
					}
					if(db->app_handle_update)
					{
                        op_ret = (*db->app_handle_update)(data, predata);
					}
                    db_table_lock(db);
                    if (0 == op_ret)
                    {
                        db_table_synced_entry(db, entry_id, DB_HW_SW_SYNCED);
                    }
                    else
                    {
                        db_table_unsynced_entry(db, entry_id, DB_HW_SW_SYNCED);
                    }
                    if(predata)
                        free(predata);
                    break;
                default:
                    break;
            }
        }

        db_table_unlock(db);
		buffer = buf + handle_len;
    }

    return 0;
}


/* keep software and hardware synced */

int dbtable_sync_hwsw_timer(void *data)
{
    struct list_head *pos;
    db_table_t *db = NULL; 
    pthread_mutex_lock(&cfg_db_lock);
    list_for_each(pos, &cfg_db)   
    {
        db = list_entry(pos, db_table_t, list);
        dbentry_common_t *dest;
        int i;
        db_table_read_lock(db);
        /* make sync operation for every table */
        for (i = 0; i < db->entry_num; i++)
        {
            dest = db->entries + i;
            if (dest->flags & DB_ENTRY_EXIST)
            {
                if (!(dest->flags & DB_HW_SW_SYNCED))
                {
    				if(db->handle_insert)
    				{
                        (*db->handle_insert)(dest->real_data);
    				}
                }
            }
            if (dest->flags & DB_DELETE_NOT_SYNCED)
            {
				if(db->handle_delete)
				{
                    (*db->handle_delete)(dest->real_data);
				}
            }
        }
        db_table_read_unlock(db);
    }
    pthread_mutex_unlock(&cfg_db_lock);
    return 0;
}


struct list_head dbtable_type_list = 
    {
        &dbtable_type_list, &dbtable_type_list
    };
hash_table_index_t *dbtable_table = NULL;

int register_table_type(
     int type, 
     create_type_dbtable_t create, 
     destroy_type_dbtable_t destroy
     )
{
    dbtable_table_oper_t *entry;
    
    entry = malloc(sizeof(dbtable_table_oper_t));
    memset(entry, 0, sizeof(*entry));
    entry->table_type = type;
    entry->create_func = create;
    entry->delete_func = destroy;
    list_add(&dbtable_type_list, &entry->list);
    return 0;
}

int create_type_table(
    char *name, 
    int type,
    char data[TABLE_TABLE_DATA_SIZE]
    )
{
    struct list_head *pos;
    dbtable_table_oper_t *entry;

    list_for_each(pos, &dbtable_type_list)
    {
        entry = list_entry(pos, dbtable_table_oper_t, list);
        if(type == entry->table_type)
            return (*entry->create_func)(name, data);
    }
    return -1;
}

int destroy_type_table(
    char *name,
    int type,
    char data[TABLE_TABLE_DATA_SIZE]
    )
{
    struct list_head *pos;
    dbtable_table_oper_t *entry;

    list_for_each(pos, &dbtable_type_list)
    {
        entry = list_entry(pos, dbtable_table_oper_t, list);
        if(type == entry->table_type)
            return (*entry->delete_func)(name, data);
    }
    return -1;
}

long  dbtable_table_insert(void *new)
{
    dbtable_table_t *entry = (dbtable_table_t*)new;

    return create_type_table(entry->table_name, entry->table_type, entry->data);
}

long  dbtable_table_delete(void *new)
{
    dbtable_table_t *entry = (dbtable_table_t*)new;

    return destroy_type_table(entry->table_name, entry->table_type, entry->data);
}

unsigned int dbtable_table_key(void *new)
{
    dbtable_table_t *entry = (dbtable_table_t*)new;
    char * name = entry->table_name;
    unsigned long ret=0;
	long n;
	unsigned long v;
	int r;
    char *c = name;

	if ((c == NULL) || (*c == '\0'))
		return(ret);

	n=0x100;
	while (*c)
		{
		v=n|(*c);
		n+=0x100;
		r= (int)((v>>2)^v)&0x0f;
		ret=(ret<<r)|(ret>>(32-r));
		ret&=0xFFFFFFFFL;
		ret^=v*v;
		c++;
		}
	return((ret>>16)^ret)%1024;
}

unsigned int dbtable_table_cmp(void *data1, void *data2)
{
    dbtable_table_t *entry1 = (dbtable_table_t*)data1;
    dbtable_table_t *entry2 = (dbtable_table_t*)data2;

    return (0 == strcmp(entry1->table_name, entry2->table_name));
}
    

int dbtable_table_init(void)
{
    db_table_t *db;
    char name[MAX_TABLE_NAME_LEN];

    sprintf(name, "dbtbl_tbl");
    create_dbtable(name, 4096, sizeof(dbtable_table_t), 
           NULL, NULL, dbtable_table_insert,
           dbtable_table_delete, NULL, NULL, NULL, NULL, NULL, 
           DB_SYNC_MCU,
           &db);
    if(NULL == db)
    {
        exit(1);
        return -1;
    }
    dbtable_create_hash_index(name, db, 1024, 
         dbtable_table_key, dbtable_table_cmp, &dbtable_table);
    if(NULL == dbtable_table)
    {
        exit(1);
        return -1;
    }
    return 0;
}

int dbtable_table_add(
     char *name, 
     int type, 
     char data[TABLE_TABLE_DATA_SIZE]
     )
{
    dbtable_table_t entry;
    int ret;

    memset(&entry, 0, sizeof(dbtable_table_t));
    strncpy(entry.table_name, name, sizeof(entry.table_name));
    entry.table_type = type;
    memcpy(entry.data, data, TABLE_TABLE_DATA_SIZE);
    ret = dbtable_hash_insert(dbtable_table, &entry);
    return ret;
}

int dbtable_table_del(
    char *name
    )
{
    dbtable_table_t entry;
    int ret;
    memset(&entry, 0, sizeof(dbtable_table_t));
    strncpy(entry.table_name, name, sizeof(entry.table_name));
    ret = dbtable_hash_delete(dbtable_table, &entry, &entry);
    return ret;
}

int dbtable_table_show_func_install(db_table_t *db, dbtable_show_func func)
{
	db_table_show *tmp_db_show = NULL;
    if(db == NULL)
    {
        return -1;
    }
	tmp_db_show = malloc(sizeof(db_table_show));
	if(tmp_db_show == NULL)
	{
		return -1;
	}
	tmp_db_show->dbtable = db;
	tmp_db_show->func = func;
	list_add(&tmp_db_show->list, &db_show_list);
    return 0;
}

dbtable_show_func dbtable_table_show_func_get(db_table_t *db)
{
    struct list_head *pos;
	db_table_show *tmp_db_show = NULL;
    if(db == NULL)
    {
        return NULL;
    }
    list_for_each(pos, &db_show_list)   
    {
        tmp_db_show = list_entry(pos, db_table_show, list);
        if(tmp_db_show->dbtable == NULL)
        {
            continue;
        }
        if ((db == tmp_db_show->dbtable) || (strncmp(db->name, tmp_db_show->dbtable->name, MAX_TABLE_NAME_LEN) == 0))
        {
            return tmp_db_show->func;
        }
    }
    return NULL;
}

int dbtable_table_show(char *name, char **show_buf, int print_header)
{
    db_table_t *db;
	char *tmp_buf = NULL, *realloc_buf = NULL;
	int current_ptr = 0;
	int total_len = 0;
	void *entry_buf = NULL;
	int i = 0, n = 1;
	int ret = 0;
	dbtable_show_func dbtable_table_show_func = NULL;
	*show_buf = NULL;
	if(name == NULL || show_buf == NULL)
	{
	    return -1;
	}
    db_get(name, &db);

    if (NULL == db)
    {
        return DB_TABLE_RETURN_CODE_DB_NOT_EXIST;
    }
	dbtable_table_show_func = dbtable_table_show_func_get(db);
	if(dbtable_table_show_func == NULL)
	{
        return -1;
	}
	entry_buf = malloc(db->entry_size);
	if(entry_buf == NULL)
	{
        return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
    tmp_buf = malloc(DEFAULT_DBTABLE_SHOW_BUFFER_LENGTH);
	if(tmp_buf == NULL)
	{
	    free(entry_buf);
        return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
	memset(tmp_buf, 0, DEFAULT_DBTABLE_SHOW_BUFFER_LENGTH);
	if(print_header == 1)
	{
	    total_len += sprintf(tmp_buf + current_ptr, "DB Table Name: %s, entry used: %d\n", db->name, db->entry_use_num);
		current_ptr = total_len;
	    total_len += sprintf(tmp_buf + current_ptr, "----------------------------------------------------------------\n");
		current_ptr = total_len;
	    ret = dbtable_table_show_func(NULL, tmp_buf + current_ptr, (n*DEFAULT_DBTABLE_SHOW_BUFFER_LENGTH) - total_len);
		if(ret >= 0)
		{
		    total_len += ret;
			current_ptr = total_len;
		}
	}
	for(i = 0; i < db->entry_num; i++)
	{
	    ret = db_table_get_entry(db, i, entry_buf);
	    if(ret == 0)
	    {
	        ret = dbtable_table_show_func(entry_buf, tmp_buf + current_ptr, (n*DEFAULT_DBTABLE_SHOW_BUFFER_LENGTH) - total_len);
			if(ret < 0)
			{
			    n++;
			    realloc_buf = realloc(tmp_buf, (n*DEFAULT_DBTABLE_SHOW_BUFFER_LENGTH));
				if(realloc_buf == NULL)
				{
				    free(entry_buf);
					free(tmp_buf);
					return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
				}
				tmp_buf = realloc_buf;
	            ret = dbtable_table_show_func(entry_buf, tmp_buf + current_ptr, (n*DEFAULT_DBTABLE_SHOW_BUFFER_LENGTH) - total_len);
				if(ret < 0)
				{
				    free(entry_buf);
					free(tmp_buf);
					return DB_TABLE_RETURN_CODE_CALLBACK_ERR;
				}
			}
			total_len += ret;
			current_ptr = total_len;
	    }
	}
	*show_buf = tmp_buf;
	return 0;
}


#ifdef __cplusplus
}
#endif

