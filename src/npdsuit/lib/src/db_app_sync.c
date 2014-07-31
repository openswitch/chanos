
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_app_sync.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		Database sync between ACT and SBY for application.
*
* DATE:
*		10/27/2010
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
#include "util/npd_list.h"
#include "tipc_api/tipc_api.h"
#include "netif_index.h"
#include "npd_database.h"
#include "chassis_man_app.h"
#include "db_app_sync.h"


int (*app_log)(int level, char *fmt, ...);
unsigned int dbsync_app_service_type = 0;
unsigned int dbsync_app_local_slot = -1;
unsigned int dbsync_app_sbymaster_slot = -1;
unsigned int dbsync_app_actmaster_slot = -1;
unsigned int app_running_on_actmaster = FALSE;
unsigned int app_running_on_master = FALSE;
unsigned int app_running_on_slave = FALSE;
unsigned int app_running_on_slaveindp = FALSE;

int monitor_sock = -1;
int data_sock[MAX_CHASSIS_SLOT_COUNT];

int (*dbsync_monitor_cb)(int sock) = NULL;

int dbsync_switchovering = FALSE;

int* dbsync_monitor_sock_get()
{
    return &monitor_sock;
}

int* dbsync_data_sock_array_get()
{
    return (int*)data_sock;
}

int dbsync_monitor(int *sock)
{
    if(dbsync_monitor_cb)
        (*dbsync_monitor_cb)(*sock);
	return 0;
}


int dbtable_app_sync(char *buffer, int len, unsigned int sync_flag, int slot_index)
{
	npd_sync_msg_header_t *header = NULL;
    int op_ret = 0;
    int i;

    if(-1 == slot_index)
    {
        for(i = 0; i < MAX_CHASSIS_SLOT_COUNT; i++)
        {
            if(data_sock[i] != -1)
            {
	            header = (npd_sync_msg_header_t*)buffer;
			
	            npd_dbtable_header_hton(header);	
	
                op_ret = send(data_sock[i], buffer, len, 0);
            }
        }
    }
    else if(data_sock[slot_index] != -1)
    {
	    header = (npd_sync_msg_header_t*)buffer;
			
	    npd_dbtable_header_hton(header);	
	
        op_ret = send(data_sock[slot_index], buffer, len, 0);
    }
    else
        (*app_log)(LOG_ERR, "Error, Can not find sync socket for slot %d\n", slot_index+1);
    return op_ret;
    
}


int dbtable_sync_client_monitor(int sock)
{
	char buffer[256];
	int len = 0;
	int ret = TIPC_FAIL;
    struct tipc_event event;
	int new_socket = 0;
	int i = 0;

	if((len = read(sock, buffer, 256)) <= 0)
	{
		if(sock)
		{
		    close(sock);
		}
		return ret;
	}
	
	memcpy(&event, buffer, sizeof(event));
    switch(event.event)
    {
		case TIPC_PUBLISHED:
			for(i = event.found_lower; i <= event.found_upper; i++)
			{
				if(tipc_get_own_node() == event.port.node)
				{
					return ret;
				}
				
				new_socket = connect_to_service(dbsync_app_service_type, i);
                
                if(new_socket > 0)
				{
				    data_sock[INSTANCE_TO_SLOT(i)-1] = new_socket;
                    ret = dbtable_sync_slot_event(event.event, dbsync_app_service_type, INSTANCE_TO_SLOT(i)-1);
				}
			}
			return ret;
		case TIPC_WITHDRAWN:
			for(i = event.found_lower; i <= event.found_upper; i++)
			{
				if(tipc_get_own_node() == event.port.node)
				{
					return ret;
				}

                if(-1 != data_sock[INSTANCE_TO_SLOT(i)-1])
                {
                    shutdown(data_sock[INSTANCE_TO_SLOT(i)-1], SHUT_RDWR);
                    close(data_sock[INSTANCE_TO_SLOT(i)-1]);
                    data_sock[INSTANCE_TO_SLOT(i)-1] = -1;
                }
 			}
			return TIPC_SUCCESS;
		case TIPC_SUBSCR_TIMEOUT:
			break;
		default:
			break;
    }
	return ret;
}

int dbtable_sync_server_monitor(int sock)
{
    int sd;
    struct sockaddr_tipc client;
    unsigned int len = sizeof(client);

    sd = accept(sock, (struct sockaddr*)&client, &len);
    if(-1 == sd)
    {
        return -1;
    }
    else
    {
        int slot_index;
        unsigned int nRecvBuf = 1024*1024*10;
        setsockopt(sd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
        slot_index = tipc_node(client.addr.id.node) - 1;
        data_sock[slot_index] = sd;
        return 0;
    }
}

int dbsync_recv(int *fd)
{
	char * buffer = NULL;
	int len = 0;

	buffer = malloc(TIPC_MAX_USER_MSG_SIZE);
    if(NULL == buffer)
        return TIPC_FAIL;
	if((len = read(*fd, buffer, TIPC_MAX_USER_MSG_SIZE)) <= 0)
	{
        /*dbsync_recv also handle the connection break down event*/
/*        dbsync_chassis_switchover();
        
        dbsync_switchovering = FALSE;           
*/
        shutdown(*fd, SHUT_RDWR);
        close(*fd);
        *fd = -1;
        free(buffer);
		return TIPC_FAIL;
	}
    if(!app_running_on_actmaster)
        dbtable_recv(*fd, buffer, len, NULL);
    free(buffer);
    return 0;
}

int dbsync_chassis_switchover()
{
    int ret = -1;
    int i;


    if(!app_running_on_master)
        return 0;
        
    for(i = 0; i < MAX_CHASSIS_SLOT_COUNT; i++)
    {
        if(data_sock[i] != -1)
        {
            shutdown(data_sock[i],SHUT_RDWR);
            close(data_sock[i]);
            data_sock[i] = -1;
        }
    }

    if(!app_running_on_actmaster)
    {
        int sleep_times = 0;
        while(sleep_times < 30)
        {
            app_running_on_actmaster 
                 = app_act_master_running();
            if(app_running_on_actmaster)
               break;
            sleep(1);
            sleep_times++;
        }
    }
    else
    {
        int sleep_times = 0;
        while(sleep_times < 30)
        {
            app_running_on_actmaster 
                 = app_act_master_running();
            if(!app_running_on_actmaster)
               break;
            sleep(1);
            sleep_times++;
        }
    }
    if(app_running_on_actmaster)
    {
        dbsync_app_actmaster_slot = app_actmaster_slot_get() - 1;
        dbsync_app_sbymaster_slot = app_sbymaster_slot_get() - 1;
        shutdown(monitor_sock,SHUT_RDWR);
        close(monitor_sock);
        monitor_sock = -1;
    
        ret = tipc_client_sock_init(dbsync_app_service_type, 
                    &monitor_sock);
        if (0 != ret)
        {
            (*app_log)(LOG_ERR, "Fatal error, can not init tipc client sock.\n");
        }
        dbsync_monitor_cb = dbtable_sync_client_monitor; 
        dbsync_switchovering = TRUE;
        if(app_running_on_slaveindp != 1)
            app_db_init(dbtable_app_sync, NULL, app_running_on_actmaster);
    }
    if(!app_running_on_actmaster)
    {
        dbsync_app_actmaster_slot = app_actmaster_slot_get() - 1;
        dbsync_app_sbymaster_slot = app_sbymaster_slot_get() - 1;
        shutdown(monitor_sock,SHUT_RDWR);
        close(monitor_sock);
        monitor_sock = -1;
    
        ret = tipc_server_sock_init(dbsync_app_service_type, dbsync_app_local_slot+1, 
                    &monitor_sock);
        if (0 != ret)
        {
            (*app_log)(LOG_ERR, "Fatal error, can not init tipc client sock.\n");
        }
        dbsync_monitor_cb = dbtable_sync_server_monitor; 
        if(app_running_on_slaveindp != 1)
            app_db_init(dbtable_app_sync, NULL, app_running_on_actmaster);
     }
    return 0;
}

int dbtable_sync_slot_event(int event, int service_type, int slot_index)
{
    int ret = 0;
    int npd_sync_done = 0;
    
    if (event == TIPC_PUBLISHED)
    {
        if(app_running_on_actmaster)
        {
            int count = 0;
            (*app_log)(LOG_DEBUG, "Slot %d inserted.\n", slot_index+1);
            while(!npd_sync_done)
            {
                npd_sync_done = app_npd_sync_done_state_get(slot_index);
                count ++;
                if(count > 30)
                    break;
                else
                    usleep(20000);
            }
            ret = dbtable_slot_online_insert(slot_index);
        }
    }
#if 0    
    else if (event == TIPC_WITHDRAWN)
    {
        if(app_running_on_actmaster)
        {
            while(1)
            {
                app_running_on_actmaster 
                     = app_act_master_running();
                if(!app_running_on_actmaster)
                   break;
                sleep(1);
            }
            if(!app_running_on_actmaster)
            {
                int i;
                dbsync_app_actmaster_slot = app_actmaster_slot_get() - 1;
                dbsync_app_sbymaster_slot = app_sbymaster_slot_get() - 1;
                shutdown(monitor_sock, SHUT_RDWR);
                close(monitor_sock);
                monitor_sock = -1;
            
                ret = tipc_server_sock_init(dbsync_app_service_type, dbsync_app_local_slot,
                            &monitor_sock);
                if (0 != ret)
                {
                    (*app_log)(LOG_ERR, "Fatal error, can not init tipc client sock.\n");
                }
                dbsync_monitor_cb = dbtable_sync_server_monitor; 
            }
        }
        return 0;
    }
#endif
    return ret;
}

int dbtable_sync_init(
    unsigned int tipc_service_type,
    int (*log)(int level, char *fmt, ...),
    int (*db_sync_complete_handler)()
    )
{
    int ret;
    
    memset(data_sock, 0xff, sizeof(data_sock));
    app_log = log;
    app_running_on_actmaster = app_act_master_running();
    app_running_on_master = app_slot_work_mode_get();
    app_running_on_slaveindp = app_slave_indpnt_runget();
    dbsync_app_service_type = tipc_service_type;
    dbsync_app_local_slot = app_local_slot_get() - 1;
    dbsync_app_actmaster_slot = app_actmaster_slot_get() - 1;
    dbsync_app_sbymaster_slot = app_sbymaster_slot_get() - 1;

    if(app_running_on_slaveindp != 1)
        app_db_init(dbtable_app_sync, db_sync_complete_handler, app_running_on_actmaster);
    if(!app_running_on_actmaster)
    {
        ret = tipc_server_sock_init(dbsync_app_service_type, 
            dbsync_app_local_slot+1, &monitor_sock);
        if(TIPC_SUCCESS != ret)
        {
            (*app_log)(LOG_ERR, "Fatal error, Can not init dbsync monitor sock\n");
            return -1;
        }
        dbsync_monitor_cb = dbtable_sync_server_monitor;
    }
    else
    {
        ret = tipc_client_sock_init(dbsync_app_service_type, &monitor_sock);
        if(TIPC_SUCCESS != ret)
        {
            (*app_log)(LOG_ERR, "Fatal error, Can not init dbsync monitor sock\n");
            return -1;
        }
        dbsync_monitor_cb = dbtable_sync_client_monitor;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

