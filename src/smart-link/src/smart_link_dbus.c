
#ifdef HAVE_SMART_LINK

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <dbus/dbus.h>
#include "dbus/smartlink/smart_link_dbus_def.h"

#include "sysdef/returncode.h"
#include "lib/db_app_sync.h"
#include "lib/netif_index.h"
#include "lib/chassis_man_app.h"
#include "lib/npd_bitop.h"
#include "lib/npd_database.h"
#include "quagga/thread.h"

#include "smart_link.h"
#include "smart_link_dbus.h"
#include "smart_link_event.h"
#include "smart_link_packet.h"
#include "smart_link_log.h"

int is_active_master_board = 0;

db_table_t* smart_link_group_db = NULL;
array_table_index_t* smart_link_array_index = NULL;

void smart_link_master_board_set(int is_active_master)
{
    is_active_master_board = is_active_master;
    return ;
}

int smart_link_group_search(struct smart_link_s* entry)
{
    return (0 == dbtable_array_get(smart_link_array_index, entry->id, entry)) ? 0 : -1;
}

int smart_link_group_insert(struct smart_link_s* entry)
{
    return (0 == dbtable_array_insert_byid(smart_link_array_index, entry->id, entry)) ? 0 : -1;
}

int smart_link_group_update(struct smart_link_s* entry)
{
    return (0 == dbtable_array_update(smart_link_array_index, entry->id, NULL, entry)) ? 0 : -1;
}

int smart_link_group_delete(struct smart_link_s* entry)
{
    return (0 == dbtable_array_delete(smart_link_array_index, entry->id, entry)) ? 0 : -1;
}

int smart_link_group_get_next(struct smart_link_s* entry)
{
    do
    {
        entry->id++;
    } while ((entry->id <= SMART_LINK_GROUP_SIZE)
        && (0 != smart_link_group_search(entry)));

    if (entry->id > SMART_LINK_GROUP_SIZE)
    {
        return -1;
    }

    return 0;
}

void smart_link_send_msg_to_npd
(
    unsigned int block_netif_index,
    unsigned int unblock_netif_index,
    unsigned int instance,
    unsigned int message_back_flag
)
{
    struct smart_link_msg_s data;
    
    memset(&data, 0, sizeof(data));
    
    data.cookie = SL_MSG_COOKIE_ID;
    data.block_port = block_netif_index;
    data.unblock_port = unblock_netif_index;
    data.instance = instance;
    data.back_id = message_back_flag;
    data.cookie_end = SL_MSG_COOKIE_ID_END;

    smart_link_send_msg((char*)&data, sizeof(data));
    smart_link_log_packet("Send msg to npd.\n");
    
    return ;
}

long smart_link_handle_update(void* new_entry, void* old_entry)
{
    struct smart_link_s* new = (struct smart_link_s*)new_entry;
    struct smart_link_s* old = (struct smart_link_s*)old_entry;

    if ((NULL == new) || (NULL == old))
    {
        return -1;
    }
    
    if (is_active_master_board)
    {
        if (new->is_enable != old->is_enable)
        {
            if (new->is_enable)
            {
                smart_link_send_msg_to_npd(new->slave_port, \
                        new->master_port, new->instance, new->id);
            }
            else
            {
                smart_link_send_msg_to_npd(old->slave_index, \
                        old->master_index, old->instance, ~0UL);
            }
        }
        else if (new->is_enable)
        {
            if (new->master_port != old->master_port)
            {
                smart_link_send_msg_to_npd(new->slave_port, \
                        new->master_port, new->instance, new->id);
            }
            else if (new->slave_port != old->slave_port)
            {
                if (new->slave_index)
                {
                    smart_link_send_msg_to_npd(new->slave_port, \
                        0, new->instance, 0);
                }
            }
        }
    }
    return 0;
}

int smart_link_db_init()
{
    int ret;

    ret = create_dbtable("__Smart_Link_db",
                        SMART_LINK_GROUP_SIZE + 1, 
                        sizeof(struct smart_link_s),
                        smart_link_handle_update, 
                        NULL,
                        NULL, 
                        NULL, 
                        NULL,
                        NULL,
                        NULL, 
                        NULL,
                        NULL,
                        DB_SYNC_ALL,
                        &(smart_link_group_db));
    if (0 != ret)
    {
        smart_link_log_error("Create Smart-Link group dbtable failed.\n");
        return -1;
    }

    ret = dbtable_create_array_index("__Smart_Link_array_idx", 
                            smart_link_group_db,
                            &smart_link_array_index);


    if (0 != ret)
    {
        smart_link_log_error("Create Smart-Link array index failed.\n");
        return -1;
    }	

    return 0;
}

unsigned int smart_link_group(unsigned int id)
{
    struct smart_link_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;

    if (0 == smart_link_group_search(&entry))
    {
        smart_link_log_debug("Smart-link group has exist.\n");
        return SMART_LINK_RETURN_CODE_SUCCESS;
    }
    else
    {
        smart_link_log_debug("Smart-link group create new one.\n");
        if (0 == smart_link_group_insert(&entry))
        {
            return SMART_LINK_RETURN_CODE_SUCCESS;
        }
        else
        {
            smart_link_log_error("Entry insert failed.\n");
            return SMART_LINK_RETURN_CODE_ERROR;
        }
    }

    return SMART_LINK_RETURN_CODE_ERROR;
}

unsigned int smart_link_no_group(unsigned int id)
{
    struct smart_link_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;

    if (0 == smart_link_group_search(&entry))
    {
        if (0 != entry.is_enable)
        {
            smart_link_log_debug("SL grp has enable.\n");
            return SMART_LINK_RETURN_CODE_GROUP_ENABLE;
        }
        
        smart_link_log_debug("Smart-link group will be destroy.\n");
        if (0 == smart_link_group_delete(&entry))
        {
            return SMART_LINK_RETURN_CODE_SUCCESS;
        }
        else
        {
            smart_link_log_error("Entry delete failed.\n");
            return SMART_LINK_RETURN_CODE_ERROR;
        }
    }

    smart_link_log_debug("Smart-link group has not exist.\n");

    return SMART_LINK_RETURN_CODE_ERROR;
}

unsigned int smart_link_enable(unsigned int id, unsigned int is_enable)
{
    struct smart_link_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    
    if (0 == smart_link_group_search(&entry))
    {
        if (((0 == entry.master_index)
            && (0 == entry.slave_index))
            || (0 == entry.instance))
        {
            smart_link_log_debug("SL grp incompleate.\n");
            return SMART_LINK_RETURN_CODE_GROUP_INCOMPLEATE;
        }

        if (entry.is_enable != is_enable)
        {
            entry.is_enable = is_enable;
            if (is_enable)
            {
                if (SL_PORT_UP == entry.master_status)
                {
                    entry.master_port = entry.master_index;
                    if (SL_PORT_UP == entry.slave_status)
                    {
                        entry.slave_port = entry.slave_index;
                    }
                }
                else if (SL_PORT_UP == entry.slave_status)
                {
                    entry.master_port = entry.slave_index;
                    entry.slave_port = 0;
                }
            }
        
            if (0 == smart_link_group_update(&entry))
            {
                return SMART_LINK_RETURN_CODE_SUCCESS;
            }
            else
            {
                smart_link_log_error("Entry update failed.\n");
                return SMART_LINK_RETURN_CODE_ERROR;
            }
        }
        else
        {
            return SMART_LINK_RETURN_CODE_SUCCESS;
        }
    }

    smart_link_log_debug("Smart-link group has not exist.\n");

    return SMART_LINK_RETURN_CODE_ERROR;
}

unsigned int smart_link_preempt(unsigned int id, unsigned int is_enable)
{
    struct smart_link_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    
    if (0 == smart_link_group_search(&entry))
    {
        entry.is_preempt = is_enable;
        if (is_enable)
        {
            if ((0 != entry.master_index)
                && (0 != entry.slave_index)
                && (0 != entry.instance))
            {
                if (SL_PORT_UP == entry.master_status)
                {
                    entry.master_port = entry.master_index;
                    if (SL_PORT_UP == entry.slave_status)
                    {
                        entry.slave_port = entry.slave_index;
                    }
                }
                else if (SL_PORT_UP == entry.slave_status)
                {
                    entry.master_port = entry.slave_index;
                    entry.slave_port = 0;
                }
            }
        }
        
        if (0 == smart_link_group_update(&entry))
        {
            return SMART_LINK_RETURN_CODE_SUCCESS;
        }
        else
        {
            smart_link_log_error("Entry update failed.\n");
            return SMART_LINK_RETURN_CODE_ERROR;
        }
    }

    smart_link_log_debug("Smart-link group has not exist.\n");

    return SMART_LINK_RETURN_CODE_ERROR;
}

unsigned int smart_link_advertise_vlan(unsigned int id, unsigned int advertise_vlan)
{
    struct smart_link_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    
    if (0 == smart_link_group_search(&entry))
    {
        entry.advertise_vlan = advertise_vlan;
        if (0 == smart_link_group_update(&entry))
        {
            return SMART_LINK_RETURN_CODE_SUCCESS;
        }
        else
        {
            smart_link_log_error("Entry update failed.\n");
            return SMART_LINK_RETURN_CODE_ERROR;
        }
    }

    smart_link_log_debug("Smart-link group has not exist.\n");

    return SMART_LINK_RETURN_CODE_ERROR;
}

unsigned int smart_link_port
(
    unsigned int id,
    unsigned int netif_m_idx,
    unsigned int netif_m_status,
    unsigned int netif_s_idx,
    unsigned int netif_s_status
)
{
    struct smart_link_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;

    if (0 == smart_link_group_search(&entry))
    {
        if (0 != entry.is_enable)
        {
            smart_link_log_debug("SL grp has enable.\n");
            return SMART_LINK_RETURN_CODE_GROUP_ENABLE;
        }
        
        entry.master_index = netif_m_idx;
        entry.master_status = netif_m_status;
        entry.slave_index = netif_s_idx;
        entry.slave_status = netif_s_status;
        entry.master_port = netif_m_idx;
        if (0 == smart_link_group_update(&entry))
        {
            return SMART_LINK_RETURN_CODE_SUCCESS;
        }
        else
        {
            smart_link_log_error("Entry update failed.\n");
            return SMART_LINK_RETURN_CODE_ERROR;
        }
    }

    smart_link_log_debug("Entry has not exist.\n");

    return SMART_LINK_RETURN_CODE_ERROR;
}


#define SMART_LINK_DBUS_FOR_ERROR(err)          \
do {                                            \
    smart_link_log_error("Unable to get input args\n");    \
    if (dbus_error_is_set(&err))                \
    {                                           \
        smart_link_log_error("%s raised: %s\n", err.name, err.message);        \
        dbus_error_free(&err);                  \
    }                                           \
    return NULL;                                \
} while (0)

#define SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret)         \
do {                                                                \
    DBusMessageIter iter;                                           \
    reply = dbus_message_new_method_return(msg);                    \
    dbus_message_iter_init_append(reply, &iter);                    \
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);  \
} while (0)

DBusMessage* smart_link_dbus_uint32_processor(DBusMessage* msg, unsigned int (*processor)(unsigned int))
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int value = 0;
    unsigned int ret = SMART_LINK_RETURN_CODE_SUCCESS;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &value,
                    DBUS_TYPE_INVALID)))
    {
        SMART_LINK_DBUS_FOR_ERROR(err);
    }

    ret = processor(value);

    SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* smart_link_dbus_2_uint32_processor(DBusMessage* msg, unsigned int (*processor)(unsigned int, unsigned int))
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int value_1 = 0;
    unsigned int value_2 = 0;
    unsigned int ret = SMART_LINK_RETURN_CODE_SUCCESS;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &value_1,
                    DBUS_TYPE_UINT32, &value_2,
                    DBUS_TYPE_INVALID)))
    {
        SMART_LINK_DBUS_FOR_ERROR(err);
    }

    ret = processor(value_1, value_2);

    SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* smart_link_dbus_5_uint32_processor
(
    DBusMessage* msg,
    unsigned int (*processor)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int)
)
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int value_1 = 0;
    unsigned int value_2 = 0;
    unsigned int value_3 = 0;
    unsigned int value_4 = 0;
    unsigned int value_5 = 0;
    unsigned int ret = SMART_LINK_RETURN_CODE_SUCCESS;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &value_1,
                    DBUS_TYPE_UINT32, &value_2,
                    DBUS_TYPE_UINT32, &value_3,
                    DBUS_TYPE_UINT32, &value_4,
                    DBUS_TYPE_UINT32, &value_5,
                    DBUS_TYPE_INVALID)))
    {
        SMART_LINK_DBUS_FOR_ERROR(err);
    }

    ret = processor(value_1, value_2, value_3, value_4, value_5);

    SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* smart_link_dbus_msti(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int id = 0;
    unsigned int instance = 0;
    unsigned int length = 0;
    unsigned int ret = SMART_LINK_RETURN_CODE_SUCCESS;
    unsigned char* p_vlan_bitmap = NULL;
    struct smart_link_s entry;
    unsigned char* msti_vlanbitmap_zero = NULL;

    memset(&entry, 0, sizeof(entry));
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &id,
                    DBUS_TYPE_UINT32, &instance,
                    DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &p_vlan_bitmap, &length,
                    DBUS_TYPE_INVALID)))
    {
        SMART_LINK_DBUS_FOR_ERROR(err);
    }

    if (length == sizeof(entry.data_vlan))
    {
        entry.id = id;
        if (0 == smart_link_group_search(&entry))
        {
            if (0 == entry.is_enable)
            {
                msti_vlanbitmap_zero = (unsigned char*)malloc(length * sizeof(unsigned char));
                if (NULL != msti_vlanbitmap_zero)
                {
                    memset(msti_vlanbitmap_zero, 0, length);
                    if (0 != memcmp(msti_vlanbitmap_zero, p_vlan_bitmap, length))
                    {
                        entry.instance = instance;
                        memcpy(&(entry.data_vlan), p_vlan_bitmap, length);
                        if (0 != smart_link_group_update(&entry))
                        {
                            smart_link_log_error("Entry update failed.\n");
            				ret = SMART_LINK_RETURN_CODE_ERROR;
                        }
                    }
                    else
                    {
                        smart_link_log_debug("Msti incompleate.\n");
                        ret = SMART_LINK_RETURN_CODE_MSTI_INCOMPLEATE;
                    }
                    free(msti_vlanbitmap_zero);
                }
                else
                {
                    smart_link_log_error("Memory exhaust.\n");
                    ret = SMART_LINK_RETURN_CODE_ERROR;
                }
            }
            else
            {
                smart_link_log_debug("SL grp has enable.\n");
                ret = SMART_LINK_RETURN_CODE_GROUP_ENABLE;
            }
        }
        else
        {
            smart_link_log_debug("Entry has not exist.\n");
            ret = SMART_LINK_RETURN_CODE_ERROR;
        }
    }
    else
    {
        ret = SMART_LINK_RETURN_CODE_ERROR;
    }

    SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}


DBusMessage* smart_link_dbus_group_get(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int ret = SMART_LINK_RETURN_CODE_SUCCESS;
    struct smart_link_s entry;

    memset(&entry, 0, sizeof(entry));
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &entry.id,
                    DBUS_TYPE_INVALID)))
    {
        SMART_LINK_DBUS_FOR_ERROR(err);
    }

    if (0 != smart_link_group_search(&entry))
    {
        smart_link_log_debug("Smart-link group has not exist.\n");
        ret = SMART_LINK_RETURN_CODE_GROUP_NOT_EXIST;
    }

    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        smart_link_log_error("Smart-link dbus failed.\n");
        return NULL;
    }

    dbus_message_append_args(reply,
                            DBUS_TYPE_UINT32, &ret,
                            DBUS_TYPE_UINT32, &entry.id,
                            DBUS_TYPE_UINT32, &entry.is_enable,
                            DBUS_TYPE_UINT32, &entry.is_preempt,
                            DBUS_TYPE_UINT32, &entry.instance,
                            DBUS_TYPE_UINT32, &entry.master_port,
                            DBUS_TYPE_UINT32, &entry.master_index,
                            DBUS_TYPE_UINT32, &entry.master_status,
                            DBUS_TYPE_UINT32, &entry.slave_index,
                            DBUS_TYPE_UINT32, &entry.slave_status,
                            DBUS_TYPE_UINT32, &entry.advertise_vlan,
							DBUS_TYPE_INVALID);

    return reply;
}

DBusMessage * smart_link_dbus_group(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_uint32_processor(msg, smart_link_group);
}

DBusMessage * smart_link_dbus_no_group(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_uint32_processor(msg, smart_link_no_group);
}

DBusMessage * smart_link_dbus_log_set(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_uint32_processor(msg, smart_link_log_type_set);
}

DBusMessage * smart_link_dbus_log_unset(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_uint32_processor(msg, smart_link_log_type_unset);
}

DBusMessage * smart_link_dbus_enable(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_2_uint32_processor(msg, smart_link_enable);
}

DBusMessage * smart_link_dbus_preempt(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_2_uint32_processor(msg, smart_link_preempt);
}

DBusMessage * smart_link_dbus_advertise_vlan(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_2_uint32_processor(msg, smart_link_advertise_vlan);
}

DBusMessage * smart_link_dbus_port(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    return smart_link_dbus_5_uint32_processor(msg, smart_link_port);
}

DBusMessage* smart_link_dbus_show_run(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessageIter iter;  
    DBusMessage*    reply = NULL;
    unsigned int id = 0;
    int len = 0;
    int ni = 0;
    struct smart_link_s entry;
    char sl_port[SL_MAX][32];
    char netif_name[32];
    char* pbuf = NULL;
    char* cur_str = NULL;
    char* ptr_char = NULL;

    
    
    pbuf = malloc(512 * SMART_LINK_GROUP_SIZE);
    cur_str = pbuf;

    for (id = 1; id < SMART_LINK_GROUP_SIZE + 1; id++)
    {
        memset(&entry, 0, sizeof(entry));
        entry.id = id;
        if (0 == smart_link_group_search(&entry))
        {
            if (entry.is_enable 
                && entry.master_index
                && entry.slave_index)
            {
                len = sprintf(cur_str, "smart-link group %d\n", entry.id);
                cur_str += len;

                memset(sl_port, 0, sizeof(sl_port));
                for (ni = SL_MASTER; ni < SL_MAX; ni++)
                {
                    memset(netif_name, 0, sizeof(netif_name));
                    if (0 == npd_netif_index_to_user_fullname(entry.port[ni].netif_index, netif_name))
                    {
                        ptr_char = strchr(netif_name, ' ');
                        ptr_char++;
                        strncpy(sl_port[ni], ptr_char, sizeof(sl_port[ni]));
                    }
                }
                
                len = sprintf(cur_str, "smart-link port master %s slave %s\n", sl_port[SL_MASTER], sl_port[SL_SLAVE]);
                cur_str += len;

                len = sprintf(cur_str, "advertise-vlan %d\n", entry.advertise_vlan);
                cur_str += len;

                len = sprintf(cur_str, "smart-link instance %d\n", entry.instance);
                cur_str += len;

                len = sprintf(cur_str, "preemption mode\n");
                cur_str += len;

                len = sprintf(cur_str, "smart-link enable\n");
                cur_str += len;

                len = sprintf(cur_str, "exit\n");
                cur_str += len;
            }
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &pbuf);

    if (NULL != pbuf)
    {
        free(pbuf);
    }

    return reply;
}

pthread_t smart_link_dubs_pid;
DBusConnection * smart_link_dbus_connection = NULL;

static DBusHandlerResult smart_link_dbus_message_handler 
(
    DBusConnection * connection, 
    DBusMessage * message, 
    void * user_data
)
{
    DBusMessage *reply = NULL;

    smart_link_log_debug("XXX: path(%s), interface(%s), method(%s)\n",
                    dbus_message_get_path(message),
                    dbus_message_get_interface(message),
                    dbus_message_get_member(message));

    if (0 == strcmp(dbus_message_get_path(message), SMART_LINK_DBUS_OBJPATH))
    {
        if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_GROUP))
        {
            reply = smart_link_dbus_group(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_NO_GROUP))
        {
            reply = smart_link_dbus_no_group(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_ENABLE))
        {
            reply = smart_link_dbus_enable(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_PREEMPT))
        {
            reply = smart_link_dbus_preempt(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_ADVERTISE_VLAN))
        {
            reply = smart_link_dbus_advertise_vlan(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_PORT))
        {
            reply = smart_link_dbus_port(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_MSTI))
        {
            reply = smart_link_dbus_msti(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_GROUP_GET))
        {
            reply = smart_link_dbus_group_get(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_LOG_TYPE_SET))
        {
            reply = smart_link_dbus_log_set(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_METHOD_LOG_TYPE_UNSET))
        {
            reply = smart_link_dbus_log_unset(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, SMART_LINK_DBUS_INTERFACE, SMART_LINK_DBUS_SHOW_RUNNING))
        {
            reply = smart_link_dbus_show_run(connection, message, user_data);
        }
    }

    if (reply)
    {
        dbus_connection_send (connection, reply, NULL);
        dbus_connection_flush(connection);
        dbus_message_unref (reply);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

int smart_link_dbus_connection_init(int* fd)
{
    DBusError   dbus_error;
    int         ret = 0;
    DBusObjectPathVTable    smart_link_vtable = {NULL, &smart_link_dbus_message_handler, NULL, NULL, NULL, NULL};

    smart_link_log_debug("Start smart-link dbus init...\n");

    dbus_connection_set_change_sigpipe (TRUE);

    dbus_error_init (&dbus_error);
    smart_link_dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbus_error);
    if (smart_link_dbus_connection == NULL)
    {
        smart_link_log_error("Smart-link get dbus for cli failed, error %s\n", dbus_error.message);
        return -1;
    }

    if (!dbus_connection_register_fallback (smart_link_dbus_connection, SMART_LINK_DBUS_OBJPATH, &smart_link_vtable, NULL))
    {
        smart_link_log_error("Smart-link dbus for cli register vtable fallback failed!\n");
        return -1;       
    }
        
    ret = dbus_bus_request_name (smart_link_dbus_connection, SMART_LINK_DBUS_BUSNAME, 0, &dbus_error);
    if (-1 == ret)
    {
        smart_link_log_error("Smart-link dbus for cli request name error %d\n", ret);
        return -1;
    }
    else
    {
        smart_link_log_debug("Smart-link dbus request name %s ok.\n", SMART_LINK_DBUS_BUSNAME);
    }

    if (dbus_error_is_set (&dbus_error))
    {
        smart_link_log_error("Smart-link dbus for cli request name message:%s.\n", dbus_error.message);
        return -1;
    }

    smart_link_log_debug("Smart-link dbus(%p) for cli dispatcher connected.\n", smart_link_dbus_connection);

    dbus_connection_get_socket(smart_link_dbus_connection, fd);
    
    return 0;
}

int smart_link_send_cmd(struct thread * t)
{
    int sock = -1;
    int ret = DBUS_DISPATCH_COMPLETE;

    sock = THREAD_FD (t);

    dbus_connection_read_write_dispatch(smart_link_dbus_connection, 100);

    ret = dbus_connection_get_dispatch_status(smart_link_dbus_connection);
    if (DBUS_DISPATCH_COMPLETE != ret)
    {
        smart_link_log_event("dbus write dispatch status %d, need do more.\n", ret);
        smart_link_event(SL_DBUS_WRITE, sock);
    }
    
    return 0;
}

int smart_link_recv_cmd(struct thread * t)
{

    int sock = -1;
    int ret = DBUS_DISPATCH_COMPLETE;

    sock = THREAD_FD (t);
    
    smart_link_event(SL_DBUS_READ, sock);

    /* for read fd */
    dbus_connection_read_write_dispatch(smart_link_dbus_connection, -1);

    /* for callback function(as vtable-function) */
    dbus_connection_read_write_dispatch(smart_link_dbus_connection, 100);

    ret = dbus_connection_get_dispatch_status(smart_link_dbus_connection);
    if (DBUS_DISPATCH_COMPLETE != ret)
    {
        smart_link_log_event("dbus write dispatch status %d, need do more.\n", ret);
        smart_link_event(SL_DBUS_WRITE, sock);
    }

    return 0;
}

int smart_link_dbus_sock_init()
{
    int sock = -1;
    
    if (0 != smart_link_dbus_connection_init(&sock))
    {
        smart_link_log_error("Smart-link dbus init failed.\n");
        return -1;
    }

    smart_link_event(SL_DBUS_READ, sock);

    smart_link_master_board_set(app_act_master_running());

    return 0;
}

#endif

