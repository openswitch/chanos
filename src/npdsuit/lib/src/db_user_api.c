
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* db_user_api.c
*
*
* CREATOR:
*		zhanwei@autelan.com
*
* DESCRIPTION:
*		DB user api. Using this api to access a db server.
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

#include "tipc_api/tipc_api.h"
#include "npd_list.h"
#include "db_usr_api.h"

int db_request_id = 0;

int db_printf_enable=0;
#define db_debug_printf if(db_printf_enable) printf

DB_CONNECT_SESSION *app_db_local_open(char *name)
{
	int len = 0;
	char path_name[32];
	DB_CONNECT_SESSION *db_connect_session = NULL;
	struct sockaddr_un servaddr={0};
    int sockfd = 0;
	if(name == NULL)
	{
		db_debug_printf("(%s:%d) DB name is NULL.\r\n", __func__, __LINE__);
		return NULL;
	}
	if((len = strlen(name)) >= MAX_DB_NAME_LEN)
	{
		db_debug_printf("(%s:%d) DB name:%s is too long.\r\n", __func__, __LINE__, name);
		return NULL;
	}
	memset(path_name, 0, 32);
	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		db_debug_printf("(%s:%d) Create AF_LOCAL SOCK_STREAM socket failed.\r\n", __func__, __LINE__);
		return NULL;
	}
	sprintf(path_name, "%s%s", DB_LOCAL_SERVICE_PATH_PREFIX, name);
    servaddr.sun_family   =   AF_LOCAL;
    strcpy(servaddr.sun_path,   path_name);
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        db_debug_printf( "connect error: %s, ", __func__);
        db_debug_printf( " %s\n ",strerror(errno));
		close(sockfd);
        return NULL;
    }
	db_connect_session = malloc(sizeof(DB_CONNECT_SESSION));
	if(db_connect_session == NULL)
	{
		close(sockfd);
		db_debug_printf("(%s:%d) Alloc memory for DB_CONNECT_SESSION failed.\r\n", __func__, __LINE__);
		return NULL;
	}
	memset(db_connect_session, 0, sizeof(DB_CONNECT_SESSION));
	db_connect_session->sock = sockfd;
	db_connect_session->name = malloc(MAX_DB_NAME_LEN);
	if(db_connect_session->name == NULL)
	{
		close(sockfd);
		free(db_connect_session);
		db_debug_printf("(%s:%d) Alloc memory for name failed.\r\n", __func__, __LINE__);
		return NULL;
	}
	memset(db_connect_session->name, 0, MAX_DB_NAME_LEN);
	strncpy(db_connect_session->name, name, len);

	return db_connect_session;
}

void app_db_conn_close(DB_CONNECT_SESSION *db_conn)
{
	if(db_conn)
	{
		if(db_conn->sock)
		{
			close(db_conn->sock);
		}
		if(db_conn->name)
		{
			free(db_conn->name);
		}
		free(db_conn);
	}
}

DB_CONNECT_SESSION *app_db_tipc_open(char *db_name, int service_type, int chassis, int dest_slot)
{
	int len = 0;
	int sock = -1;
	DB_CONNECT_SESSION *db_connect_session = NULL;
	int logic_slot = CHASSIS_SLOT_TO_DST(chassis, dest_slot);
	if(db_name == NULL)
	{
		return NULL;
	}
	if((len = strlen(db_name)) >= MAX_DB_NAME_LEN)
	{
		return NULL;
	}
	
    sock = connect_to_service(service_type, SLOT_TO_INSTANCE(logic_slot));

    if (sock <= 0)
	{
        db_debug_printf("failed to connect to server %d,instance %d\r\n",service_type, SLOT_TO_INSTANCE(logic_slot));
        return NULL;
    }
	db_connect_session = malloc(sizeof(DB_CONNECT_SESSION));
	if(db_connect_session == NULL)
	{
		close(sock);
		db_debug_printf("(%s:%d) Alloc memory for DB_CONNECT_SESSION failed.\r\n", __func__, __LINE__);
		return NULL;
	}
	memset(db_connect_session, 0, sizeof(DB_CONNECT_SESSION));
	db_connect_session->sock = sock;
	db_connect_session->name = malloc(MAX_DB_NAME_LEN);
	if(db_connect_session->name == NULL)
	{
		close(sock);
		free(db_connect_session);
		db_debug_printf("(%s:%d) Alloc memory for name failed.\r\n", __func__, __LINE__);
		return NULL;
	}
	memset(db_connect_session->name, 0, MAX_DB_NAME_LEN);
	strncpy(db_connect_session->name, db_name, len);
	return db_connect_session;
	
}

int app_db_send_request(DB_CONNECT_SESSION *db_conn, DBTABLE_REQUEST *request)
{
	int len = 0;
	int ret = -1;
	char *request_packet = NULL;
	if(request == NULL)
	{
		return -1;
	}
	if(request->db_name == NULL)
	{
		return -1;
	}
	if(request->table_name == NULL)
	{
		return -1;
	}
	if(request->index_name == NULL)
	{
		return -1;
	}
	if(request->request_entry.request_entry_data == NULL)
	{
		return -1;
	}
	if(db_conn == NULL)
	{
		return -1;
	}
	if(db_conn->sock <= 0)
	{
		return -1;
	}
	request_packet = malloc(MAX_DB_PACKET_LEN);
	if(request_packet == NULL)
	{
		return -1;
	}
	len = osal_db_table_request_generate_packet(request, request_packet);
	if(len > 0)
	{
		ret = write(db_conn->sock, request_packet, len);
		if(ret < 0)
		{
			free(request_packet);
			return -1;
		}
	}
	free(request_packet);
	return 0;
}
DBTABLE_RESPONSE *app_db_get_response(DB_CONNECT_SESSION *db_conn)
{
	int ret = -1;
	int sock = 0;
	int len = -1;
	int num_sock = 0;
  	struct timeval    tv; 
	fd_set rfds;
	DBTABLE_RESPONSE *response = NULL;
	char *response_packet = NULL;
	FD_ZERO(&rfds);
    if(db_conn == NULL)
    {
		return NULL;
    }
	if(db_conn->sock <= 0)
	{
		return NULL;
	}
	sock = db_conn->sock;
	response_packet = malloc(MAX_DB_PACKET_LEN);
	if(response_packet == NULL)
	{
		return NULL;
	}
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
	num_sock = sock+1;
    tv.tv_sec = 10;
    tv.tv_usec = 500000;	
	if(select(num_sock, &rfds, NULL, NULL, &tv) < 0)
	{
		free(response_packet);
        db_debug_printf("Select error:%s\r\n", strerror(errno));
		return NULL;
	}
	if(FD_ISSET(sock, &rfds))
    {
		len = read(sock, response_packet, MAX_DB_PACKET_LEN);
		if(len <= 0)
		{
			db_debug_printf("Read response packet from socket %d error.\r\n", sock);
			FD_CLR(sock, &rfds);
		}
		else
		{
			if(len == MAX_DB_PACKET_LEN)
			{
				db_debug_printf("Recv data length warning.\r\n");
			}
		}
    }	
	if(len >= sizeof(DBTABLE_RESPONSE_PACKET))
	{
		response = malloc(sizeof(DBTABLE_RESPONSE));
		if(response == NULL)
		{
			db_debug_printf("(%s:%d) Alloc memory for DBTABLE_RESPONSE failed.\r\n", __func__, __LINE__);
			return NULL;
		}
		memset(response, 0, sizeof(DBTABLE_RESPONSE));
		ret = osal_db_table_response_parse_packet(response_packet, response);
		if(ret == 0 && response != NULL)
		{
			free(response_packet);
			return response;
		}
		else
		{
			free(response_packet);
			dbtable_response_free(response);
			return NULL;
		}
	}
	else
	{
		free(response_packet);
	    return NULL;
	}
}
DBTABLE_RESPONSE *app_db_table_request(DB_CONNECT_SESSION *db_conn, char *table_name, char *index_name, int event, void *data, int size)
{
	int ret = -1;
	DBTABLE_RESPONSE *response = NULL;
	DBTABLE_REQUEST request;
	if(db_conn == NULL)
	{
		return NULL;
	}
	request.db_name = db_conn->name;
	request.table_name = table_name;
	request.index_name = index_name;
	request.request_entry.request_entry_data = data;
	request.request_entry.size = size;
	request.request_type = event;
	request.request_id = db_request_id++;
	ret = app_db_send_request(db_conn, &request);
	if(ret !=0 )
	{
		db_debug_printf("(%s:%d) Send db(%s:%s) request packet failed.\r\n", __func__, __LINE__,  db_conn->name, table_name);
		return NULL;
	}
	response = app_db_get_response(db_conn);
	if(response == NULL)
	{
		db_debug_printf("(%s:%d) Get response from db(%s:%s) failed.\r\n", __func__, __LINE__,  db_conn->name, table_name);
		return NULL;
	}
	return response;
}
#ifdef __cplusplus
}
#endif

