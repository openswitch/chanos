#ifdef HAVE_SMART_LINK
#include <string.h>
#include <unistd.h>

#include "util/npd_list.h"
#include "lib/db_app_sync.h"
#include "lib/netif_index.h"
#include "npd/npd_netif_event.h"
#include "lib/npd_bitop.h"
#include "quagga/thread.h"

#include "smart_link.h"
#include "smart_link_log.h"
#include "smart_link_dbus.h"
#include "smart_link_event.h"

struct smart_link_event_s sl_event_cmd[PORT_NOTIFIER_TYPE_MAX];
struct smart_link_event_s sl_relate_event_cmd[PORT_NOTIFIER_RELATE_MAX];

void smart_link_app_event_install(int event, int (*func)(unsigned int, unsigned int, int, char*))
{
    if ((0 == sl_event_cmd[event].event)
        && (NULL == sl_event_cmd[event].func))
    {
        sl_event_cmd[event].event = event;
        sl_event_cmd[event].func = func;
    }
    else
    {
        smart_link_log_error("App event (%d) init failed.\n", event);
    }

    return ;
}

void smart_link_app_relate_event_install(int event, int (*func)(unsigned int, unsigned int, int, char*))
{
    if ((0 == sl_relate_event_cmd[event].event)
        && (NULL == sl_relate_event_cmd[event].func))
    {
        sl_relate_event_cmd[event].event = event;
        sl_relate_event_cmd[event].func = func;
    }
    else
    {
        smart_link_log_error("App relate event (%d) init failed.\n", event);
    }

    return ;
}

struct smart_link_event_s* smart_link_app_event_find(int event)
{
    if (event == sl_event_cmd[event].event)
    {
        return &sl_event_cmd[event];
    }
    else
    {
        smart_link_log_error("App event (%d) callback-handler not find.\n", event);
    }

    return NULL;
}

struct smart_link_event_s* smart_link_app_relate_event_find(int event)
{
    if (event == sl_relate_event_cmd[event].event)
    {
        return &sl_relate_event_cmd[event];
    }
    else
    {
        smart_link_log_error("App relate event (%d) callback-handler not find.\n", event);
    }

    return NULL;
}

int smart_link_app_event_linkup(unsigned int netif_index, unsigned int son_netif_index, int length, char* data)
{
    struct smart_link_s entry;
    
    smart_link_log_event("%s, netif:%x, length:%d, data:%s\n", __func__, netif_index, length, data);

    memset(&entry, 0, sizeof(struct smart_link_s));
    
    while (0 == smart_link_group_get_next(&entry))
    {
        if ((netif_index == entry.master_index) || (netif_index == entry.slave_index))
        {
            if ((SL_PORT_DOWN == entry.master_status) && (SL_PORT_DOWN == entry.slave_status))
            {
                if (netif_index == entry.master_index)
                {
                    entry.master_status = SL_PORT_UP;
                }
                else
                {
                    entry.slave_status = SL_PORT_UP;
                }
                entry.master_port = netif_index;
            }
            else if ((SL_PORT_UP == entry.master_status) && (SL_PORT_DOWN == entry.slave_status))
            {
                if (netif_index == entry.slave_index)
                {
                    entry.slave_port = netif_index;
                    entry.slave_status = SL_PORT_UP;
                }
                else
                {
                    smart_link_log_error("Uplink port recevie linkup event.\n");
                }
            }
            else if ((SL_PORT_DOWN == entry.master_status) && (SL_PORT_UP == entry.slave_status))
            {
                if (netif_index == entry.master_index)
                {
                    entry.master_status = SL_PORT_UP;
                    if (entry.is_preempt)
                    {
                        entry.master_port = entry.master_index;
                        entry.slave_port = entry.slave_index;
                    }
                    else
                    {
                        entry.slave_port = netif_index;
                    }
                }
                else
                {
                    smart_link_log_error("Uplink port recevie linkup event.\n");
                }
            }
            else
            {
                smart_link_log_error("all port linkup.\n");
            }

            if (0 != smart_link_group_update(&entry))
            {
                smart_link_log_error("Execute sl-grp update faild in event func linkup.\n");
            }
        }
    }
    
    return 0;
}

int smart_link_app_event_linkdown(unsigned int netif_index, unsigned int son_netif_index, int length, char* data)
{
    struct smart_link_s entry;
    
    smart_link_log_event("%s, netif:%x, length:%d, data:%s\n", __func__, netif_index, length, data);

    memset(&entry, 0, sizeof(struct smart_link_s));

    while (0 == smart_link_group_get_next(&entry))
    {
        if ((netif_index == entry.master_index) || (netif_index == entry.slave_index))
        {
            if ((SL_PORT_UP == entry.master_status) && (SL_PORT_UP == entry.slave_status))
            {
                if (netif_index == entry.master_port)
                {
                    if (entry.master_port == entry.master_index)
                    {
                        entry.master_status = SL_PORT_DOWN;
                        entry.master_port = entry.slave_index;
                    }
                    else
                    {
                        entry.slave_status = SL_PORT_DOWN;
                        entry.master_port = entry.master_index;
                    }
                }
                else
                {
                    if (netif_index == entry.master_index)
                    {
                        entry.master_status = SL_PORT_DOWN;
                    }
                    else
                    {
                        entry.slave_status = SL_PORT_DOWN;
                    }
                }
            }
            else if ((SL_PORT_DOWN == entry.master_status) && (SL_PORT_UP == entry.slave_status))
            {
                if (netif_index == entry.slave_index)
                {
                    entry.master_port = 0;
                    entry.slave_status = SL_PORT_DOWN;
                }
                else
                {
                    smart_link_log_error("DownLink port recevie linkdown event.\n");
                }
            }
            else if ((SL_PORT_UP == entry.master_status) && (SL_PORT_DOWN == entry.slave_status))
            {
                if (netif_index == entry.master_index)
                {
                    entry.master_port = 0;
                    entry.master_status = SL_PORT_DOWN;
                }
                else
                {
                    smart_link_log_error("DownLink port recevie linkdown event.\n");
                }
            }
            else
            {
                smart_link_log_error("all port linkdown.\n");
            }

            entry.slave_port = 0;
            
            if (0 != smart_link_group_update(&entry))
            {
                smart_link_log_error("Execute sl-grp update faild in event func linkdown.\n");
            }            
        }
    }
    
    return 0;
}

int smart_link_app_event_delete(unsigned int netif_index, unsigned int son_netif_index, int length, char* data)
{
    unsigned int vlan_id = 0;
    struct smart_link_s entry;
    
    smart_link_log_event("%s, netif:%x, length:%d, data:%s\n", __func__, netif_index, length, data);

    if (NPD_NETIF_VLAN_TYPE != npd_netif_type_get(netif_index))
    {
        smart_link_log_event("XXX:just execute the delete event of vlan type.\n");
        return 0;
    }

    memset(&entry, 0, sizeof(struct smart_link_s));
    
    while (0 == smart_link_group_get_next(&entry))
    {
        vlan_id = npd_netif_vlan_get_vid(netif_index);
        if (NPD_VBMP_MEMBER(entry.data_vlan, vlan_id))
        {
            NPD_VBMP_VLAN_REMOVE(entry.data_vlan, vlan_id);
            if (0 != smart_link_group_update(&entry))
            {
                smart_link_log_error("Execute sl-grp update faild in event func delete.\n");
            }
        }
    }
    
    return 0;
}

int smart_link_app_event_l2delete(unsigned int netif_index, unsigned int son_netif_index, int length, char* data)
{
    struct smart_link_s entry;
    
    smart_link_log_event("%s, netif:%x, length:%d, data:%s\n", __func__, netif_index, length, data);
    if ((NPD_NETIF_ETH_TYPE != npd_netif_type_get(netif_index))
        && (NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(netif_index)))
    {
        smart_link_log_event("XXX:just execute the l2-delete event of trunk or eth-port.\n");
        return 0;
    }
    
    memset(&entry, 0, sizeof(struct smart_link_s));
    
    while (0 == smart_link_group_get_next(&entry))
    {
        if ((netif_index == entry.master_index) || (netif_index == entry.slave_index))
        {
            if (netif_index == entry.master_port)
            {
                if (entry.master_port == entry.master_index)
                {
                    entry.master_status = SL_PORT_DOWN;
                    entry.master_index = 0;
                    if ((SL_PORT_UP == entry.slave_status)
                        && (0 != entry.slave_index))
                    {
                        entry.master_port = entry.slave_index;
                    }
                }
                else
                {
                    entry.slave_status = SL_PORT_DOWN;
                    entry.slave_index = 0;
                    if ((SL_PORT_UP == entry.master_status)
                        && (0 != entry.master_index))
                    {
                        entry.master_port = entry.master_index;
                    }
                }
            }
            else
            {
                if (netif_index == entry.master_index)
                {
                    entry.master_status = SL_PORT_DOWN;
                    entry.master_index = 0;
                }
                else
                {
                    entry.slave_status = SL_PORT_DOWN;
                    entry.slave_index = 0;
                }
            }
            entry.slave_port = 0;

            if (0 != smart_link_group_update(&entry))
            {
                smart_link_log_error("Execute sl-grp update faild in event func l2-delete.\n");
            }
        }
    }

    return 0;
}

int smart_link_app_event_switchover(unsigned int netif_index, unsigned int son_netif_index, int length, char* data)
{
    if (NPD_GLOBAL_NETIF_INDEX != netif_index)
    {
        smart_link_log_event("%s, netif:%x, son_netif:%x, length:%d, data:%s\n", __func__, netif_index, son_netif_index, length, data);
        return -1;
    }
    
    smart_link_master_board_set(app_act_master_running());

    return 0;
}

int smart_link_app_relate_event_leave(unsigned int netif_index, unsigned int son_netif_index, int length, char* data)
{
    int vlan_id = 0;
    struct smart_link_s entry;

    smart_link_log_event("%s, netif:%x, son_netif:%x, length:%d, data:%s\n", __func__, netif_index, son_netif_index, length, data);

    if ((NPD_NETIF_VLAN_TYPE != npd_netif_type_get(netif_index))
        || ((NPD_NETIF_ETH_TYPE != npd_netif_type_get(son_netif_index))
            && (NPD_NETIF_TRUNK_TYPE != npd_netif_type_get(son_netif_index))))
    {
        smart_link_log_event("XXX:just execute the leave event of vlan remove trunk or eth-port.\n");
        return 0;
    }
    
    memset(&entry, 0, sizeof(struct smart_link_s));

    while (0 == smart_link_group_get_next(&entry))
    {
        if ((son_netif_index == entry.master_index) || (son_netif_index == entry.slave_index))
        {
            vlan_id = npd_netif_vlan_get_vid(netif_index);
            if (NPD_VBMP_MEMBER(entry.data_vlan, vlan_id))
            {
                if (son_netif_index == entry.master_port)
                {
                    if ((SL_PORT_UP == entry.slave_status)
                            && (0 != entry.slave_index))
                    {
                        entry.slave_port = entry.master_port;
                        if (entry.master_port == entry.master_index)
                        {
                            entry.master_port = entry.slave_index;
                        }
                        else
                        {
                            entry.master_port = entry.master_index;
                        }
                        
                        if (0 != smart_link_group_update(&entry))
                        {
                            smart_link_log_error("Execute sl-grp update faild in event func leave.\n");
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void smart_link_app_event_handler
(
    unsigned int netif_index,
    int event,
    char* buff,
    int len
)
{
    int ret = 0;
    struct smart_link_event_s* cmd = NULL;

    cmd = smart_link_app_event_find(event);
    if ((NULL != cmd) && (NULL != cmd->func))
    {
        ret = cmd->func(netif_index, 0, len, buff);
        if (0 != ret)
        {
            smart_link_log_event("App event (%d) execute callback function failed, return-code (%d).\n", event, ret);
        }
    }
    else
    {
        smart_link_log_event("App event (%d) handler not find.\n", event);
    }

    return ;
}

void smart_link_app_relate_event_handler
(
    unsigned int netif_index, 
    unsigned int son_netif_index, 
    int event, 
    char* private, 
    int len
)
{
    int ret = 0;
    struct smart_link_event_s* cmd = NULL;

    cmd = smart_link_app_relate_event_find(event);
    if ((NULL != cmd) && (NULL != cmd->func))
    {
        ret = cmd->func(netif_index, son_netif_index, len, private);
        if (0 != ret)
        {
            smart_link_log_event("App releate event (%d) execute callback function failed, return-code (%d).\n", event, ret);
        }
    }
    else
    {
        smart_link_log_event("App releate event (%d) handler not find.\n", event);
    }

    return ;
}

int smart_link_recv_event(struct thread * t)
{
    int ret = 0;
    int sock = -1;

    sock = THREAD_FD (t);
    smart_link_event(SL_EVENT, sock);

    ret = netif_app_event_handle(sock, smart_link_app_event_handler, smart_link_app_relate_event_handler);
    if (0 != ret)
    {
        smart_link_log_error("Execute netif app event handle functoin failed.\n");
    }

    return ret;
}

int smart_link_app_event_init()
{
    int sock = -1;
    int rc = 1;
    
    while (0 != rc)
    {
        rc = dbtable_sync_init(SMART_LINK_DBSYNC_SERVICE, smart_link_sync_log_event, NULL);
        usleep(100);
    }

    sock = netif_app_event_init(SMART_LINK_TIPC_APP_INSTANCE);
    if (sock < 0)
    {
        return -1;
    }

    smart_link_event(SL_EVENT, sock);

    memset(sl_event_cmd, 0, sizeof(sl_event_cmd));
    smart_link_app_event_install(PORT_NOTIFIER_LINKUP_E, smart_link_app_event_linkup);
    smart_link_app_event_install(PORT_NOTIFIER_LINKDOWN_E, smart_link_app_event_linkdown);
    smart_link_app_event_install(PORT_NOTIFIER_DELETE, smart_link_app_event_delete);
    smart_link_app_event_install(PORT_NOTIFIER_L2DELETE, smart_link_app_event_l2delete);
    smart_link_app_event_install(NOTIFIER_SWITCHOVER, smart_link_app_event_switchover);

    memset(sl_relate_event_cmd, 0, sizeof(sl_relate_event_cmd));
    smart_link_app_relate_event_install(PORT_NOTIFIER_LEAVE, smart_link_app_relate_event_leave);

    return 0;
}

#endif

