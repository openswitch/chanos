/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*npd_pkt_list.c
*
*
*CREATOR:
*	chengjun@autelan.com
*
*DESCRIPTION:
*<some description about this file>
*
*DATE:
*******************************************************************************/
#ifdef __cplusplus
extern "C" 
{
#endif

#include "lib/osinc.h"

#include "util/npd_list.h"
#include "npd_pkt_list.h"
#include "npd_log.h"
#include "tipc_api/tipc_api.h"
#include "nam/nam_utilus.h"

extern void npd_init_tell_whoami
(
	char *tName,
	unsigned char lastTeller
);

// Creates a thread condition (wrapper for pthread_cond_init)
int npd_create_thread_condition(pthread_cond_t *theCondition) 
{
	switch(pthread_cond_init(theCondition, NULL)) 
    {
		case 0: // success
			break;
		case ENOMEM:
			return -1;
		default:
			return -1;
	}
	return 0;
}

// Frees a thread condition (wrapper for pthread_cond_destroy)
void npd_destroy_thread_condition(pthread_cond_t *theCondition) 
{
	pthread_cond_destroy(theCondition);
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
int npd_wait_thread_condition(pthread_cond_t *theCondition, pthread_mutex_t *theMutex) 
{
	return pthread_cond_wait(theCondition, theMutex);
}

// Wait for a thread condition (wrapper for pthread_cond_wait)
int npd_wait_thread_timed_condition(
            pthread_cond_t *theCondition, 
            pthread_mutex_t *theMutex, 
            struct timespec* pTimeout) 
{
	return pthread_cond_timedwait(theCondition, theMutex, pTimeout);
}

// Signal a thread condition (wrapper for pthread_cond_signal)
void npd_signal_thread_condition(pthread_cond_t *theCondition) 
{
	if(theCondition == NULL) return;
	
	pthread_cond_signal(theCondition);
}

// Creates a thread mutex (wrapper for pthread_mutex_init)
int npd_create_mutex(pthread_mutex_t *theMutex) 
{
	return pthread_mutex_init(theMutex, NULL);
}


// Free a thread mutex (wrapper for pthread_mutex_destroy)
void npd_destroy_mutex(pthread_mutex_t *theMutex)  {
	if(theMutex == NULL) return;
	pthread_mutex_destroy( theMutex );
}

// locks a mutex among threads at the specified address (blocking)
int npd_lock_mutex(pthread_mutex_t *theMutex) {
	return pthread_mutex_lock( theMutex );
}

// locks a mutex among threads at the specified address (non-blocking).
// CW_TRUE if lock was acquired, CW_FALSE otherwise
int npd_trylock_mutex(pthread_mutex_t *theMutex)
{
	if(theMutex == NULL) {
		return -1;
	}
	return pthread_mutex_trylock( theMutex );
}

// unlocks a mutex among threads at the specified address
void npd_unlock_mutex(pthread_mutex_t *theMutex) 
{
	if(theMutex == NULL) return;
	pthread_mutex_unlock( theMutex );
}


int npd_create_msg_list(npd_msg_list_t** msg_list)
{
    
    *msg_list = malloc(sizeof(npd_msg_list_t));
    if(NULL == *msg_list)
        return -1;

    memset(*msg_list, 0, sizeof(npd_msg_list_t));

    INIT_LIST_HEAD(&(*msg_list)->list);

    pthread_mutex_init(&(*msg_list)->mutex, NULL);
    pthread_cond_init(&(*msg_list)->cond, NULL);

    return 0;
}

int npd_destroy_msg_list(npd_msg_list_t *msg_list)
{
    if(msg_list)
        free(msg_list);
	return 0;
}


int npd_msg_list_lock(npd_msg_list_t * msg_list)
{
    return npd_lock_mutex(&msg_list->mutex);
}

void npd_msg_list_unlock(npd_msg_list_t *msg_list)
{
    return npd_unlock_mutex(&msg_list->mutex);
}

int npd_msg_list_wait(npd_msg_list_t * msg_list)
{
    return npd_wait_thread_condition(&msg_list->cond, &msg_list->mutex);
}

int npd_msg_list_timed_wait(npd_msg_list_t *msg_list, struct timespec *timeout)
{
    return npd_wait_thread_timed_condition(&msg_list->cond, &msg_list->mutex, 
        timeout);
}

int npd_msg_list_signal(npd_msg_list_t * msg_list)
{
    npd_signal_thread_condition(&msg_list->cond);
    return 0;
}

unsigned long npd_msg_list_count(npd_msg_list_t *  msg_list)
{
    return msg_list->count;
}
int npd_msg_list_add_head(npd_msg_list_t *msg_list, struct list_head *node)
{
    list_add(node, &msg_list->list);

	msg_list->count++;
	npd_pkt_list_signal(msg_list);
	return 0;
}

int npd_msg_list_get_head(npd_msg_list_t *msg_list, struct list_head **node)
{
    if(list_empty(&msg_list->list))
    {
        *node = NULL;
        return 0;
    }
    *node = msg_list->list.next;

    return 0;
}

int npd_msg_list_remove_head(npd_msg_list_t *msg_list, struct list_head **node)
{
    if(list_empty(&msg_list->list))
    {
        *node = NULL;
        return 0;
    }
    *node = msg_list->list.next;

    list_del(*node);
	msg_list->count--;
	return 0;	
}

int npd_msg_list_add_tail(npd_msg_list_t *msg_list, struct list_head *node)
{
    list_add_tail(node, &msg_list->list);

	msg_list->count++;
	npd_pkt_list_signal(msg_list);
	return 0;
	
}
int npd_msg_list_remove_tail(npd_msg_list_t *msg_list, struct list_head **node)
{
    if(list_empty(&msg_list->list))
    {
        *node = NULL;
        return 0;
    }
    *node = msg_list->list.prev;

    list_del(*node);
	msg_list->count--;
	return 0;	
}

int npd_msg_list_clear(npd_msg_list_t *msg_list, void (*delete_func)(struct list_head *node))
{
	struct list_head *node = NULL;
	while(1)
	{
		npd_msg_list_remove_head(msg_list, &node);
		if (node == NULL)
			break;

		if (delete_func != NULL)
			(*delete_func)(node);
	}
	return 0;
}

// No thread-safe
int npd_pkt_list_add_head(npd_pkt_list_t * pkt_list, void * data, int size)
{
    npd_pkt_t *new_pkt = malloc(sizeof(npd_pkt_t));

    if(NULL == new_pkt)
        return -1;
    new_pkt->data = data;
	new_pkt->len = size;
    list_add(&new_pkt->list, &pkt_list->list);

	pkt_list->count++;
	npd_pkt_list_signal(pkt_list);
	return 0;
}

// No thread-safe
void* npd_pkt_list_get_head(npd_pkt_list_t * pkt_list, int * size)
{
    npd_pkt_t *pkt;
    struct list_head *pos;

    if(list_empty(&pkt_list->list))
    {
        *size = 0;
        return NULL;
    }
    pos = pkt_list->list.next;

    pkt = list_entry(pos, npd_pkt_t, list);
    *size = pkt->len;

    return pkt->data;
}

// No thread-safe
void* npd_pkt_list_remove_head(npd_pkt_list_t * pkt_list, int * size)
{
	void* data;
    npd_pkt_t *pkt;
    struct list_head *pos;

    if(list_empty(&pkt_list->list))
    {
        *size = 0;
        return NULL;
    }
    pos = pkt_list->list.next;

    pkt = list_entry(pos, npd_pkt_t, list);
    *size = pkt->len;
    data = pkt->data;

    list_del(&pkt->list);
    free(pkt);

	pkt_list->count--;
	return data;
}

// No thread-safe
int npd_pkt_list_add_tail(npd_pkt_list_t * pkt_list, void * data, int size)
{
    npd_pkt_t *new_pkt = malloc(sizeof(npd_pkt_t));

    if(NULL == new_pkt)
        return -1;
    new_pkt->data = data;
	new_pkt->len = size;
    list_add_tail(&new_pkt->list, &pkt_list->list);

	pkt_list->count++;
	npd_pkt_list_signal(pkt_list);
	return 0;
}

// No thread-safe
void* npd_pkt_list_get_tail(npd_pkt_list_t * pkt_list, int * size)
{
    npd_pkt_t *pkt;
    struct list_head *pos;

    if(list_empty(&pkt_list->list))
    {
        *size = 0;
        return NULL;
    }
    pos = pkt_list->list.prev;

    pkt = list_entry(pos, npd_pkt_t, list);
    *size = pkt->len;

    return pkt->data;
}

// No thread-safe
void* npd_pkt_list_remove_tail(npd_pkt_list_t * pkt_list, int * size)
{
	void* data;
    npd_pkt_t *pkt;
    struct list_head *pos;

    if(list_empty(&pkt_list->list))
    {
        *size = 0;
        return NULL;
    }
    pos = pkt_list->list.prev;

    pkt = list_entry(pos, npd_pkt_t, list);
    *size = pkt->len;
    data = pkt->data;

    list_del(&pkt->list);
    free(pkt);

	pkt_list->count--;
	return data;
}

// No thread-safe
void npd_pkt_list_clear(npd_pkt_list_t * pkt_list, void (*delete_func)(void *))
{
	void* data;
    int size;

	while(1)
	{
		data = npd_pkt_list_remove_head(pkt_list, &size);
		if (data == NULL)
			break;

		if (delete_func != NULL)
			(*delete_func)(data);
	}
}
int npd_app_msg_dispatch(int fd, char *buffer, int len, void * private_data)
{
    npd_app_msg_ctrl *msg_ctrl = (npd_app_msg_ctrl *)private_data;
    npd_pkt_list_t *pkt_list = &(msg_ctrl->msg_list);
	
	if(len < 0)
	{
		return -1;
	}
	
	if(private_data == NULL)
	{
		return -1;
	}
	
    npd_pkt_list_lock(pkt_list);
    npd_pkt_list_add_tail(pkt_list, buffer, len);
    npd_pkt_list_unlock(pkt_list);
	return -5;/*return TIPC_BUFFER_OWNED;*/
}

unsigned int npd_app_msg_hander(void *arg)
{
	npd_app_msg_ctrl *msg_ctrl = (npd_app_msg_ctrl *)arg;
    npd_pkt_list_t *pkt_list = &(msg_ctrl->msg_list);
    int len;
    char *buff;
	
	npd_init_tell_whoami(msg_ctrl->module_name,0);
	
    while(1)
    {
        npd_pkt_list_lock(pkt_list);
        npd_pkt_list_wait(pkt_list);
		buff = npd_pkt_list_remove_head(pkt_list, &len);
		npd_pkt_list_unlock(pkt_list);
		while(buff)
		{
			if(msg_ctrl->func_handler)
			{
				(*msg_ctrl->func_handler)(buff, len);
			}
			free(buff);
            npd_pkt_list_lock(pkt_list);
    		buff = npd_pkt_list_remove_head(pkt_list, &len);
            npd_pkt_list_unlock(pkt_list);
		}
    }
	return 0;
}

int npd_app_msg_socket_register(int fd, char *module_name, npd_app_msg_handler func, int msg_size)
{
	npd_app_msg_ctrl *msg_ctrl = NULL;
	if(func == NULL)
	{
		return -1;
	}
	
	if(msg_size == 0)
	{
		return -1;
	}
	
	if(msg_size > NPD_APP_MSG_MAX_LEN)
	{
		npd_syslog_dbg("Max msg length is %d. Please modify NPD_APP_MSG_MAX_LEN\r\n", NPD_APP_MSG_MAX_LEN);
		return -1;
	}
	
	msg_ctrl = malloc(sizeof(npd_app_msg_ctrl));
	if(msg_ctrl == NULL)
	{
		npd_syslog_dbg("No enough memory to alloc app msg ctrl.\r\n");
		return -1;
	}
	memset(msg_ctrl, 0, sizeof(npd_app_msg_ctrl));
	if(module_name)
	{
	    msg_ctrl->module_name = module_name;
	}
	else
	{
		msg_ctrl->module_name = "appMsg";
	}
	msg_ctrl->msg_size = msg_size;
	msg_ctrl->func_handler = func;
    INIT_LIST_HEAD(&(msg_ctrl->msg_list.list));

    pthread_mutex_init(&(msg_ctrl->msg_list.mutex), NULL);
    pthread_cond_init(&(msg_ctrl->msg_list.cond), NULL);

	osal_register_read_fd(fd, NPD_APP_MSG_SERVICE, npd_app_msg_dispatch, msg_ctrl, 1);
	
	nam_thread_create(module_name, npd_app_msg_hander, msg_ctrl, 1, 0);
	return 0;
}

unsigned int npd_app_msg_thread_main(void *arg)
{
    npd_init_tell_whoami("npdMsgThread",0);
    osal_thread_read_buffer_length_set(NPD_APP_MSG_SERVICE, NPD_APP_MSG_MAX_LEN);
    osal_thread_master_run(NPD_APP_MSG_SERVICE);
	return 0;
}

void npd_app_msg_run()
{
	nam_thread_create("npdMsg", npd_app_msg_thread_main, NULL, 1, 0);
}

#ifdef __cplusplus
}
#endif

