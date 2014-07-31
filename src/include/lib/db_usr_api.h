#ifndef DB_USR_API_H
#define DB_USR_API_H

#include <unistd.h>
#include "npd_database.h"
#define MAX_DB_PACKET_LEN 20000
#define DB_LOCAL_SERVICE_PATH_PREFIX "/tmp/db."
#define DB_LOCAL_SERVICE     0x1
#define DB_TIPC_SERVICE      0x2
typedef struct db_cmp_index_s
{
    db_index_common_t common;
    void *data;
  	int  (*cmp_cmp)(struct db_cmp_index_s *, void *, void *);/* Data compare function*/
}db_cmp_index_t;

/*这个主要用于dump出所有的连接情况*/
typedef struct _DB_CONNECT_SESSION_
{
    struct list_head list;
	int sock;
	char *name;
	void *data;
}DB_CONNECT_SESSION;

typedef struct _DB_LISTEN_SESSION_
{
	int sock;
    struct list_head connect_list;
}DB_LISTEN_SESSION;

typedef struct _OSAL_DB_CTRL_
{
    struct list_head list;
    char name[MAX_DB_NAME_LEN];
	int service_type;
    struct list_head table_list;
	DB_LISTEN_SESSION *local_listen;/*local 和TIPC可能可以统一到一个上*/
	DB_LISTEN_SESSION *tipc_listen;
}OSAL_DB_CTRL;

OSAL_DB_CTRL *create_db(char *name, int service_type);

typedef enum _REQUEST_TYPE_
{
	DB_TABLE_ENTRY_INSERT,
	DB_TABLE_ENTRY_DELETE,
	DB_TABLE_ENTRY_SEARCH,
	DB_TABLE_ENTRY_UPDATE,
	DB_TABLE_ENTRY_TRAVER,
	DB_TABLE_ENTRY_COUNT,
	DB_TABLE_ENTRY_HEAD,
	DB_TABLE_ENTRY_NEXT,
	DB_TABLE_ENTRY_SHOW,
	DB_TABLE_REQUEST_MAX,
	DB_TABLE_INDEX_CREATE
}REQUEST_TYPE;

typedef struct _DBTABLE_REQUEST_ENTRY_
{
	int size;
	void *request_entry_data;
}DBTABLE_REQUEST_ENTRY;

typedef struct _DBTABLE_REQUEST_INDEX_
{
	int size;
	char *index_name;
}DBTABLE_REQUEST_INDEX;

typedef struct _DBTABLE_REQUEST_PACKET_
{
	char db_name[MAX_DB_NAME_LEN];
	char table_name[MAX_TABLE_NAME_LEN];
	int flag;
	int request_id;
	int request_type;
	char index_name[MAX_INDEX_NAME_LEN];
	int entry_size;
}DBTABLE_REQUEST_PACKET;


typedef struct _DBTABLE_REQUEST_
{
	char *db_name;
	char *table_name;
	int flag;
	int request_id;
	int request_type;
	char *index_name;
	DBTABLE_REQUEST_ENTRY request_entry;
}DBTABLE_REQUEST;

typedef struct _DBTABLE_RESPONSE_ENTRY_
{
	void *response_entry_data;
}DBTABLE_RESPONSE_ENTRY;

typedef struct _DBTABLE_RESPONSE_PACKET_
{
	char db_name[MAX_DB_NAME_LEN];
	char table_name[MAX_TABLE_NAME_LEN];
	int flag;
	int response_id;
	int response_type;
	int return_code;
	int entry_count;
	int entry_size;
}DBTABLE_RESPONSE_PACKET;

typedef struct _DBTABLE_RESPONSE_
{
	char *db_name;
	char *table_name;
	int flag;
	int response_id;
	int response_type;
	int return_code;
	int entry_count;
	int entry_size;
	DBTABLE_RESPONSE_ENTRY *response_entry;
}DBTABLE_RESPONSE;

int osal_db_table_request_generate_packet(DBTABLE_REQUEST *request, char *packet_buff);
int osal_db_table_request_parse_packet(char *packet_buff, DBTABLE_REQUEST *request);
int osal_db_table_response_generate_packet(DBTABLE_RESPONSE *response, char *packet_buff);
int osal_db_table_response_parse_packet(char *packet_buff, DBTABLE_RESPONSE *response);
void dbtable_response_free(DBTABLE_RESPONSE *response);

typedef int (*dbtable_request_func_t)( hash_table_index_t *index, DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response);
typedef dbtable_request_func_t dbtable_request_event_func_t;

DB_CONNECT_SESSION *app_db_local_open(char *name);
void app_db_conn_close(DB_CONNECT_SESSION *db_conn);
DB_CONNECT_SESSION *app_db_tipc_open(char *db_name, int service_type, int chassis, int dest_slot);
int app_db_send_request(DB_CONNECT_SESSION *db_conn, DBTABLE_REQUEST *request);
DBTABLE_RESPONSE *app_db_get_response(DB_CONNECT_SESSION *db_conn);
DBTABLE_RESPONSE *app_db_table_request(DB_CONNECT_SESSION *db_conn, char *table_name, char *index_name, int event, void *data, int size);

#endif
