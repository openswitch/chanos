#ifndef _NPD_DATABASE_

#include "npd/npd_list.h"
#define _DB_MUTEX_

#define _NPD_DATABASE_
#define MAX_SEND_SIZE 20000
#define MAX_DB_NAME_LEN 31
#define MAX_TABLE_NAME_LEN MAX_DB_NAME_LEN
#define MAX_INDEX_NAME_LEN MAX_DB_NAME_LEN
#define DB_ENTRY_EXIST              (1<<0)
#define DB_HW_SW_SYNCED             (1<<1)
#define DB_MASTER_SLAVE_SYNCED      (1<<2)
#define DB_DELETE_NOT_SYNCED        (1<<3)

#ifdef _DB_RWLOCK_
typedef struct
{
    pthread_mutex_t wait;
    pthread_mutex_t access;
    pthread_mutex_t atomic;
    long volatile   readcnt;
} npd_rwlock_t;
#endif

typedef struct dbentry_common_s
{
    unsigned int flags;
    unsigned int modifier_pri; /**/
    unsigned int modify_counter;  /**/
	void *real_data;
}dbentry_common_t;

#define DB_SYNC_ALL      (1<<0) /*sync to all*/
#define DB_SYNC_SERVICE  (1<<1) /*sync only to service*/
#define DB_SYNC_MCU       (1<<2) /* sync only to mcu*/
#define DB_SYNC_NONE        (1<<3)
#define DB_SYNC_ASYNC		(1<<4)
typedef struct _ENTRY_ALLOC_LIST_
{
    struct list_head list;
	unsigned int entry_index;
}ENTRY_ALLOC_LIST;

typedef struct db_element_s
{
    struct list_head list;
    unsigned int start_offset;
    unsigned int len;
    int sync_flag;
}db_element_t;


typedef struct db_table_s
{
    struct list_head list;
    char name[MAX_TABLE_NAME_LEN];
    dbentry_common_t *entries;
    unsigned int entry_num;
    unsigned int entry_use_num_limit;
    unsigned int entry_use_num;
    unsigned int entry_size;
    unsigned int entry_end;
#ifdef _DB_MUTEX_
    pthread_mutex_t lock;
#endif
#ifdef _DB_RWLOCK_
    npd_rwlock_t lock;
#endif
    unsigned int sync_mode; /*sync to all, sync only to service, sync only to mcu*/
    long (*handle_update)(void *new_entry, void *old_entry);
    long (*app_handle_update)(void *new_entry, void *old_entry);
	long (*app_handle_delete)(void *del_entry);
    long (*handle_insert)(void *entry);
    long (*handle_delete)(void *entry);
    long (*alloc_entry)(struct db_table_s *db, char **entry, int *index);
    long (*free_entry)(struct db_table_s *db, char *entry);
    int (*entry_ntoh)(void *entry);
    int (*entry_hton)(void *entry);
    struct list_head element_list;
    struct list_head index_list;
	struct list_head entry_alloc_list;
    unsigned int sync_size;
}db_table_t;

extern db_table_t sys_database;

typedef struct db_index_common_s
{
    struct list_head list;
	char index_name[32];
    db_table_t *db;
	int index_type;
    int (*index_insert)(struct db_index_common_s*, void *, unsigned int index);
    int (*index_delete)(struct db_index_common_s*, void *, unsigned int index);
}db_index_common_t;

typedef struct dbhash_bucket
{
  int	index;					/* pointer to db entry index*/
  struct dbhash_bucket *next;	/* next hash bucket for key*/
  struct dbhash_bucket *pre;   /* pre hash bucket for key*/
}dbhash_bucket_t;


typedef struct dbhash_s
{
	int 		  size; 			/* Total hash bucket number*/
	int 		  count;		  	/* Total node number within all buckets */
  	struct dbhash_bucket    **index;      	/* Hash bucket pointer  for key1*/
  	unsigned int  (*hash_key)(void *); /* Key make function for key1*/
  	unsigned int           (*hash_cmp)(void *, void *);/* Data compare function*/
}dbhash_t;

typedef struct hash_table_index_s
{
    db_index_common_t common;
    dbhash_t hash_index;
}hash_table_index_t;


typedef struct dbsequence_bucket
{
    unsigned int index;
    struct dbsequence_bucket *next;
    struct dbsequence_bucket *pre;
}dbsequence_bucket_t;


typedef struct dbsequence_s
{
  	int 		  size; 			/* Total sequence bucket number*/
  	struct dbsequence_bucket    **index;      	/* seuqence bucket pointer  for key1*/
  	unsigned int  (*sequence_index)(unsigned int); /* index make function for input index*/
    int (*sequence_cmp)(void*, void*);
    unsigned int (*sequence_key)(void*);
}dbsequence_t;
    
typedef struct sequence_table_index_s
{
    db_index_common_t common;
    dbsequence_t sequence;
}sequence_table_index_t;

typedef db_index_common_t array_table_index_t;

typedef struct btree_bucket_s
{
    int index;
    int empty;
    struct btree_bucket_s *l_son_node;
    struct btree_bucket_s *r_son_node;
    struct btree_bucket_s *father;
    struct btree_bucket_s *next;
}btree_bucket_t;

enum
{
    DB_BTREE_LEFT,
    DB_BTREE_SELF,
    DB_BTREE_RIGHT,
    DB_BTREE_BROTHER
};

enum
{
    DB_BTREE_LEVEL_GREAT,
    DB_BTREE_LEVEL_LESS,
    DB_BTREE_LEVEL_EQ,
    DB_BTREE_LEVEL_IGNORE
};

enum
{
    DB_BTREE_PREV = -1,
    DB_BTREE_EQUAL,
    DB_BTREE_FOLLOW
};

typedef struct dbbtree_s
{
    int count;
    struct btree_bucket_s *top;
    void* (*btree_key)(void *new);
    int (*btree_key_cmp)(void *new, void *exist);
    int (*btree_data_cmp)(void *new, void *exist);
    int (*btree_level_cmp)(void *new, void *exist);
    int (*btree_new_father)(void *son_new, void *son_exist, void *father);
    int (*btree_empty_father)(void *father);
    int (*btree_data_list_cmp)(void *new, void *exist);
}dbbtree_t;

typedef struct btree_table_index_s
{
    db_index_common_t common;
    dbbtree_t btree;
}btree_table_index_t;

typedef enum _DB_TABLE_INDEX_TYPE_
{
	DB_TABLE_INDEX_HASH,
	DB_TABLE_INDEX_SEQ,
	DB_TABLE_INDEX_ARRAY,
	DB_TABLE_INDEX_BTREE,
	DB_TABLE_INDEX_MAX
}DB_TABLE_INDEX_TYPE;

    

typedef enum _npd_sync_type_e_{
	NPD_SYNC_TYPE_TBL,
	NPD_SYNC_TYPE_ENTRY,
	NPD_SYNC_TYPE_MAX
} NPD_SYNC_TYPE;

typedef enum _npd_sync_op_e_ {
	NPD_SYNC_OP_ITEM_ADD,
	NPD_SYNC_OP_ITEM_DEL,
	NPD_SYNC_OP_ITEM_UPDATE,
	NPD_SYNC_OP_TABLE_ADD,
	NPD_SYNC_OP_TBL_SYNC,
	NPD_SYNC_OP_COMPLETE
} NPD_SYNC_OP;

typedef enum _npd_sync_tbl_e_{
	NPD_SYNC_TBL_ETH_PORT_INFO,
	NPD_SYNC_TBL_ETH_PORT_PVLAN,
	NPD_SYNC_TBL_ETH_PORT_DOT1Q,
	NPD_SYNC_TBL_ETH_PORT_TRUNK,
	NPD_SYNC_TBL_ARP_SNOOPING,
	NPD_SYNC_TBL_FDB_BLACKLIST,
	NPD_SYNC_TBL_FDB_STATIC,
	NPD_SYNC_TBL_FDB_DYNAMIC,
	NPD_SYNC_TBL_IGMP_SNP,
	NPD_SYNC_TBL_RSTP,
	NPD_SYNC_TBL_L3INTF,
	NPD_SYNC_TBL_ROUTE,
	NPD_SYNC_TBL_END
} NPD_SYNC_TBL_ID;

typedef struct npd_sync_msg_header_s{
    unsigned int    version;
	NPD_SYNC_TYPE   type;
	NPD_SYNC_OP     op;
	NPD_SYNC_TBL_ID tbl_id;
    char name[MAX_TABLE_NAME_LEN];
	int total_len;
    int entry_len;
    int sync_len;
    unsigned int start_index;
    unsigned int entry_num;
} npd_sync_msg_header_t;

#define TABLE_TABLE_DATA_SIZE 64

typedef struct dbtable_table_s
{
    char table_name[32];
    int table_type;
    char data[TABLE_TABLE_DATA_SIZE];
}dbtable_table_t;

#define DEFAULT_DBTABLE_SHOW_BUFFER_LENGTH 0x10000
/*
函数说明:
entry: 传入NULL,则表示输出db header
buf: 输入的buffer空间
buf_len: buffer空间的可用长度
返回值说明:
小于0:表示内存长度不足
大于0:返回值表示往buffer里新增的内容长度
等于0:表示没有往buffer里新增内容，但是需要继续
*/
typedef int (*dbtable_show_func)(void* entry, char *buf, int buf_len);


typedef struct db_table_show_s
{
    struct list_head list;
	db_table_t *dbtable;
	dbtable_show_func func;
}db_table_show;


typedef int (*create_type_dbtable_t)(char *name, char data[TABLE_TABLE_DATA_SIZE]);
typedef int (*destroy_type_dbtable_t)(char *name, char data[TABLE_TABLE_DATA_SIZE]);
typedef struct dbtable_table_oper_s
{
    struct list_head list;
    char table_type;
    create_type_dbtable_t create_func;
    destroy_type_dbtable_t delete_func;
}dbtable_table_oper_t;


int register_table_type(
     int type,
     create_type_dbtable_t create,
     destroy_type_dbtable_t destroy
     );

#define SYSTEM_CFG_TABLE_NAME    "system_cfg"
#define SYSTEM_CFG_TABLE_INDEX   0

int app_db_init(
    int (*db_sync)(char *buffer, int len, unsigned int sync_flag, int slot_index),
    int (*db_sync_complete_handler)(),
    int db_relay_flag
    );

int db_table_init();

int create_dbtable(
    char name[MAX_TABLE_NAME_LEN],
    unsigned int entry_num,
    unsigned int entry_size,
    long (*handle_update)(void *, void*),
    long (*app_handle_update)(void*, void*),    
    long (*handle_insert)(void *),
    long (*handle_delete)(void *),
    long (*app_handle_delete)(void*),
    long (*alloc_entry)(struct db_table_s *, char **, int *),
    long (*free_entry)(struct db_table_s *, char *entry),
    int (*entry_ntoh)(void*),
    int (*entry_hton)(void*),
    int sync_flag,
    db_table_t **dbtable
    );

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
);


int destroy_dbtable(
	db_table_t *table
	);

int db_table_entry(
    db_table_t *db,
    unsigned int entry_id,
    void *data);

int db_table_lock_all(void);

int db_table_read_lock_all(void);

int db_table_unlock_all(void);

int db_table_lock(
    db_table_t *db
);

int db_table_unlock(
    db_table_t *db
);

int db_table_read_lock(
    db_table_t *db
);

int db_table_read_unlock(
	db_table_t *db
);

int dbtable_hash_lock(
    hash_table_index_t *hash
    );

int dbtable_hash_unlock(
    hash_table_index_t *hash
    );

int dbtable_sequence_lock(
    sequence_table_index_t *sequence
    );

int dbtable_sequence_unlock(
    sequence_table_index_t *sequence
    );

int dbtable_array_lock(
    array_table_index_t *array
    );

int dbtable_array_unlock(
    array_table_index_t *array
    );

int dbtable_slot_online_insert(
	int slot_index
);

int db_table_insert_entry(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
);

int db_table_free_entry_index(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
);

int db_table_delete_entry(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
);

int db_table_update_entry(
    db_table_t *db,
    unsigned int entry_id,
    void * data,
    db_index_common_t *orig_index,
    int local_flag
);

int db_table_synced_entry(
    db_table_t *db,
    unsigned int entry_id,
    unsigned int flag
);


int dbtable_create_hash_index(
	char *index_name,
	db_table_t *db,
	unsigned int hash_bucket_size,
	unsigned int  (*hash_key)(void *),
	unsigned int (*hash_cmp)(void *, void *),
	hash_table_index_t **hash
);

int dbtable_hash_insert(
    hash_table_index_t *hash,
    void *data
);

int dbtable_hash_insert_seq(
    hash_table_index_t *hash,
    void *data,
    int (*list_prev)(void*, void*)
);

int dbtable_hash_delete(
    hash_table_index_t *hash,
    void *data,
    void *ret_data
);

int dbtable_hash_update(
    hash_table_index_t *hash,
    void *predata,
    void *data
);

int dbtable_hash_count(
    hash_table_index_t *hash
);

int dbtable_hash_search 
( 
	hash_table_index_t *hash, 
	void *data,
	unsigned int (*compFunc)(void*,void*),
	void *retData
);


int dbtable_hash_head
(
    hash_table_index_t *hash,
    void * indata,
    void * outdata,
    unsigned int (*filter)(void *,void *)
);


int dbtable_hash_next
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
);

int dbtable_hash_head_key
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
);

int dbtable_hash_next_key
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
);


int dbtable_hash_traversal(
	hash_table_index_t *hash,
	unsigned int flag,
	void * data,
	unsigned int (*filter)(void *,void *),
    int (*processor)(hash_table_index_t*, void *,unsigned int)
);

int dbtable_hash_traversal_key(
    hash_table_index_t *hash,
    unsigned int flag,
    void * data,
    unsigned int (*filter)(void *,void *),
    int (*processor)(hash_table_index_t *, void *, unsigned int)
);

void dbtable_hash_show
(
	hash_table_index_t *hash,
	char* string,
	void (*showFunc)(void *, char*)
);

int dbtable_hash_return
(
    hash_table_index_t *hash,
    void * indata,
    void * outdata,
    unsigned int len,
    unsigned int (*filter)(void *,void *)
);

int npd_dbtable_header_hton(npd_sync_msg_header_t *header);


int dbtable_create_sequence_index(
	char *index_name,
    db_table_t *db,
    unsigned int sequence_size,
    unsigned int (*sequence_index)(unsigned int),
    unsigned int (*sequence_key)(void*),
    int (*sequence_cmp)(void *, void*),
    sequence_table_index_t **sequence
);
int dbtable_sequence_insert(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data
);
int dbtable_sequence_search(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data
);
int dbtable_sequence_head(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data
);
int dbtable_sequence_next(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data,
    void *ret_data,
    unsigned int (*filter)(void *,void *)
    
);
int dbtable_sequence_traverse_next(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *ret_data
);

int dbtable_sequence_delete(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *data,
    void *ret_data
);

int dbtable_sequence_traversal(
	sequence_table_index_t *sequence,
	unsigned int flag,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*processor)(sequence_table_index_t*, void *,unsigned int)
);

int dbtable_sequence_show(
	sequence_table_index_t *sequence,
	char * string,
	int *string_size,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*show)(void *, char*, int *)
);

int dbtable_sequence_update(
    sequence_table_index_t *sequence,
    unsigned int sequence_index,
    void *predata,
    void *data
);

int dbtable_create_array_index(
	char *index_name,
    db_table_t *db,
    array_table_index_t **array
);

int dbtable_array_insert(
    array_table_index_t *array,
    unsigned int *array_index,
    void *data
);

int dbtable_array_insert_byid(
    array_table_index_t *array,
    unsigned int array_index,
    void *data
);
int dbtable_array_insert_after(
    array_table_index_t *array,
    unsigned int *array_index,
    void *data,
    unsigned int start_index
);

int dbtable_array_get(
    array_table_index_t *array,
    unsigned int array_index,
    void *data
);

int dbtable_array_totalcount(
    array_table_index_t *array
    );

int dbtable_array_delete(
    array_table_index_t *array,
    unsigned int array_index,
    void *ret_data
);


int dbtable_array_update(
    array_table_index_t *array,
    unsigned int array_index,
    void *predata,
    void *data
);

int dbtable_array_show(
	array_table_index_t *array,
	char * string,
	int* string_size,
	void * data,
	unsigned int (*filter)(void *,void *),
	int (*show)(void *, char*, int*)
);

int dbtable_btree_insert(
    btree_table_index_t *btree,
    void *data
);

int dbtable_btree_update(
    btree_table_index_t *btree,
    void *predata,
    void *data
);

int dbtable_btree_search(
    btree_table_index_t *btree,
    void *data,
    void *ret_data
    );

int dbtable_btree_delete(
    btree_table_index_t *btree,
    void *data
);

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
);

int dbtable_recv(
    int fd, 
    char* buf, 
    int len, 
    void *private_data
);

int dbtable_sync_hwsw_timer(
    void *data
);

long db_get(char *name, db_table_t **db);

int dbtable_index_get_by_name(char *db_name, char *table_name, char *index_name, hash_table_index_t **hash);

int dbtable_table_show_func_install(db_table_t *db, dbtable_show_func func);

int dbtable_table_show(char *name, char **show_buf, int print_header);

typedef enum _DB_TABLE_RETURN_CODE_
{
	DB_TABLE_RETURN_CODE_OK,
	DB_TABLE_RETURN_CODE_DB_NOT_EXIST,
	DB_TABLE_RETURN_CODE_TABLE_NOT_EXIST,
	DB_TABLE_RETURN_CODE_TABLE_NAME_LEN_ERR,
	DB_TABLE_RETURN_CODE_BAD_INDEX,
	DB_TABLE_RETURN_CODE_INDEX_NAME_LEN_ERR,
	DB_TABLE_RETURN_CODE_KEY_OVERLAP,
	DB_TABLE_RETURN_CODE_BAD_REQUEST_TYPE,
	DB_TABLE_RETURN_CODE_INDEX_NOT_EXIST,
	DB_TABLE_RETURN_CODE_BAD_INPUT,
	DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR,
	DB_TABLE_RETURN_CODE_TABLE_FULL,
	DB_TABLE_RETURN_CODE_TABLE_EMPTY,
	DB_TABLE_RETURN_CODE_ENTRY_NOT_EXIST,
	DB_TABLE_RETURN_CODE_ELEMENT_NOT_EXIST,
	DB_TABLE_RETURN_CODE_CALLBACK_ERR,
	DB_TABLE_RETURN_CODE_SYNC_FAILED
}DB_TABLE_RETURN_CODE;

#endif


