#ifndef __MAN_SMART_LINK_H_
#define __MAN_SMART_LINK_H_

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



struct man_smart_link_port_s
{
    unsigned int netif_index;
    unsigned int status;
};

struct man_smart_link_request_s
{
    unsigned int id;
    unsigned int is_enable;
    unsigned int is_preempt;
    unsigned int instance;
    unsigned int master_port;
    struct man_smart_link_port_s port[SL_MAX];
    unsigned int advertise_vlan;
#define master_index port[SL_MASTER].netif_index
#define master_status port[SL_MASTER].status
#define slave_index port[SL_SLAVE].netif_index
#define slave_status port[SL_SLAVE].status
};

#define MAN_ADV_VLAN_LIST_MAX   8
struct man_smart_link_port_list_s
{
    unsigned int netif_index;   /* XXX: just as global_port_ifindex in switch_port_db_s*/
    short adv_vlan_list[MAN_ADV_VLAN_LIST_MAX];
};

#endif
