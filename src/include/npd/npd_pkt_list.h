#ifndef __NPD_PACKET_LIST_H_
#define __NPD_PACKET_LIST_H_

#include "npd/npd_list.h"
typedef struct 
{
    struct list_head list;
	void* data;
	int len;
} npd_pkt_t;

typedef struct 
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	unsigned long count;
    struct list_head list;
} npd_msg_list_t;

typedef npd_msg_list_t npd_pkt_list_t;

#define NPD_APP_MSG_MAX_LEN 1024
#define NPD_APP_MSG_SERVICE 2000

typedef int (*npd_app_msg_handler)(char *msg, int len);

typedef  struct _NPD_APP_MSG_CTRL_
{
	npd_pkt_list_t msg_list;
	int msg_size;
	char *module_name;
	npd_app_msg_handler func_handler;
}npd_app_msg_ctrl;

int npd_create_thread_condition(pthread_cond_t *theCondition) ;


// Frees a thread condition (wrapper for pthread_cond_destroy)
void npd_destroy_thread_condition(pthread_cond_t *theCondition) ;

// Wait for a thread condition (wrapper for pthread_cond_wait)
int npd_wait_thread_condition(pthread_cond_t *theCondition, pthread_mutex_t *theMutex); 

// Wait for a thread condition (wrapper for pthread_cond_wait)
int npd_wait_thread_timed_condition(
            pthread_cond_t *theCondition, 
            pthread_mutex_t *theMutex, 
            struct timespec* pTimeout); 

// Signal a thread condition (wrapper for pthread_cond_signal)
void npd_signal_thread_condition(pthread_cond_t *theCondition) ;


// Creates a thread mutex (wrapper for pthread_mutex_init)
int npd_create_mutex(pthread_mutex_t *theMutex) ;


// Free a thread mutex (wrapper for pthread_mutex_destroy)
void npd_destroy_mutex(pthread_mutex_t *theMutex);

// locks a mutex among threads at the specified address (blocking)
int npd_lock_mutex(pthread_mutex_t *theMutex);
// locks a mutex among threads at the specified address (non-blocking).
// CW_TRUE if lock was acquired, CW_FALSE otherwise
int npd_trylock_mutex(pthread_mutex_t *theMutex);

// unlocks a mutex among threads at the specified address
void npd_unlock_mutex(pthread_mutex_t *theMutex) ;

int npd_create_msg_list(npd_msg_list_t **msg_list);
#define npd_create_pkt_list npd_create_msg_list

int npd_destroy_msg_list(npd_msg_list_t *msg_list);
#define npd_destroy_pkt_list npd_destroy_msg_list

int npd_msg_list_set_mutex(npd_msg_list_t *msg_list, pthread_mutex_t* mutex);
#define npd_pkt_list_set_mutex npd_msg_list_set_mutex

void npd_msg_list_set_cond(npd_msg_list_t *msg_list, pthread_cond_t* cond);
#define  npd_pkt_list_set_cond npd_msg_list_set_cond

int npd_msg_list_lock(npd_msg_list_t *msg_list);
#define npd_pkt_list_lock  npd_msg_list_lock

void npd_msg_list_unlock(npd_msg_list_t *msg_list);
#define  npd_pkt_list_unlock npd_msg_list_unlock

int npd_msg_list_wait(npd_msg_list_t *msg_list);
#define  npd_pkt_list_wait npd_msg_list_wait

int npd_msg_list_signal(npd_msg_list_t * msg_list);
#define npd_pkt_list_signal npd_msg_list_signal

unsigned long npd_msg_list_count(npd_msg_list_t * msg_list);
#define npd_pkt_list_count npd_msg_list_count

int npd_msg_list_add_head(npd_msg_list_t *msg_list, struct list_head *node);
int npd_msg_list_get_head(npd_msg_list_t *msg_list, struct list_head **node);
int npd_msg_list_remove_head(npd_msg_list_t *msg_list, struct list_head **node);
int npd_msg_list_add_tail(npd_msg_list_t *msg_list, struct list_head *node);
int npd_msg_list_remove_tail(npd_msg_list_t *msg_list, struct list_head **node);
int npd_msg_list_clear(npd_msg_list_t *msg_list, void (*delete_func)(struct list_head *node));


int npd_pkt_list_add_head(npd_pkt_list_t * pkt_list, void* data, int size);
void* npd_pkt_list_get_head(npd_pkt_list_t *  pkt_list, int* size);
void* npd_pkt_list_remove_head(npd_pkt_list_t *  pkt_list, int* size);
int npd_pkt_list_add_tail(npd_pkt_list_t *  pkt_list, void* data, int size);
void* npd_pkt_list_remove_tail(npd_pkt_list_t *  pkt_list, int* size);
void npd_pkt_list_clear(npd_pkt_list_t *  pkt_list, void (*delete_func)(void *));
int npd_app_msg_dispatch(int fd, char *buffer, int len, void * private_data);
int npd_app_msg_socket_register(int fd, char *module_name, npd_app_msg_handler func, int msg_size);

void npd_app_msg_run();

#endif
 
