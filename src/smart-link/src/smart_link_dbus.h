#ifndef __SMART_LINK_DBUS_H_
#define __SMART_LINK_DBUS_H_

#define SMART_LINK_GROUP_SIZE       8

enum 
{
    SL_MASTER,
    SL_SLAVE,
    SL_MAX
};

enum
{
    SL_PORT_DOWN,
    SL_PORT_UP
};

struct smart_link_port_s
{
    unsigned int netif_index;
    unsigned int status;
};

struct smart_link_s
{
    unsigned int id;
    unsigned int is_enable;
    unsigned int is_preempt;
    unsigned int instance;
    unsigned int master_port;
    unsigned int slave_port;
    struct smart_link_port_s port[SL_MAX];
    unsigned int advertise_vlan;
    npd_vbmp_t data_vlan;
#define master_index port[SL_MASTER].netif_index
#define master_status port[SL_MASTER].status
#define slave_index port[SL_SLAVE].netif_index
#define slave_status port[SL_SLAVE].status
};

void smart_link_master_board_set(int );
int smart_link_group_search(struct smart_link_s* );
int smart_link_group_update(struct smart_link_s* );
int smart_link_group_delete(struct smart_link_s* );
int smart_link_group_get_next(struct smart_link_s* );
int smart_link_db_init();
int smart_link_dbus_should_set();
int smart_link_dbus_sock_init();
int smart_link_recv_cmd(struct thread * );
int smart_link_send_cmd(struct thread * );

#endif
