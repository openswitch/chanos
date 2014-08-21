/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_VRRP
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"
#include "chasm_manage_proto.h"
#include "npd_tracking.h"
extern int npd_netif_get_status(unsigned int netif_index);

db_table_t   *npd_tracking_group_db_table = NULL;
array_table_index_t *npd_tracking_group_array_index = NULL;

int npd_tracking_get(int track_id, struct track_group* query)
{    
    if (NULL == query)
    {
        return -1;
    }
       
    if (0 != dbtable_array_get(npd_tracking_group_array_index, track_id, query))
    {
        return -1;
    }

    return 0;
}

int npd_tracking_set(int track_id, struct track_group* user)
{
    struct track_group temp;

    if (NULL == user)
    {
        return -1;
    }

    if (0 != dbtable_array_update(npd_tracking_group_array_index, track_id, &temp, user))
    {
        return -1;
    }
    
    return 0;
}

int npd_tracking_add(int track_id, struct track_group* user)
{
    if (NULL == user)
    {
        return -1;
    }
    
    if (0 != dbtable_array_insert_byid(npd_tracking_group_array_index, track_id, user))
    {
        return -1;
    }

    return 0;
}

int npd_tracking_delete(int track_id, struct track_group* user)
{
    if (NULL == user)
    {
        return -1;
    }

    if (0 != dbtable_array_delete(npd_tracking_group_array_index, track_id, user))
    {
        return -1;
    }

    return 0;
}

int npd_tracking_get_next(int track_id, struct track_group* query)
{
    int ret = 0;
    int group_id = track_id;
    
    do 
    {
        group_id += 1;
        ret = npd_tracking_get(group_id, query);
    } while (0 != ret && group_id <= NPD_TRACKING_GROUP_SIZE);

    return ret;
}

void npd_tracking_group_action_down_handler(unsigned int netif_index, unsigned int value)
{
    int trunk_id = 0;
    int array_index = 0;
    unsigned int eth_g_index = 0;
    struct trunk_s node;
    
    if (NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
    {
        eth_port_sw_attr_update(netif_index, ADMIN, value);
    }
    else if (NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
    {
        memset(&node, 0, sizeof(node));
        trunk_id = npd_netif_trunk_get_tid(netif_index);
        if (0 == npd_find_trunk(trunk_id, &node))
        {
            NPD_PBMP_ITER(node.ports, array_index)
            {
                eth_g_index = eth_port_array_index_to_ifindex(array_index);
                eth_port_sw_attr_update(eth_g_index, ADMIN, value);
            }
        }
    }
}

int npd_tracking_calculate_event(struct track_group* entry)
{
    if (0 == entry->tracking_count)
    {
        entry->tracking_event = PORT_LINK_UP;
        entry->dl_happened = 0;
    }
    else if (entry->tracking_up_count)
    {
        if (tracking_mode_any == entry->tracking_mode)
        {
            entry->tracking_event = PORT_LINK_UP;
            entry->dl_happened = 0;
        }
        else if (tracking_mode_all == entry->tracking_mode
            && entry->tracking_up_count == entry->tracking_count)
        {
            entry->tracking_event = PORT_LINK_UP;
            entry->dl_happened = 0;
        }
        else
        {
            entry->tracking_event = PORT_LINK_DOWN;
            entry->dl_happened = 1;
        }
    }
    else
    {
        entry->tracking_event = PORT_LINK_DOWN;
        entry->dl_happened = 1;
    }
    
    return 0;
}

int npd_track_object_event(struct track_group* entry, unsigned int netif_index, enum PORT_NOTIFIER_ENT event)
{
    int ni = 0;
    int flag_up = 0;

    if (0 == netif_index)
    {
        return 0;
    }

    if (PORT_NOTIFIER_LINKUP_E == event
        || PORT_NOTIFIER_L3LINKUP == event
        || NOTIFIER_SLOT_INSERT == event)
    {
        flag_up = 1;
    }
    else
    {
        flag_up = 0;
    }

    for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
    {
        if (netif_index == entry->tracking_object[ni].netif_index)
        {
            if (entry->tracking_object[ni].status == flag_up)
            {
                return 0;
            }
            else
            {
                entry->tracking_object[ni].status = flag_up;
                entry->tracking_up_count += (flag_up ? 1 : -1);
                return 1;
            }
        }
    }

    return 0;  
}

int npd_track_object_event_delete(struct track_group* entry, unsigned int netif_index)
{
    int ni = 0;

    if (0 == netif_index)
    {
        return 0;
    }

    if (netif_index == entry->dl_index)
    {
        entry->dl_index = 0;
        entry->dl_happened = 0;
        return 1;
    }

    for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
    {
        if (netif_index == entry->tracking_object[ni].netif_index)
        {
            entry->tracking_up_count -= (entry->tracking_object[ni].status ? 1 : 0);
            entry->tracking_object[ni].netif_index = 0;
            entry->tracking_object[ni].status = 0;
            entry->tracking_count--;

            return 1;
        }
    }

    return 0;  
}

void npd_tracking_notify_event(unsigned int netif_index, enum PORT_NOTIFIER_ENT event, char *data, int datalen)
{
    int ni = 0;
    int ret = 0;
    struct track_group query;
    
    npd_syslog_dbg("Info npd Tracking notify event\n");


    if (event == NOTIFIER_SLOT_INSERT
        || event == NOTIFIER_SLOT_REMOVE
        || event == NOTIFIER_SLOT_DELETE)
    {
        netif_index = CHASSIS_SLOT_INDEX2NO(netif_index);
    }
    
    for (ni = 0; ni <= NPD_TRACKING_GROUP_SIZE; ni++)
    {
        if (0 != npd_tracking_get(ni, &query))
        {
            continue;
        }

        if (0 == query.tracking_count)
        {
            continue;
        }
        
        switch(event)
        {
            case PORT_NOTIFIER_LINKUP_E:
            case PORT_NOTIFIER_LINKDOWN_E:
            case PORT_NOTIFIER_REMOVE:
            case PORT_NOTIFIER_L3LINKUP:
            case PORT_NOTIFIER_L3LINKDOWN:
            case NOTIFIER_SLOT_INSERT:
            case NOTIFIER_SLOT_REMOVE:
            {
                ret = npd_track_object_event(&query, netif_index, event);
                break;
            }
            case PORT_NOTIFIER_L2DELETE:
            case PORT_NOTIFIER_L3DELETE:
            case NOTIFIER_SLOT_DELETE:
            {
                ret = npd_track_object_event_delete(&query, netif_index);
                break;
            }
            default:
            {
                break;
            }
        }

        if (0 != ret)
        {
            npd_tracking_calculate_event(&query);

            if (0 != npd_tracking_set(query.tracking_group_id, &query))
            {
                return ;
            }
        }
    }

    return;
}

void npd_tracking_notify_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char* data,
    int datalen
)
{
    npd_syslog_dbg("Info npd Tracking notify relate event\n");
    
    return;
}

netif_event_notifier_t npd_tracking_port_notifier =
{
    .netif_event_handle_f = &npd_tracking_notify_event,
    .netif_relate_handle_f = &npd_tracking_notify_relate_event
};

long npd_tracking_handle_update(void* new_entry, void* old_entry)
{
    struct track_group*  new_query = (struct track_group *)new_entry;
    struct track_group*  old_query = (struct track_group *)old_entry;
    int tracking_event = 0; 

    if ((0 != new_query->dl_index)
        && (0 == old_query->dl_index))
    {
        if (new_query->dl_happened)
        {
            npd_tracking_group_action_down_handler(new_query->dl_index, 0x0);
        }
    }
    else if ((0 == new_query->dl_index)
        && (0 != old_query->dl_index))
    {
        if (old_query->dl_happened)
        {
            npd_tracking_group_action_down_handler(old_query->dl_index, 0x1);
        }
    }
    else if ((0 != new_query->dl_index)
        && (0 != old_query->dl_index))
    {
        if (new_query->dl_index == old_query->dl_index)
        {
            if (new_query->dl_happened != old_query->dl_happened)
            {
                if (1 == new_query->dl_happened)    /* down */
                {
                    npd_tracking_group_action_down_handler(new_query->dl_index, 0x0);
                }
                else    /* up */
                {
                    npd_tracking_group_action_down_handler(new_query->dl_index, 0x1);
                }
            }
        }
        else if (new_query->dl_index != old_query->dl_index)
        {
            if (new_query->dl_happened == old_query->dl_happened)
            {
                if (1 == new_query->dl_happened)
                {
                    npd_tracking_group_action_down_handler(old_query->dl_index, 0x1);
                    npd_tracking_group_action_down_handler(new_query->dl_index, 0x0);
                }
            }
            else
            {
                if (old_query->dl_happened)
                {
                    npd_tracking_group_action_down_handler(old_query->dl_index, 0x1);
                }

                if (new_query->dl_happened)
                {
                    npd_tracking_group_action_down_handler(new_query->dl_index, 0x0);
                }
            }
        }
    }

    if (new_query->tracking_count != old_query->tracking_count
        || new_query->tracking_up_count != old_query->tracking_up_count
        || new_query->tracking_mode != old_query->tracking_mode)
    {
        if (new_query->tracking_event != old_query->tracking_event)
        {
            tracking_event = (new_query->tracking_event == PORT_LINK_UP)?PORT_NOTIFIER_TRACK_UP:PORT_NOTIFIER_TRACK_DOWN;
            netif_app_notify_event(new_query->tracking_group_id, tracking_event, NULL, 0);
        }
    }

    return 0;
}

long npd_tracking_handle_insert(void* entry)
{
    struct track_group*  query = (struct track_group *)entry;
    int tracking_event; 

    if (0 != query->tracking_count)
    {
        if (query->dl_happened && query->dl_index)
        {
            npd_tracking_group_action_down_handler(query->dl_index, 0x0);
        }

        tracking_event = (query->tracking_event == PORT_LINK_UP)?PORT_NOTIFIER_TRACK_UP:PORT_NOTIFIER_TRACK_DOWN;
        netif_app_notify_event(query->tracking_group_id, tracking_event, NULL, 0);
    }
        
    return 0;
}

long npd_tracking_handle_delete(void* entry)
{
    struct track_group*  query = (struct track_group *)entry;

    if (query->dl_happened && query->dl_index)
    {
        npd_tracking_group_action_down_handler(query->dl_index, 0x1);
    }

    netif_app_notify_event(query->tracking_group_id, PORT_NOTIFIER_TRACK_DELETE, NULL, 0);
    
    return 0;
}

int npd_tracking_db_table_init()
{
    int ret;
    
    ret = create_dbtable("npd_tracking_group_db_table",
                            NPD_TRACKING_GROUP_SIZE + 1, 
                            sizeof(struct track_group),
                            npd_tracking_handle_update, 
                            NULL,
                            npd_tracking_handle_insert, 
                            npd_tracking_handle_delete, 
                            NULL,
                            NULL,
                            NULL, 
                            NULL,
                            NULL,
                            DB_SYNC_ALL,
                            &(npd_tracking_group_db_table));
    if (0 != ret)
    {
        npd_syslog_err("create npd tracking group dbtable fail\n");
        return NPD_FAIL;
    }


    ret = dbtable_create_array_index("npd_tracking_group_array_index", 
                            npd_tracking_group_db_table,  
                            &npd_tracking_group_array_index);


    if( 0  != ret )
    {
        npd_syslog_err("create npd tracking group detable index fail\n");
        return NPD_FAIL;
    }    
    
    return 0;
}

void npd_tracking_init(void) 
{
    int ret = 0;
    
    ret = npd_tracking_db_table_init();
    if (0 != ret) 
    {
        npd_syslog_dbg("Init npd Tracking db table fail\n");
        return;
    }

    register_netif_notifier(&npd_tracking_port_notifier);

    return;    
}

int npd_tracking_get_netif_status(unsigned int netif_index, unsigned int netif_index_layer, unsigned int* status)
{
    int ret = 0;
    int temp_status = 0;
    unsigned int slot_index = 0;

    if (netif_index_slot == netif_index_layer)
    {
        ret = chasm_local_check(netif_index);
        if (BOARD_RETURN_CODE_ERR_NONE == ret)
        {
            slot_index = SYS_CHASSIS_SLOT_NO2INDEX(netif_index);

            if (slot_index != localmoduleinfo->slot_index)
            {
                temp_status = MODULE_STATUS_ON_SLOT_INDEX(slot_index);
            }
            else
            {
                temp_status = localmoduleinfo->rmtstate[localmoduleinfo->slot_index];
            }

            *status = (RMT_BOARD_RUNNING == temp_status) ? 1 : 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        temp_status = npd_netif_get_status(netif_index);
            
        if (0 == temp_status || 1 == temp_status)
        {
            *status = temp_status;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

int npd_tracking_group(unsigned int group_id)
{
    struct track_group query;
    
    if (0 != npd_tracking_get(group_id, &query))
    {
        memset(&query, 0, sizeof(struct track_group));
        query.tracking_group_id = group_id;
        query.tracking_event = PORT_LINK_UP;
        if (0 != npd_tracking_add(group_id, &query))
        {
            return NPD_RETURN_CODE_TRACKING_ERROR;
        }
    }

    return NPD_RETURN_CODE_TRACKING_SUCCESS;
}

int npd_tracking_group_delete(unsigned int group_id)
{
    struct track_group query;
    
    if (0 == npd_tracking_get(group_id, &query))
    {
        if (0 != npd_tracking_delete(group_id, &query))
        {
            return NPD_RETURN_CODE_TRACKING_ERROR;
        }
    }
    else
    {
        return NPD_RETURN_CODE_TRACKING_NOT_EXIST;
    }

    return NPD_RETURN_CODE_TRACKING_SUCCESS;
}

int npd_tracking_group_add_object(unsigned int group_id, unsigned int netif_index, unsigned int netif_index_layer)
{
    int ni = 0;
    struct track_group query;
    
    if (0 == npd_tracking_get(group_id, &query))
    {
        if (query.tracking_count > NPD_TRACKING_MAX_OBJECT)
        {
            return NPD_RETURN_CODE_TRACKING_OBJECT_MAX_OVER;
        }
        
        for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
        {
            if (netif_index == query.tracking_object[ni].netif_index  
                && netif_index_layer == query.tracking_object[ni].netif_index_layer)
            {
                return NPD_RETURN_CODE_TRACKING_OBJECT_EXIST;
            }
        }

        if (netif_index == query.dl_index)
        {
            return NPD_RETURN_CODE_TRACKING_OBJECT_EXIST;
        }

        for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
        {
            if (0 == query.tracking_object[ni].netif_index)
            {
                if (0 != npd_tracking_get_netif_status(netif_index, netif_index_layer, &query.tracking_object[ni].status))
                {
                    return NPD_RETURN_CODE_TRACKING_GET_NETIF_STATUS_ERROR;
                }

                query.tracking_up_count += (query.tracking_object[ni].status ? 1 : 0);
                query.tracking_object[ni].netif_index = netif_index;
                query.tracking_object[ni].netif_index_layer = netif_index_layer;
                query.tracking_count++;
                break;
            }
        }
    }
    else
    {
        return NPD_RETURN_CODE_TRACKING_NOT_EXIST;
    }

    npd_tracking_calculate_event(&query);

    if (0 != npd_tracking_set(group_id, &query))
    {
        return NPD_RETURN_CODE_TRACKING_ERROR;
    }

    return NPD_RETURN_CODE_TRACKING_SUCCESS;
}

int npd_tracking_group_remove_object(unsigned int group_id, unsigned int netif_index, unsigned int netif_index_layer)
{
    int ni = 0;
    struct track_group query;
    
    if (0 == npd_tracking_get(group_id, &query))
    {
        if (query.tracking_count <= 0)
        {
            return NPD_RETURN_CODE_TRACKING_OBJECT_NULL;
        }
        
        for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
        {
            if (netif_index == query.tracking_object[ni].netif_index
                && netif_index_layer == query.tracking_object[ni].netif_index_layer)
            {
                query.tracking_up_count -= (query.tracking_object[ni].status ? 1 : 0);
                query.tracking_object[ni].netif_index = 0;
                query.tracking_object[ni].netif_index_layer = 0;
                query.tracking_object[ni].status = 0;
                query.tracking_count--;
                break;
            }
        }

        if (NPD_TRACKING_MAX_OBJECT == ni)
        {
            return NPD_RETURN_CODE_TRACKING_OBJECT_NOT_EXIST;
        }
    }
    else
    {
        return NPD_RETURN_CODE_TRACKING_NOT_EXIST;
    }

    npd_tracking_calculate_event(&query);

    if (0 != npd_tracking_set(group_id, &query))
    {
        return NPD_RETURN_CODE_TRACKING_ERROR;
    }

    return NPD_RETURN_CODE_TRACKING_SUCCESS;
}

int npd_tracking_group_action_down(unsigned int group_id, unsigned int netif_index)
{
    int ni = 0;
    struct track_group query;
    
    if (0 == npd_tracking_get(group_id, &query))
    {
        if (0 != netif_index)
        {
            for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
            {
                if (netif_index == query.tracking_object[ni].netif_index)
                {
                    return NPD_RETURN_CODE_TRACKING_OBJECT_EXIST;
                }
            }
        }
        query.dl_index = netif_index;
        query.dl_happened = 0;
    }
    else
    {
        return NPD_RETURN_CODE_TRACKING_NOT_EXIST;
    }

    npd_tracking_calculate_event(&query);

    if (0 != npd_tracking_set(group_id, &query))
    {
        return NPD_RETURN_CODE_TRACKING_ERROR;
    }

    return NPD_RETURN_CODE_TRACKING_SUCCESS;
}

int npd_tracking_group_mode(unsigned int group_id, unsigned int tracking_mode)
{
    struct track_group query;
    
    if (0 == npd_tracking_get(group_id, &query))
    {
        query.tracking_mode = tracking_mode;
    }
    else
    {
        return NPD_RETURN_CODE_TRACKING_NOT_EXIST;
    }

    npd_tracking_calculate_event(&query);

    if (0 != npd_tracking_set(group_id, &query))
    {
        return NPD_RETURN_CODE_TRACKING_ERROR;
    }

    return NPD_RETURN_CODE_TRACKING_SUCCESS;
}

int npd_tracking_group_status(unsigned int group_id, unsigned int* group_status)
{
    struct track_group query;
    
    if (0 == npd_tracking_get(group_id, &query))
    {
        *group_status = query.tracking_event;
    }
    else
    {
        return NPD_RETURN_CODE_TRACKING_NOT_EXIST;
    }

    return NPD_RETURN_CODE_TRACKING_SUCCESS;
}

int npd_tracking_group_show_running(char** show_str)
{
    int len = 0;
    int netif_id = 0;
    char*   cur_str = *show_str;
    char    track_object_name[32];
    struct track_group query;

    memset(&query, 0, sizeof(query));

    while (0 == npd_tracking_get_next(query.tracking_group_id, &query))
    {
        len = sprintf(cur_str, "track-group %d\n", query.tracking_group_id);
        cur_str += len;

        len = sprintf(cur_str, " track-group mode %s\n", (query.tracking_mode == tracking_mode_any) ? "any" : "all");
        cur_str += len;

        for (netif_id = 0; netif_id < NPD_TRACKING_MAX_OBJECT; netif_id++)
        {
            if (0 != query.tracking_object[netif_id].netif_index)
            {
                memset(track_object_name, 0, sizeof(track_object_name));
                
                if (netif_index_l2 == query.tracking_object[netif_id].netif_index_layer)
                {
                    npd_netif_index_to_user_fullname(query.tracking_object[netif_id].netif_index, track_object_name);
                    len = sprintf(cur_str, " track-object interface %s\n", track_object_name);
                }
                else if (netif_index_l3 == query.tracking_object[netif_id].netif_index_layer)
                {
                    npd_netif_index_to_l3intf_name(query.tracking_object[netif_id].netif_index, track_object_name);
                    len = sprintf(cur_str, " track-object interface %s\n", track_object_name);
                }
                else if (netif_index_slot == query.tracking_object[netif_id].netif_index_layer)
                {
                    len = sprintf(cur_str, " track-object slot %d\n", query.tracking_object[netif_id].netif_index);
                }
                else
                {
                    continue;
                }

                cur_str += len;
            }
        }

		if(0 != query.dl_index)
		{
			memset(track_object_name, 0, sizeof(track_object_name));
        	npd_netif_index_to_user_fullname(query.dl_index, track_object_name);
        	len = sprintf(cur_str, "track-action interface %s down\n", track_object_name);
	        cur_str += len;
		}
		
        len = sprintf(cur_str, " exit\n");
        cur_str += len;
    }

    return 0;
}

DBusMessage* npd_dbus_tracking_group(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    group_id = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_tracking_group(group_id);
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_group_delete(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    group_id = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_tracking_group_delete(group_id);
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_group_add_object(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    group_id = 0;
    unsigned int    netif_index = 0;
    unsigned int    netif_index_layer = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_UINT32, &netif_index,
                    DBUS_TYPE_UINT32, &netif_index_layer,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_tracking_group_add_object(group_id, netif_index, netif_index_layer);
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_group_remove_object(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    group_id = 0;
    unsigned int    netif_index = 0;
    unsigned int    netif_index_layer = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_UINT32, &netif_index,
                    DBUS_TYPE_UINT32, &netif_index_layer,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_tracking_group_remove_object(group_id, netif_index, netif_index_layer);
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_group_action_down(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter;
    DBusError       err;
    unsigned int    group_id = 0;
    unsigned int    netif_index = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_UINT32, &netif_index,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_tracking_group_action_down(group_id, netif_index);
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_group_mode(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    group_id = 0;
    unsigned int    tracking_mode = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_UINT32, &tracking_mode,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_tracking_group_mode(group_id, tracking_mode);
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_object_l3exist(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    netif_index = 0;
    unsigned int    ret = 0;
    unsigned int    ifindex = ~0UI;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &netif_index,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    if (NPD_TRUE == npd_intf_exist_check(netif_index, &ifindex))
    {
        ret = NPD_RETURN_CODE_TRACKING_SUCCESS;
    }
    else
    {
        ret = NPD_RETURN_CODE_TRACKING_INTERFACE_NOT_EXIST;
    }
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_object_l2exist(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    netif_index = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &netif_index,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_switch_port_exist_check(netif_index);

    if (0 == ret)
    {
        ret = NPD_RETURN_CODE_TRACKING_SUCCESS;
    }
    else
    {
        ret = NPD_RETURN_CODE_TRACKING_INTERFACE_NOT_EXIST;;
    }
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);

    return reply;
}

DBusMessage* npd_dbus_tracking_group_get(DBusMessage* msg, int (*processor)(int , struct track_group* ))
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    int             ni = 0;
    unsigned int    group_id = 0;
    unsigned int    ret = 0;
    unsigned int    netif_index[NPD_TRACKING_MAX_OBJECT];
    unsigned int    netif_index_layer[NPD_TRACKING_MAX_OBJECT];
    unsigned int    status[NPD_TRACKING_MAX_OBJECT];
    unsigned int*   ptr_netif_index = netif_index;
    unsigned int*   ptr_netif_index_layer = netif_index_layer;
    unsigned int*   ptr_status = status;
    struct track_group query;

    memset(&query, 0, sizeof(struct track_group));

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    if (0 != processor(group_id, &query))
    {
        ret = NPD_RETURN_CODE_TRACKING_NOT_EXIST;
    }
    else
    {
        ret = NPD_RETURN_CODE_TRACKING_SUCCESS;
    }

    for (ni = 0; ni < NPD_TRACKING_MAX_OBJECT; ni++)
    {
        netif_index[ni] = query.tracking_object[ni].netif_index;
        netif_index_layer[ni] = query.tracking_object[ni].netif_index_layer;
        status[ni] = query.tracking_object[ni].status;
    }
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }

    dbus_message_append_args(reply,
                        DBUS_TYPE_UINT32,
                        &ret,

                        DBUS_TYPE_UINT32,
                        &query.tracking_group_id,

                        DBUS_TYPE_UINT32,
                        &query.tracking_mode,

                        DBUS_TYPE_UINT32,
                        &query.tracking_count,

                        DBUS_TYPE_UINT32,
                        &query.tracking_up_count,

                        DBUS_TYPE_UINT32,
                        &query.tracking_event,

                        DBUS_TYPE_UINT32,
                        &query.dl_index,

                        DBUS_TYPE_ARRAY,
                        DBUS_TYPE_UINT32,
                        &ptr_netif_index,
                        NPD_TRACKING_MAX_OBJECT,

                        DBUS_TYPE_ARRAY,
                        DBUS_TYPE_UINT32,
                        &ptr_netif_index_layer,
                        NPD_TRACKING_MAX_OBJECT,

                        DBUS_TYPE_ARRAY,
                        DBUS_TYPE_UINT32,
                        &ptr_status,
                        NPD_TRACKING_MAX_OBJECT,

                        DBUS_TYPE_INVALID);

    return reply;
}

DBusMessage* npd_dbus_tracking_group_query(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_tracking_group_get(msg, npd_tracking_get);
}

DBusMessage* npd_dbus_tracking_group_query_next(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_tracking_group_get(msg, npd_tracking_get_next);
}

DBusMessage* npd_dbus_tracking_group_show_running(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    char*  show_str = NULL;

    show_str = (char*)malloc(NPD_TRACKING_GROUP_SHOW_RUNNING_BUFFER_LEN);
    if (NULL == show_str)
    {
        npd_syslog_err("Track group memory malloc error\n");
        return NULL;
    }
    memset(show_str, 0, NPD_TRACKING_GROUP_SHOW_RUNNING_BUFFER_LEN);

    if (0 != npd_tracking_group_show_running(&show_str))
    {
        memset(&show_str, 0, NPD_TRACKING_GROUP_SHOW_RUNNING_BUFFER_LEN);
    }
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        free(show_str);
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                    DBUS_TYPE_STRING, &show_str);    

    free(show_str);

    return reply;
}


DBusMessage* npd_dbus_tracking_group_status(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter = {0};
    DBusError       err;
    unsigned int    group_id = 0;
    unsigned int    group_status = 0;
    unsigned int    ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &group_id,
                    DBUS_TYPE_INVALID)))
    {
        npd_syslog_dbg("tracking unable to get input args\n");
        if (dbus_error_is_set(&err))
        {
            npd_syslog_err("tracking %s raised:%s\n", err.name, err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    ret = npd_tracking_group_status(group_id, &group_status);
    
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("tracking dbus set error!\n");
        return reply;
    }
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                    DBUS_TYPE_UINT32, &ret);
    dbus_message_iter_append_basic(&iter,
                                    DBUS_TYPE_UINT32, &group_status);    

    return reply;
}
#endif


