#ifndef _TIPC_API_H_
#define _TIPC_API_H_

enum osal_thread_evnet {OSAL_SERVER_CTRL, OSAL_THREAD_READ, OSAL_THREAD_WRITE, OSAL_THREAD_TIMER};

#define TIPC_SERVER_MODE 1
#define TIPC_CLIENT_MODE 2
#define TIPC_NORMAL_FD_MODE 3

#define BROADCAST_SLOT 0xFFFFFFFF

#define TIPC_BUFFER_OWNED  -5

typedef int (*osal_thread_handler)(int fd, char *buffer, int len, void * private_data);

typedef struct OSAL_THREAD_SESSION
{
	int service_type;
	int repeat_flag;
	int timer_val;
	int (*read_handler)(int, char *, int, void *);
	int (*write_handler)(int, char *, int, void *);
	int (*accept_handler)(int, void **);
	int (*timer_handler)(void *);
	struct thread * read_thread;
	struct thread * write_thread;
	struct thread * timer_thread;
	void * private_data;
	int max_length;
}osal_thread_session;

typedef struct OSAL_THREAD_MASTER_LIST
{
	int service_type;
	int running;
	struct thread_master * osal_thread_master;
	struct OSAL_THREAD_MASTER_LIST *next;
}osal_thread_master_list;
void osal_thread_master_pool_del(int service_type);
struct thread_master *osal_thread_master_pool_find(int service_type);
void osal_thread_event(struct thread_master * osal_thread_master, enum osal_thread_evnet osal_event, int fd, osal_thread_session * osal_session);
int osal_thread_master_pool_add(struct thread_master * osal_thread_master, int service_type);
void osal_thread_master_running_set(int service_type, int running);
int osal_thread_master_running_get(int service_type);
void osal_thread_master_run(int service_type);
int osal_thread_read_buffer_length_set(int service_type, int len);
int osal_register_read_fd(int fd, int service_type, osal_thread_handler func, void * private_data, int repeat_flag);
int osal_register_server_ctrl_fd(int fd, int service_type, int (*server_ctrl_handler)(int, void **), int(*read_hander)(int, char *, int, void *), void * private_data);

int osal_register_timer(int timeout, int service_type, int (*func)(void *), void * private_data, int repeat_flag);

typedef struct TIPC_THREAD_SESSION
{
	int service_type;
	int instance;
	int local_node;
	int remote_node;
	int server_or_client;
	char * buff;
	int len;
	int (*monitor_handler)(int, int, int);
	int (*accept_handler)(int, int, void **);/*ctrl sock, new sock, private data*/
	int (*read_handler)(int, char *, int, void *);
	void * private_data;
}tipc_thread_session;

typedef struct NPD_TIPC_SOCKET_LIST
{
	int sock;
	int service_type;
	int instance;
	struct NPD_TIPC_SOCKET_LIST *next;
}npd_tipc_socket_list;

/*如果支持多机框，可以改变下面两个宏，slot编号形式跟tipc id最好一致*/
#define CHASSIS_SLOT_TO_DST(chassis, slot) ((chassis << 12) | slot)

#define SLOT_TO_INSTANCE(slot) (0x1001000|slot)
#define INSTANCE_TO_SLOT(instance) (instance&0xFFF)

#define NODE_TO_SLOT(node) (node&0xFFFFFF)

#define TIPC_SUCCESS 0
#define TIPC_FAIL -1
#define TIPC_RX_TIMEOUT -2

#ifndef CHASSIS_SLOT_COUNT
#define CHASSIS_SLOT_COUNT 16
#endif
#define MAX_SLOT_NUM CHASSIS_SLOT_COUNT

#define FOREVER ~0

#define MAX_TIPC_SEND_LEN 10240

unsigned int tipc_get_own_node();
unsigned int tipc_get_peer_node(int sd);
int tipc_client_sock_init(int service_type, int *rtn_sock);
int tipc_server_sock_init(int service_type, int local_slot, int *rtn_sock);
void tipc_client_socket_pool_del(int service_type, int instance);
int tipc_client_socket_pool_add(int sock, int service_type, int instance);
int tipc_client_socket_pool_find(int service_type, int instance);

void tipc_server_socket_pool_del(int service_type, int instance);
int tipc_server_socket_pool_add(int sock, int service_type, int instance);
int tipc_server_socket_pool_find(int service_type, int instance);
extern int gettimeofday(struct timeval *, struct timezone *);
int connect_to_service(int service_type, int instance);
int tipc_client_sync_send(int service_type, int dest_slot, char *buffer, int len);
int tipc_client_async_send(int service_type, int dest_slot, char *buffer, int len);
int tipc_server_sync_send(int service_type, int dest_slot, char *buffer, int len);
int osal_thread_create( char* name,
                           void (*pfnFunc)( void * ),
                           unsigned long ulStartPriority,
                           unsigned long ulStackSize,
                           void * pArgs);
int tipc_server_init(int service_type, 
                   int local_slot, 
                   int(* user_read_handler)(int, char *, int, void *), 
                   int(* accept_callback)(int, int, void * *), 
                   void * private_data);

int tipc_client_init(int service_type, int(* monitor_handler)(int, int, int));

#define SERVICE_MONITOR_TYPE 8888

#define PORT_EVENT_NOTIFIER 1000
#define FILE_TRANSFER_SERVICE 1107
#define FILE_CLIENT_SERVICE 1108
#define DEFAULT_SERVICE_TYPE 1234
#define CLI_TUNNEL_SERVICE 1235
#define REMOTE_EXEC_SERVICE 1236
#define DBUS_RELAY_TIPC_SERVICE 3333
#define NAM_PACKET_TX_SERVICE 3230
#define NAM_PACKET_RX_SERVICE 2230
#define TIPC_APP_EVENT_SERVICE 6000

#endif

