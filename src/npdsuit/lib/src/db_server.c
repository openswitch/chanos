
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* db_server.c
*
*
* CREATOR:
*		zhanwei@autelan.com
*
* DESCRIPTION:
*		DB server api. Using this api to create a db server.
*
* DATE:
*		08/05/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $
*******************************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"

#include "sysdef/npd_sysdef.h"
#include "tipc_api/tipc_api.h"
#include "npd_list.h"
#include "npd_database.h"
#include "db_usr_api.h"

void dump_request_struct(DBTABLE_REQUEST *request);
static void dump_request_packet(char *packet_buff);
static void dump_response_struct(DBTABLE_RESPONSE *response);
static void dump_response_packet(char *packet_buff);
DBTABLE_RESPONSE *dbtable_request(DBTABLE_REQUEST *request);

int db_server_printf_enable = 0;
#define db_server_printf if(db_server_printf_enable) printf

void osal_db_debug_enable()
{
	db_server_printf_enable = 1;
}

void osal_db_debug_disable()
{
	db_server_printf_enable = 0;
}

int osal_db_local_service_ctrl_handler(int sock, void **data)
{
	struct sockaddr_un client_addr = {0}; 
	DB_CONNECT_SESSION *connect_session = NULL;
	OSAL_DB_CTRL *db_copy = NULL;/*保存DB的一个副本*/
	int new_socket = 0, name_len = 0;
	unsigned len = 0;
	memset(&client_addr,0,sizeof(struct sockaddr_un));
	len = sizeof(struct sockaddr_un);

	new_socket = accept(sock, (struct sockaddr *)&client_addr, &len);
	if (new_socket < 0 )
	{
		return -1;
	}
	db_server_printf("A connection, socket %d from %s\r\n", new_socket, client_addr.sun_path);
    db_copy = malloc(sizeof(OSAL_DB_CTRL));
    if(db_copy == NULL)
    {
    	close(new_socket);
    	return -1;
    }
	connect_session = malloc(sizeof(DB_CONNECT_SESSION));
    if(connect_session == NULL)
    {
		free(db_copy);
    	close(new_socket);
    	return -1;
    }
	connect_session->sock = new_socket;
	if((name_len = strlen(client_addr.sun_path)) > 0)
	{
	    connect_session->name = malloc(name_len + 1);
		memset(connect_session->name, 0, name_len + 1);
		strncpy(connect_session->name, client_addr.sun_path, name_len);
		db_server_printf("Connected from %s\r\n", connect_session->name);
	}
	else
	{
		connect_session->name = NULL;
	}
	connect_session->data = NULL;
	if(*data)
	{
		memcpy(db_copy, *data, sizeof(OSAL_DB_CTRL));
	}
	*data = db_copy;
	list_add(&connect_session->list, &(db_copy->local_listen->connect_list));

	return new_socket;
}

static DB_CONNECT_SESSION *osal_db_local_service_get_session(OSAL_DB_CTRL *db, int sock)
{
    struct list_head *pos;
	DB_CONNECT_SESSION *con_sess = NULL;
	if(db == NULL)
	{
		return NULL;
	}
	if(db->local_listen == NULL)
	{
		return NULL;
	}
	
    for (pos = db->local_listen->connect_list.next; pos != &db->local_listen->connect_list; \
            pos = pos->next)
        /*
            list_for_each(pos, &db->local_listen->connect_list)
        */
    {
        con_sess = list_entry(pos, DB_CONNECT_SESSION, list);

        if (con_sess->sock == sock)
        {
            return con_sess;
        }
    }
	return NULL;
}

int osal_db_local_service_read_handler(int sock, char *buff, int len, void *data)
{
	int ret = 0;
	DBTABLE_RESPONSE *response = NULL;
	DBTABLE_REQUEST *request = NULL;
	DB_CONNECT_SESSION *connect_session = NULL;
	OSAL_DB_CTRL *db_copy = (OSAL_DB_CTRL *)data;/*保存DB的一个副本*/
	char *send_buff = NULL;
	if(len <= 0)
	{
		connect_session = osal_db_local_service_get_session(db_copy, sock);
		if(connect_session)
		{
			list_del(&connect_session->list);
			if(connect_session->name)
			{
				free(connect_session->name);
			}
			free(connect_session);
		}
		return -1;
	}
	dump_request_packet(buff);
	request = malloc(sizeof(DBTABLE_REQUEST));
	if(request == NULL)
	{
		db_server_printf("(%s:%d) Alloc memory for DBTABLE_REQUEST failed.\r\n", __func__, __LINE__);
		/*连内存申请不到了，就没法给上层回应了*/
		return 0;/*注意:返回值不能为-1,否则讲关闭socket*/
	}
	memset(request, 0, sizeof(DBTABLE_REQUEST));
	send_buff = malloc(MAX_DB_PACKET_LEN);
	if(send_buff == NULL)
	{
		free(request);
		db_server_printf("(%s:%d) Alloc memory for send buffer failed.\r\n", __func__, __LINE__);
		return 0;
	}
	ret = osal_db_table_request_parse_packet(buff, request);
	if(ret != 0)
	{
		db_server_printf("(%s:%d) Parse request packet failed.\r\n", __func__, __LINE__);
		/*应该给上面一个回应*/
		response = malloc(sizeof(DBTABLE_RESPONSE));
		if(response)
		{
			memset(response, 0, sizeof(DBTABLE_RESPONSE));
			response->return_code = DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
			response->response_id = 0xFFFFFFFF;
			ret = osal_db_table_response_generate_packet(response, send_buff);
			if(ret > 0)
			{
				write(sock, send_buff, ret);
			}
			dbtable_response_free(response);
		}
		free(send_buff);
	    free(request);
		return 0;/*注意:返回值不能为-1,否则讲关闭socket*/
	}
	response = dbtable_request(request);
	if(response == NULL)
	{
	    free(request);
	    free(send_buff);
		return 0;
	}
	dump_response_struct(response);
	ret = osal_db_table_response_generate_packet(response, send_buff);
	if(ret > 0)
	{
		dump_response_packet(send_buff);
		write(sock, send_buff, ret);
	}
	dbtable_response_free(response);
	free(request);
	free(send_buff);
	return 0;
}

static int osal_db_create_service_local(OSAL_DB_CTRL *db)
{
	int ret = -1;
	int sock = 0;
	char path_name[32];
	struct sockaddr_un servaddr = {0};
	db->local_listen = NULL;
	sock = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(sock <= 0)
	{
		db_server_printf("(%s:%d) Create AF_LOCAL SOCK_STREAM socket failed.\r\n", __func__, __LINE__);
		return -1;
	}
	memset(path_name, 0, 32);
	sprintf(path_name, "%s%s", DB_LOCAL_SERVICE_PATH_PREFIX, db->name);
    unlink(path_name);
    servaddr.sun_family   =   AF_LOCAL;
    strcpy(servaddr.sun_path, path_name);
    if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		db_server_printf("(%s:%d) Bind %s to AF_LOCAL SOCK_STREAM socket %d failed.\r\n", __func__, __LINE__, path_name, sock);
		return -1;
    }
	chmod(servaddr.sun_path, 0777);
    if(listen(sock, 15) < 0)
    {
		db_server_printf("(%s:%d) Listen on AF_LOCAL SOCK_STREAM socket %d failed.\r\n", __func__, __LINE__, sock);
		return -1;
    }
	db->local_listen = malloc(sizeof(DB_LISTEN_SESSION));
	if(db->local_listen == NULL)
	{
		close(sock);
		return -1;
	}
	db->local_listen->connect_list.next = &db->local_listen->connect_list;
	db->local_listen->connect_list.prev = &db->local_listen->connect_list;
	db->local_listen->sock = sock;
	
	ret = osal_register_server_ctrl_fd(sock, db->service_type, osal_db_local_service_ctrl_handler, osal_db_local_service_read_handler, db);
	if(ret != 0)
	{
		db_server_printf("(%s:%d) Register sock %d to thread master failed.\r\n", __func__, __LINE__, sock);
		free(db->local_listen);
		db->local_listen = NULL;
		close(sock);
		return -1;
	}
	return 0;
}

static DB_CONNECT_SESSION *osal_db_tipc_service_get_session(OSAL_DB_CTRL *db, int sock)
{
    struct list_head *pos;
	DB_CONNECT_SESSION *con_sess = NULL;
	if(db == NULL)
	{
		return NULL;
	}
	if(db->tipc_listen == NULL)
	{
		return NULL;
	}
	
    for (pos = db->tipc_listen->connect_list.next; pos != &db->tipc_listen->connect_list; \
            pos = pos->next)
        /*
            list_for_each(pos, &db->local_listen->connect_list)
        */
    {
        con_sess = list_entry(pos, DB_CONNECT_SESSION, list);

        if (con_sess->sock == sock)
        {
            return con_sess;
        }
    }
	return NULL;
}

int osal_db_tipc_server_handler(int sock, char *buff, int len, void *data)
{
	int ret = 0;
	DBTABLE_RESPONSE *response = NULL;
	DBTABLE_REQUEST *request = NULL;
	DB_CONNECT_SESSION *connect_session = NULL;
	OSAL_DB_CTRL *db_copy = (OSAL_DB_CTRL *)data;/*保存DB的一个副本*/
	char *send_buff = NULL;
	if(len <= 0)
	{
		connect_session = osal_db_tipc_service_get_session(db_copy, sock);
		if(connect_session)
		{
			list_del(&connect_session->list);
			if(connect_session->name)
			{
				free(connect_session->name);
			}
			free(connect_session);
		}
		return -1;
	}
	dump_request_packet(buff);
	request = malloc(sizeof(DBTABLE_REQUEST));
	if(request == NULL)
	{
		db_server_printf("(%s:%d) Alloc memory for DBTABLE_REQUEST failed.\r\n", __func__, __LINE__);
		/*连内存申请不到了，就没法给上层回应了*/
		return 0;
	}
	memset(request, 0, sizeof(DBTABLE_REQUEST));
	send_buff = malloc(MAX_DB_PACKET_LEN);
	if(send_buff == NULL)
	{
		free(request);
		db_server_printf("(%s:%d) Alloc memory for send buffer failed.\r\n", __func__, __LINE__);
		return 0;
	}
	ret = osal_db_table_request_parse_packet(buff, request);
	if(ret != 0)
	{
		db_server_printf("(%s:%d) Parse request packet failed.\r\n", __func__, __LINE__);
		/*应该给上面一个回应*/
		response = malloc(sizeof(DBTABLE_RESPONSE));
		if(response)
		{
			memset(response, 0, sizeof(DBTABLE_RESPONSE));
			response->return_code = DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
			response->response_id = 0xFFFFFFFF;
			ret = osal_db_table_response_generate_packet(response, send_buff);
			if(ret > 0)
			{
				write(sock, send_buff, ret);
			}
			dbtable_response_free(response);
		}
		free(send_buff);
	    free(request);
		return 0;
	}
	response = dbtable_request(request);
	if(response == NULL)
	{
	    free(request);
	    free(send_buff);
		return 0;
	}
	dump_response_struct(response);
	ret = osal_db_table_response_generate_packet(response, send_buff);
	if(ret > 0)
	{
		dump_response_packet(send_buff);
		write(sock, send_buff, ret);
	}
	dbtable_response_free(response);
	free(request);
	free(send_buff);
	return 0;
}

int osal_db_tipc_accept_callback(int ctrl_sock, int new_socket, void **data)
{
	DB_CONNECT_SESSION *connect_session = NULL;
	OSAL_DB_CTRL *db_copy = NULL;/*保存DB的一个副本*/
    OSAL_DB_CTRL *db_old = (OSAL_DB_CTRL *)(*data);
	int tipc_addr = 0;
	tipc_addr = tipc_get_peer_node(new_socket);
	if(tipc_addr == 0)
	{
		return -1;
	}
    db_copy = malloc(sizeof(OSAL_DB_CTRL));
    if(db_copy == NULL)
    {
    	return -1;
    }
	connect_session = malloc(sizeof(DB_CONNECT_SESSION));
    if(connect_session == NULL)
    {
		free(db_copy);
    	return -1;
    }
	connect_session->sock = new_socket;
	connect_session->name = malloc(32);
	if(connect_session->name == NULL)
	{
		free(db_copy);
		free(connect_session);
    	return -1;
	}
	memset(connect_session->name, 0, 32);
	sprintf(connect_session->name, "<%d.%d.%d>", tipc_zone(tipc_addr), tipc_cluster(tipc_addr), tipc_node(tipc_addr));
	connect_session->data = NULL;
	if(db_old)
	{
		db_old->tipc_listen->sock = ctrl_sock;
		memcpy(db_copy, *data, sizeof(OSAL_DB_CTRL));
	}
	*data = db_copy;
	list_add(&connect_session->list, &(db_copy->tipc_listen->connect_list));

	return 0;
}

/*注意:tipc_server_init函数最好能把扩展参数带进去*/
static int osal_db_create_service_tipc(OSAL_DB_CTRL *db)
{
	int ret = -1;
	unsigned int local_slot = 0;
	db->tipc_listen = NULL;
	db->tipc_listen = malloc(sizeof(DB_LISTEN_SESSION));
	if(db->tipc_listen == NULL)
	{
		return -1;
	}
	local_slot = tipc_get_own_node();
	local_slot = local_slot&0xF;
	db->tipc_listen->connect_list.next = &db->tipc_listen->connect_list;
	db->tipc_listen->connect_list.prev = &db->tipc_listen->connect_list;
	db->tipc_listen->sock = 0;
	
	/*FIXME...slot no should be the real local slot no combined with chassis no*/
	ret = tipc_server_init(db->service_type,local_slot, osal_db_tipc_server_handler, osal_db_tipc_accept_callback, db);
	if(ret != 0)
	{
		db_server_printf("(%s:%d) Create TIPC server for db (%s:%d) failed.\r\n", __func__, __LINE__, db->name, db->service_type);
		free(db->tipc_listen);
		db->tipc_listen = NULL;
		return -1;
	}
	return 0;
}

static int osal_db_create_service(OSAL_DB_CTRL *db, int flag)
{
	int ret = -1;
	if(flag&DB_LOCAL_SERVICE)
	{
    	ret = osal_db_create_service_local(db);
    	if(ret!= 0)
    	{
    		db_server_printf("Create db(%s:%d) local servie failed.\r\n", db->name, db->service_type);
    		return -1;
    	}
	}
	if(flag&DB_TIPC_SERVICE)
	{
    	ret = osal_db_create_service_tipc(db);
    	if(ret != 0)
    	{
    		db_server_printf("Create db(%s:%d) tipc servie failed.\r\n", db->name, db->service_type);
    		return -1;
    	}
	}
	return ret;
}
/*暂时只启动local service*/
OSAL_DB_CTRL *create_db(char *name, int service_type)
{
	int ret = -1;
	OSAL_DB_CTRL *osal_db = NULL;
	if(name == NULL)
	{
		return NULL;
	}
    if(strlen(name) >= MAX_DB_NAME_LEN)
    {
		return NULL;
    }
	osal_db = malloc(sizeof(OSAL_DB_CTRL));
	if(osal_db == NULL)
	{
		return NULL;
	}
	memset(osal_db, 0, sizeof(OSAL_DB_CTRL));
	strncpy(osal_db->name, name, MAX_DB_NAME_LEN);
	osal_db->service_type = service_type;
	osal_db->table_list.next = &osal_db->table_list;
	osal_db->table_list.prev = &osal_db->table_list;
	ret = osal_db_create_service(osal_db, DB_LOCAL_SERVICE);
	if(ret != 0)
	{
		db_server_printf("Create db service for %s failed.\r\n", name);
	}
	osal_thread_create(NULL, (void (*)( void * ))osal_thread_master_run, 80, 0x80000, (void *)service_type);
	return osal_db;
}

int dbtable_cmp_cmp(db_cmp_index_t *cmp_index, void *data, void *data_input)
{
	char *saved_index_data = (char *)cmp_index->data;
	char *saved_data = (char *)data;
	char *input_data = (char *)data_input;
	int len = 0;
	int i = 0;
	len = cmp_index->common.db->entry_size;
	for(i = 0; i < len; i++)
	{
		if((unsigned char)saved_index_data[i] == 0xFF)
		{
			if(saved_data[i] != input_data[i])
			{
				return NPD_FALSE;
			}
		}
	}
	return NPD_TRUE;
}

int dbtable_create_cmp_index(char *index_name, db_table_t *db, db_cmp_index_t **cmp_index, void *data)
{
	int len = 0;
	if(index_name == NULL)
	{
        *cmp_index = NULL;
        return -1;
    }
	len = strlen(index_name);
	if(len <= 0 || len >= MAX_INDEX_NAME_LEN)
	{
        *cmp_index = NULL;
        return -1;
    }
    *cmp_index = malloc(sizeof(db_cmp_index_t));

    if (NULL == *cmp_index)
    {
        *cmp_index = NULL;
        return -1;
    }

    (*cmp_index)->common.db = db;
	strncpy((*cmp_index)->common.index_name, index_name, MAX_INDEX_NAME_LEN);
	(*cmp_index)->common.index_type = DB_TABLE_INDEX_HASH;/*not used*/
    (*cmp_index)->common.index_insert = NULL;
    (*cmp_index)->common.index_delete = NULL;
	(*cmp_index)->cmp_cmp = dbtable_cmp_cmp;
    list_add(&((*cmp_index)->common.list), &db->index_list);
    return 0;
}

int dbtable_commom_traversal(db_cmp_index_t *cmp_index, void *index_data, DBTABLE_RESPONSE_ENTRY **return_data)
{
	DBTABLE_RESPONSE_ENTRY *response_entry = NULL;
    dbentry_common_t *entry = NULL;
	int count = 0;
	int i = 0;
	void **temp_ptr_matched = NULL;
	int entry_size = cmp_index->common.db->entry_size;
	int entry_count = cmp_index->common.db->entry_num;
	temp_ptr_matched = malloc(entry_count * sizeof(void *));
	if(temp_ptr_matched == NULL)
	{
		return -1;
	}
	for(i = 0; i < entry_count; i++)
	{
		temp_ptr_matched[i] = NULL;
		entry = cmp_index->common.db->entries+i;
		if(entry->real_data && entry->flags&DB_ENTRY_EXIST)
		{
		    if(1 /*dbtable_cmp_cmp(cmp_index, entry->real_data, index_data)*/)/*need more checking*/
		    {
				temp_ptr_matched[count] = entry->real_data;
				count++;
		    }
		}
	}
	*return_data = malloc(count*sizeof(DBTABLE_RESPONSE_ENTRY));
	if(*return_data == NULL)
	{
		free(temp_ptr_matched);
		return -1;
	}
	for(i = 0; i < count; i++)
	{
		response_entry = *return_data + i;
		response_entry->response_entry_data = malloc(entry_size);
		memcpy(response_entry->response_entry_data, temp_ptr_matched[i], entry_size);
	}
	free(temp_ptr_matched);
	return count;
}

/*注意:request->request_entry.request_entry_data由调用者处理*/
void dbtable_request_free(DBTABLE_REQUEST *request)
{
	if(request)
	{
		free(request);
	}
}

void dbtable_response_free(DBTABLE_RESPONSE *response)
{
	int i = 0;
	if(response)
	{
		if(response->response_entry)
		{
		    for(i = 0; i < response->entry_count; i++)
		    {
    			if(response->response_entry[i].response_entry_data)
    			{
    				free(response->response_entry[i].response_entry_data);
    				response->response_entry[i].response_entry_data = NULL;
    			}
		    }
			free(response->response_entry);
			response->response_entry = NULL;
		}
		free(response);
	}
}

int dbtable_request_hash_search(hash_table_index_t *hash, DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response)
{
	response->response_entry = malloc(sizeof(DBTABLE_RESPONSE_ENTRY));
	if(response->response_entry == NULL)
	{
		return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
	response->entry_size = hash->common.db->entry_size;
	response->response_entry->response_entry_data = malloc(response->entry_size);
	if(response->response_entry->response_entry_data == NULL)
	{
		free(response->response_entry);
		response->response_entry = NULL;
		return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
	response->entry_count = 1;
	return dbtable_hash_search(hash, request->request_entry.request_entry_data, NULL, response->response_entry->response_entry_data);
}

int dbtable_request_hash_insert(hash_table_index_t *hash, DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response)
{
	return dbtable_hash_insert(hash, request->request_entry.request_entry_data);
}

int dbtable_request_hash_delete(hash_table_index_t *hash, DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response)
{
	response->response_entry = malloc(sizeof(DBTABLE_RESPONSE_ENTRY));
	if(response->response_entry == NULL)
	{
		return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
	response->entry_size = hash->common.db->entry_size;
	response->response_entry->response_entry_data = malloc(response->entry_size);
	if(response->response_entry->response_entry_data == NULL)
	{
		free(response->response_entry);
		response->response_entry = NULL;
		return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
	response->entry_count = 1;
	return dbtable_hash_delete(hash, request->request_entry.request_entry_data, response->response_entry->response_entry_data);
}

int dbtable_request_hash_update(hash_table_index_t *hash, DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response)
{
    return dbtable_hash_insert(hash, request->request_entry.request_entry_data);
}

int dbtable_request_hash_traversal(hash_table_index_t *hash, DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response)
{
	int count = 0;
	count = dbtable_commom_traversal((db_cmp_index_t *)hash, request->request_entry.request_entry_data, &response->response_entry);
	response->entry_count = count;
	response->entry_size = hash->common.db->entry_size;
	return 0;
}

int dbtable_request_hash_count(hash_table_index_t *hash, DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response)
{
	response->response_entry = malloc(sizeof(DBTABLE_RESPONSE_ENTRY));
	if(response->response_entry == NULL)
	{
		return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
	response->entry_size = sizeof(int);
	response->response_entry->response_entry_data = malloc(response->entry_size);
	if(response->response_entry->response_entry_data == NULL)
	{
		free(response->response_entry);
		response->response_entry = NULL;
		return DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
	}
	response->entry_count = 1;
	*(int *)(response->response_entry->response_entry_data) = dbtable_hash_count(hash);
	return DB_TABLE_RETURN_CODE_OK;
}
int dbtable_request_common_index_create(DBTABLE_REQUEST *request, DBTABLE_RESPONSE *response)
{
	db_cmp_index_t *cmp_index = NULL;
	int ret = -1;
	db_table_t *db_table = NULL;
	ret = db_get(request->table_name, &db_table);
	if(ret != DB_TABLE_RETURN_CODE_OK || db_table == NULL)
	{
		response->return_code = ret;
		return ret;
	}
	ret = dbtable_create_cmp_index(request->index_name, db_table, &cmp_index, request->request_entry.request_entry_data);
	if(ret != 0)
	{
		response->return_code = DB_TABLE_RETURN_CODE_BAD_INDEX;
	}
	else
	{
		response->return_code = 0;
	}
	return response->return_code;
}

dbtable_request_func_t hash_dbtable_request_func_t[DB_TABLE_REQUEST_MAX+1] = 
{
	dbtable_request_hash_insert,
	dbtable_request_hash_delete,
	dbtable_request_hash_search,
	dbtable_request_hash_update,
	dbtable_request_hash_traversal,
	dbtable_request_hash_count,
	NULL/*dbtable_request_hash_head*/,
	NULL/*dbtable_request_hash_next*/,
	NULL/*dbtable_request_hash_show*/,
	NULL
};

dbtable_request_event_func_t *dbtable_request_func_matrix[DB_TABLE_INDEX_MAX] = 
{
	hash_dbtable_request_func_t,
	NULL,
	NULL,
	NULL
};

void dump_request_struct(DBTABLE_REQUEST *request)
{
	int ii = 0;
	char *entry_data = NULL;
	if(request->db_name)
	{
		db_server_printf("db name:          %s\r\n", request->db_name);
	}
	if(request->table_name)
	{
		db_server_printf("table name:       %s\r\n", request->table_name);
	}
	if(request->index_name)
	{
		db_server_printf("index name:       %s\r\n", request->index_name);
	}
	db_server_printf("flag:             %x\r\n", request->flag);
	db_server_printf("request_id:       %d\r\n", request->request_id);
	db_server_printf("request_type:     %d\r\n", request->request_type);
	db_server_printf("entry size:       %d\r\n", request->request_entry.size);
	db_server_printf("entry data:\r\n");
	entry_data = (char *)request->request_entry.request_entry_data;
	for(ii = 0; ii < request->request_entry.size; ii ++)
	{
		db_server_printf("%02x ", (unsigned char)(entry_data[ii]));
		if(ii%8 == 7)
		{
			db_server_printf("\r\n");
		}
	}
    db_server_printf("\r\n");
	
}

static void dump_request_packet(char *packet_buff)
{
	DBTABLE_REQUEST_PACKET *request_packet = (DBTABLE_REQUEST_PACKET *)packet_buff;
	int ii = 0;
	char *entry_data = NULL;
	db_server_printf("db name:          %s\r\n", request_packet->db_name);
	db_server_printf("table name:       %s\r\n", request_packet->table_name);
	db_server_printf("index name:       %s\r\n", request_packet->index_name);
	db_server_printf("flag:             %x\r\n", request_packet->flag);
	db_server_printf("request_id:       %d\r\n", request_packet->request_id);
	db_server_printf("request_type:     %d\r\n", request_packet->request_type);
	db_server_printf("entry size:       %d\r\n", request_packet->entry_size);
	db_server_printf("entry data:\r\n");
	entry_data = (char *)(request_packet + 1);
	for(ii = 0; ii < request_packet->entry_size; ii ++)
	{
		db_server_printf("%02x ", (unsigned char)(entry_data[ii]));
		if(ii%8 == 7)
		{
			db_server_printf("\r\n");
		}
	}
    db_server_printf("\r\n");
	
}

static void dump_response_struct(DBTABLE_RESPONSE *response)
{
	int i = 0;
	int ii = 0;
	char *entry_data = NULL;
	if(response->db_name)
	{
		db_server_printf("db name:          %s\r\n", response->db_name);
	}
	if(response->table_name)
	{
		db_server_printf("table name:       %s\r\n", response->table_name);
	}
	db_server_printf("flag:             %x\r\n", response->flag);
	db_server_printf("return_code:      %d\r\n", response->return_code);
	db_server_printf("response_id:      %d\r\n", response->response_id);
	db_server_printf("response_type:    %d\r\n", response->response_type);
	db_server_printf("entry size:       %d\r\n", response->entry_size);
	db_server_printf("entry count:      %d\r\n", response->entry_count);
	db_server_printf("entry data:\r\n");
	for(i = 0; i < response->entry_count; i++)
	{
		db_server_printf("(%d)\r\n", i);
    	entry_data = response->response_entry[i].response_entry_data;
    	for(ii = 0; ii < response->entry_size; ii ++)
    	{
		    db_server_printf("%02x ", (unsigned char)(entry_data[ii]));
    		if(ii%8 == 7)
    		{
    			db_server_printf("\r\n");
    		}
    	}
	}
    db_server_printf("\r\n");
	
}

static void dump_response_packet(char *packet_buff)
{
	DBTABLE_RESPONSE_PACKET *response_packet = (DBTABLE_RESPONSE_PACKET *)packet_buff;
	int i = 0, ii = 0;
	char *entry_data = NULL;
	db_server_printf("db name:          %s\r\n", response_packet->db_name);
	db_server_printf("table name:       %s\r\n", response_packet->table_name);
	db_server_printf("flag:             %x\r\n", response_packet->flag);
	db_server_printf("return_code:      %d\r\n", response_packet->return_code);
	db_server_printf("response_id:      %d\r\n", response_packet->response_id);
	db_server_printf("response_type:    %d\r\n", response_packet->response_type);
	db_server_printf("entry size:       %d\r\n", response_packet->entry_size);
	db_server_printf("entry count:      %d\r\n", response_packet->entry_count);
	db_server_printf("entry data:\r\n");
    entry_data = (char *)(response_packet + 1);
	for(i = 0; i < response_packet->entry_count; i++)
	{
		db_server_printf("(%d)\r\n", i);
		entry_data = entry_data + i*(response_packet->entry_size);
    	for(ii = 0; ii < response_packet->entry_size; ii ++)
    	{
		    db_server_printf("%02x ", (unsigned char)(entry_data[ii]));
    		if(ii%8 == 7)
    		{
    			db_server_printf("\r\n");
    		}
    	}
	}
    db_server_printf("\r\n");
	
}

DBTABLE_RESPONSE *dbtable_request(DBTABLE_REQUEST *request)
{
	int ret = -1;
	dbtable_request_func_t *request_func = NULL;
	hash_table_index_t *common_index = NULL;
	DBTABLE_RESPONSE *response = NULL;
	if(request == NULL)
	{
		return NULL;
	}
	if(request->db_name == NULL)
	{
		return NULL;
	}
	if(request->table_name == NULL)
	{
		return NULL;
	}
	if(request->index_name == NULL)
	{
		return NULL;
	}
	if(request->request_entry.request_entry_data == NULL)
	{
		return NULL;
	}
	response = malloc(sizeof(DBTABLE_RESPONSE));
	if(response == NULL)
	{
		return NULL;
	}
	memset(response, 0, sizeof(DBTABLE_RESPONSE));
	response->entry_count = 0;
	response->entry_size= 0;
	response->table_name = request->table_name;
	response->response_id = request->request_id;
	response->response_type = request->request_type;
	if(request->request_type == DB_TABLE_INDEX_CREATE)
	{
		dbtable_request_common_index_create(request, response);
		return response;
	}
	
	ret = dbtable_index_get_by_name(request->db_name, request->table_name, request->index_name, &common_index);
	if(ret != DB_TABLE_RETURN_CODE_OK)
	{
		response->return_code = ret;
		return response;
	}
	if(common_index->common.index_type < 0 || common_index->common.index_type > DB_TABLE_INDEX_MAX)
	{
		response->return_code = DB_TABLE_RETURN_CODE_BAD_INDEX;
		return response;
	}
	
	if(request->request_type < 0 || request->request_type > DB_TABLE_REQUEST_MAX)
	{
		response->return_code = DB_TABLE_RETURN_CODE_BAD_REQUEST_TYPE;
		return response;
	}
	if(dbtable_request_func_matrix[common_index->common.index_type] == NULL)
	{
		ret = DB_TABLE_RETURN_CODE_BAD_INDEX;
	}
	else
	{
		request_func = dbtable_request_func_matrix[common_index->common.index_type];
        if(request_func[request->request_type])
        {
    	    ret = (*request_func[request->request_type])(common_index, request, response);
        }
    	else
    	{
    		ret = DB_TABLE_RETURN_CODE_BAD_REQUEST_TYPE;
    	}
	}
	response->return_code = ret;
	return response;
}

int osal_db_table_request_generate_packet(DBTABLE_REQUEST *request, char *packet_buff)
{
	DBTABLE_REQUEST_PACKET *request_packet = NULL;

	if(request == NULL || packet_buff == NULL)
	{
		return -1;
	}
	if(request->table_name == NULL || request->index_name == NULL)
	{
		return -1;
	}
	request_packet = (DBTABLE_REQUEST_PACKET *)packet_buff;
	memset(request_packet, 0, sizeof(DBTABLE_REQUEST_PACKET));
	
	if(request->db_name)
	{
		if(strlen(request->db_name) < MAX_DB_NAME_LEN)
		{
			sprintf(request_packet->db_name, request->db_name);
		}
	}
	if(strlen(request->table_name) < MAX_TABLE_NAME_LEN)
	{
	    sprintf(request_packet->table_name, request->table_name);
	}
	else
	{
		return -1;
	}
	if(strlen(request->index_name) < MAX_INDEX_NAME_LEN)
	{
	    sprintf(request_packet->index_name, request->index_name);
	}
	else
	{
		return -1;
	}
	request_packet->flag = request->flag;
	request_packet->request_id = request->request_id;
	request_packet->request_type = request->request_type;
	request_packet->entry_size = request->request_entry.size;
	memcpy((char *)(request_packet + 1), request->request_entry.request_entry_data, request->request_entry.size);
	return sizeof(DBTABLE_REQUEST_PACKET) + request->request_entry.size;
}

int osal_db_table_request_parse_packet(char *packet_buff, DBTABLE_REQUEST *request)
{
	DBTABLE_REQUEST_PACKET *request_packet = (DBTABLE_REQUEST_PACKET *)packet_buff;

	if(request == NULL || packet_buff == NULL)
	{
		return -1;
	}
	if(strlen(request_packet->db_name) >= MAX_DB_NAME_LEN)
	{
		return -1;
	}
	if(strlen(request_packet->db_name) == 0)
	{
		request->db_name = NULL;
	}
	else
	{
		request->db_name = request_packet->db_name;
	}
	
	if(strlen(request_packet->table_name) >= MAX_TABLE_NAME_LEN)
	{
		return -1;
	}
	if(strlen(request_packet->table_name) == 0)
	{
		request->table_name = NULL;
	}
	else
	{
		request->table_name = request_packet->table_name;
	}
	
	if(strlen(request_packet->index_name) >= MAX_INDEX_NAME_LEN)
	{
		return -1;
	}
	if(strlen(request_packet->index_name) == 0)
	{
		request->index_name = NULL;
	}
	else
	{
		request->index_name = request_packet->index_name;
	}
	request->flag = request_packet->flag;
	request->request_id = request_packet->request_id;
	request->request_type = request_packet->request_type;
	request->request_entry.size = request_packet->entry_size;
	request->request_entry.request_entry_data = (request_packet + 1);
	return 0;
}

int osal_db_table_response_generate_packet(DBTABLE_RESPONSE *response, char *packet_buff)
{
	DBTABLE_RESPONSE_PACKET *response_packet = NULL;
	int packet_len = 0;
	int i = 0;
	char *current_ptr = NULL, *temp_ptr = NULL;

	if(response == NULL || packet_buff == NULL)
	{
		return -1;
	}
	if(response->table_name == NULL)
	{
		return -1;
	}
	response_packet = (DBTABLE_RESPONSE_PACKET *)packet_buff;
	memset(response_packet, 0, sizeof(DBTABLE_RESPONSE_PACKET));
	
	if(response->db_name)
	{
		if(strlen(response->db_name) < MAX_DB_NAME_LEN)
		{
			sprintf(response_packet->db_name, response->db_name);
		}
	}
	if(strlen(response->table_name) < MAX_TABLE_NAME_LEN)
	{
	    sprintf(response_packet->table_name, response->table_name);
	}
	else
	{
		return -1;
	}
	
	response_packet->flag = response->flag;
	response_packet->response_id = response->response_id;
	response_packet->response_type = response->response_type;
	response_packet->return_code = response->return_code;
	response_packet->entry_count = response->entry_count;
	response_packet->entry_size = response->entry_size;
	temp_ptr = current_ptr = (char *)(response_packet + 1);
	packet_len = sizeof(DBTABLE_RESPONSE_PACKET);
	/*规定每个响应的entry size相同，可以简化下面的计算*/
	for(i = 0; i < response_packet->entry_count; i++)
	{
		memcpy(current_ptr, response->response_entry[i].response_entry_data, response->entry_size);
		current_ptr += response->entry_size;
	}
	packet_len += (current_ptr - temp_ptr);
	return packet_len;
}

int osal_db_table_response_parse_packet(char *packet_buff, DBTABLE_RESPONSE *response)
{
	DBTABLE_RESPONSE_PACKET *response_packet = (DBTABLE_RESPONSE_PACKET *)packet_buff;
	DBTABLE_RESPONSE_ENTRY *reponse_entry = NULL;
    int i = 0;
	char *current_ptr = NULL;

	if(response == NULL || packet_buff == NULL)
	{
		return -1;
	}
	if(strlen(response_packet->db_name) >= MAX_DB_NAME_LEN)
	{
		return -1;
	}
	if(strlen(response_packet->db_name) == 0)
	{
		response->db_name = NULL;
	}
	else
	{
		response->db_name = response_packet->db_name;
	}
	
	if(strlen(response_packet->table_name) >= MAX_TABLE_NAME_LEN)
	{
		return -1;
	}
	if(strlen(response_packet->table_name) == 0)
	{
		response->table_name = NULL;
	}
	else
	{
		response->table_name = response_packet->table_name;
	}
	
	response->flag = response_packet->flag;
	response->response_id = response_packet->response_id;
	response->response_type = response_packet->response_type;
	response->return_code = response_packet->return_code;
	response->entry_count = response_packet->entry_count;
	response->entry_size = response_packet->entry_size;
	current_ptr = (char *)(response_packet + 1);
	response->response_entry = malloc(sizeof(DBTABLE_RESPONSE_ENTRY) * response_packet->entry_count);
	if(response->response_entry == NULL)
	{
		response->return_code = DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
		return 0;
	}
	memset(response->response_entry, 0, (sizeof(DBTABLE_RESPONSE_ENTRY) * response_packet->entry_count));
	for(i = 0; i < response_packet->entry_count; i++)
	{
		reponse_entry = &(response->response_entry[i]);
		reponse_entry->response_entry_data = malloc(response->entry_size);
		if(reponse_entry->response_entry_data == NULL)
		{
			response->return_code = DB_TABLE_RETURN_CODE_MEM_ALLOC_ERR;
		    return 0;
		}
		else
		{
		    memcpy(reponse_entry->response_entry_data, current_ptr, response->entry_size);
		}
		current_ptr += response->entry_size;
	}
	return 0;
}

DBTABLE_RESPONSE *osal_db_table_request(char *db_name, char *table_name, char *index_name, int event, void *data, int size)
{
	DBTABLE_REQUEST request;
	request.db_name = db_name;
	request.table_name = table_name;
	request.index_name = index_name;
	request.request_entry.request_entry_data = data;
	request.request_entry.size = size;
	request.request_type = event;
	return dbtable_request(&request);
}


#ifdef __cplusplus
}
#endif

