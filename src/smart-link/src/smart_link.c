
#ifdef HAVE_SMART_LINK
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include "lib/npd_bitop.h"
#include "quagga/thread.h"

#include "smart_link.h"
#include "smart_link_dbus.h"
#include "smart_link_packet.h"
#include "smart_link_event.h"
#include "smart_link_log.h"

struct thread_master* sl_master;

void smart_link_event(enum sl_event event, int sock)
{
    switch (event)
    {
        case SL_EVENT:
        {
            smart_link_log_event("sl-event event, fd(%d).\n", sock);
            thread_add_read(sl_master, smart_link_recv_event, NULL, sock);
            break;
        }
        case SL_MSG:
        {
            smart_link_log_event("sl-event msg, fd(%d).\n", sock);
            thread_add_read(sl_master, smart_link_recv_msg, NULL, sock);
            break;
        }
        case SL_DBUS_READ:
        {
            smart_link_log_event("sl-event dbus-read, fd(%d).\n", sock);
            thread_add_read(sl_master, smart_link_recv_cmd, NULL, sock);
            break;
        }
        case SL_DBUS_WRITE:
        {
            smart_link_log_event("sl-event dbus-write, fd(%d).\n", sock);
            thread_add_write(sl_master, smart_link_send_cmd, NULL, sock);
            break;
        }
        default :
        {
            smart_link_log_error("sl-event default, fd(%d).\n", sock);
            break;
        }
    }
}

void smart_link_master()
{
    struct thread thread;
    
    while (thread_fetch(sl_master, &thread))
    {
        thread_call (&thread);
    }
}

int main()
{
//    (void)smart_link_log_type_set(0xff);

    sl_master = thread_master_create ();
    
    if (0 != smart_link_db_init())
    {
        smart_link_log_error("Smart-link data-base init failed.\n");
        return -1;
    }

    if (0 != smart_link_app_event_init())
    {
        smart_link_log_error("Smart-link app event init failed.\n");
        return -1;
    }

    if (0 != smart_link_app_msg_sock_init())
    {
        smart_link_log_error("Smart-link msg socket init failed.\n");
        return -1;
    }

    if (0 != smart_link_packet_socket_init())
    {
        smart_link_log_error("Smart-link socket init failed.\n");
        return -1;
    }
        
    if (0 != smart_link_dbus_sock_init())
    {
        smart_link_log_error("Smart-link socket init failed.\n");
        return -1;
    }

    if (0 != smart_link_sysmac_init())
    {
        smart_link_log_error("Smart-link get system MAC failed.\n");
    }
    app_module_inst_set("smart-link", getpid());
//    (void)smart_link_log_type_unset(0xff);

    smart_link_master();

    smart_link_log_error("Game Over!\n");
    thread_master_free(sl_master);

    return 0;
}

#endif

