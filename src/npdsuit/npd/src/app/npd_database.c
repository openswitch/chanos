
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
*		chengjun@autelan.com
*
* DESCRIPTION:
*		For NPD db sync.
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
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

#include "npd_database.h"
#define SERVICE_NPD_DB  0x8002

int npd_dbtable_recv(int fd, char* buf, int len, void *private_data)
{
#ifdef HAVE_CHASSIS_SUPPORT
    static int recv_first = 1;
    int nRecvBuf;
    if(len <= 0)
    {
        return 0;
    } 
#if 0	
    header = (npd_sync_msg_header_t *)buf;
	if (len < sizeof(npd_sync_msg_header_t))
        return 0;
		
	npd_dbtable_header_ntoh(header);
    if(SYS_LOCAL_MODULE_RUNNINGSTATE < LOCAL_SLAVE_READY)
        return -1;

    if((SYS_LOCAL_MODULE_RUNNINGSTATE < LOCAL_SLAVE_RUNNING)
        && (header->op != NPD_SYNC_OP_TABLE_ADD))
        return -1;
#endif
    dbtable_recv(fd, buf, len, private_data);
	if(recv_first == 1)
	{
		recv_first = 0;
        nRecvBuf = 320 * NPD_FDB_TABLE_SIZE;
        setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
	}
#endif
    return 0;
}

int npd_dbtable_slot_sync_begin(int slot_index)
{
#ifdef HAVE_CHASSIS_SUPPORT
    char buf[4];
    char filename[32];

    sprintf(filename, NPD_DBSYNC_DONE_STATE_FILE, slot_index);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", 0); 
	write_to_file(filename, buf, strlen(buf));
#endif
	return NPD_SUCCESS;
}

int npd_dbtable_slot_sync_done(int slot_index)
{
    char buf[4];
    char filename[32];

    sprintf(filename, NPD_DBSYNC_DONE_STATE_FILE, slot_index);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", 1); 
	write_to_file(filename, buf, strlen(buf));
    return NPD_SUCCESS;
}

int npd_dbtable_sync_alldone()
{
    char buf[4];
    char filename[32];

    sprintf(filename, NPD_DBSYNC_ALLDONE_STATE_FILE);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%d", 1); 
	write_to_file(filename, buf, strlen(buf));
	return NPD_SUCCESS;
}

int npd_dbtable_slot_event(int event, int service_type, int instance)
{
#ifdef HAVE_CHASSIS_SUPPORT
	int slot_index = INSTANCE_TO_SLOT(instance);
    int ret;
    if (event == TIPC_PUBLISHED)
    {
        if (service_type != SERVICE_NPD_DB)
            return -1;
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
        {
            int nRecvBuf;
			int sock = 0;
            nRecvBuf = 320 * NPD_FDB_TABLE_SIZE;
	        sock = tipc_client_socket_pool_find(service_type, instance);
			if(sock > 0)
                setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nRecvBuf,sizeof(int));
			
            ret = dbtable_slot_online_insert(slot_index);
            sleep(2);
            npd_dbtable_slot_sync_done(slot_index-1);
        }
    }
    else if (event == TIPC_WITHDRAWN)
    {        
		if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
        {
			npd_key_database_lock();
			netif_notify_event(slot_index-1, NOTIFIER_SLOT_REMOVE);
			netif_app_notify_event(slot_index-1, NOTIFIER_SLOT_REMOVE, NULL, 0);
			npd_key_database_unlock();
		}
        return 0;
    }
#endif
    return 0;
}

int npd_dbtable_sync_startup()
{
	system("echo 0 > /var/run/npd_sync_over");
	return 0;
}

int npd_dbtable_sync_complete_handler()
{
	system("echo 1 > /var/run/npd_sync_over");
    npd_startup_end = 1;
    SYS_LOCAL_MODULE_SYNC_OVER_CONF;
    npd_eth_port_startup_end_update();
	return 0;
}

#ifdef HAVE_FDB_SW_SYNC
char *sync_buffer = NULL;
int sync_num = 0;
int sync_len = 0;
#define MAX_ASYNC_MSG_NUM 128
pthread_mutex_t fdb_async_lock = PTHREAD_MUTEX_INITIALIZER;
int npd_fdb_async_lock()
{
    return pthread_mutex_lock(&fdb_async_lock);
}

int npd_fdb_async_unlock()
{
    return pthread_mutex_unlock(&fdb_async_lock);
}
#endif

int npd_dbtable_sync(char *buffer, int len, unsigned int sync_flag, int slot_index)
{
    int op_ret = 0;
#ifdef HAVE_CHASSIS_SUPPORT
	npd_sync_msg_header_t *header = NULL;
	char *buffer_head = NULL;
    int i;

	header = (npd_sync_msg_header_t*)buffer;
#ifdef HAVE_FDB_SW_SYNC	
	if((sync_flag & DB_SYNC_ASYNC) && (header->entry_num == 1))/*暂时只支持FDB*/
	{
		if(sync_buffer == NULL)
		{
			npd_fdb_async_lock();
			sync_buffer = malloc(header->total_len*MAX_ASYNC_MSG_NUM);
			if(sync_buffer != NULL)
			{
				memcpy(sync_buffer, buffer, header->total_len);
				sync_len = header->total_len;
				sync_num = 1;
				npd_dbtable_header_hton((npd_sync_msg_header_t *)(sync_buffer));
				npd_fdb_async_unlock();
				return 0;
			}
			npd_fdb_async_unlock();
		}
		else
		{
			npd_fdb_async_lock();
			
			buffer_head = sync_buffer;
			memcpy(buffer_head + sync_len, buffer, header->total_len);
			npd_dbtable_header_hton((npd_sync_msg_header_t *)(buffer_head + sync_len));
			sync_len += header->total_len;
			sync_num += 1;
			if(sync_num >= MAX_ASYNC_MSG_NUM)
			{
				op_ret = tipc_client_sync_send(SERVICE_NPD_DB, slot_index, sync_buffer, sync_len);
				sync_len = 0;
				sync_num = 0;
				npd_fdb_async_unlock();
				return op_ret;
			}
			npd_fdb_async_unlock();
			return 0;
		}
	}
#endif	
	npd_dbtable_header_hton(header);	
	
    if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        if (sync_flag & DB_SYNC_ALL)
        {
            op_ret = tipc_client_sync_send(SERVICE_NPD_DB, slot_index, buffer, len);
			if(op_ret < 0)
			{
			}
		}
        else if (sync_flag & DB_SYNC_SERVICE)
        {
            for (i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
            {
                if (i == SYS_LOCAL_MODULE_SLOT_INDEX)
                    continue;

                if (SYS_MODULE_SLOT_INTERNAL_SERVICE(i))
                    op_ret = tipc_client_sync_send(SERVICE_NPD_DB, i + 1, buffer, len);
            }
        }
        else if (sync_flag & DB_SYNC_MCU)
        {
            for (i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
            {
                if (i == SYS_LOCAL_MODULE_SLOT_INDEX)
                    continue;
                
                if (SYS_MODULE_SLOT_ISMCU(i))
                    op_ret = tipc_client_sync_send(SERVICE_NPD_DB, i + 1, buffer, len);
            }
        }
    }
    else
    {
        op_ret = tipc_server_sync_send(SERVICE_NPD_DB, SYS_MASTER_ACTIVE_SLOT_INDEX + 1,
                                       buffer, len);
    }
#endif	
    return op_ret;
    
}

int npd_dbtable_init()
{
    if(SYS_MODULE_SLOT_ISMCU(SYS_LOCAL_MODULE_SLOT_INDEX)
        || !SYS_LOCAL_MODULE_ISMASTERACTIVE)
        return app_db_init(npd_dbtable_sync, npd_dbtable_sync_complete_handler, SYS_LOCAL_MODULE_ISMASTERACTIVE);
    return 0;
}
#ifdef HAVE_FDB_SW_SYNC
#define NPD_DBTABLE_HW_SW_SYNCTIMER 30

unsigned int npd_fdb_async_thread_main(void *arg)
{
	while(1)
	{
		if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
		{
    		if(sync_buffer)
    		{
    			npd_fdb_async_lock();
    			if(sync_num > 0 && sync_len > sizeof(npd_sync_msg_header_t))
    			{
    				tipc_client_sync_send(SERVICE_NPD_DB, 0xFFFFFFFF, sync_buffer, sync_len);
    				sync_len = 0;
    				sync_num = 0;
    			}
    			npd_fdb_async_unlock();
    		}
		}
		sleep(1);
	}
    return 0;
}
void npd_fdb_async_thread_start()
{
	nam_thread_create("fdb_async", npd_fdb_async_thread_main, NULL, FALSE, FALSE);
}
#endif

#ifdef HAVE_CHASSIS_SUPPORT
int npd_dbtable_thread_main()
{
#ifdef HAVE_CHASSIS_SUPPORT	
	npd_dbtable_init();
    if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        tipc_server_init(SERVICE_NPD_DB, SYS_LOCAL_MODULE_SLOT_INDEX + 1, npd_dbtable_recv, NULL, NULL);
        tipc_client_init(SERVICE_NPD_DB, npd_dbtable_slot_event);
    }        
    else if(SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        tipc_server_init(SERVICE_NPD_DB, SYS_LOCAL_MODULE_SLOT_INDEX + 1, npd_dbtable_recv, NULL, NULL);
        tipc_client_init(SERVICE_NPD_DB, npd_dbtable_slot_event);
    }
    else
        tipc_server_init(SERVICE_NPD_DB, SYS_LOCAL_MODULE_SLOT_INDEX + 1, npd_dbtable_recv, NULL, NULL);
        
    //osal_register_timer(NPD_DBTABLE_HW_SW_SYNCTIMER, SERVICE_NPD_DB, dbtable_sync_hwsw_timer, NULL, 1);
#ifdef HAVE_FDB_SW_SYNC
    npd_fdb_async_thread_start();
#endif
    osal_thread_master_run(SERVICE_NPD_DB);
#endif
    return 0;
}
#endif
#ifdef __cplusplus
}
#endif

