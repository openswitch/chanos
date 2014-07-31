#ifndef __SMART_LINK_EVENT_H_
#define __SMART_LINK_EVENT_H_

#define SMART_LINK_DBSYNC_SERVICE 7531
#define SMART_LINK_TIPC_APP_INSTANCE  31

struct smart_link_event_s
{
    int event;
    int (*func)(unsigned int, unsigned int, int, char*);
};

int smart_link_app_event_init();
void smart_link_send_msg_to_npd
(
    unsigned int,
    unsigned int,
    unsigned int,
    unsigned int 
);

int smart_link_recv_event(struct thread * );

#endif

