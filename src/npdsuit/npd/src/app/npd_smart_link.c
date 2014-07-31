/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_SMART_LINK
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd/protocol/stp_api.h"
#include "npd_smart_link.h"

int npd_sl_app_msg_sock = -1;
struct	sockaddr_un npd_sl_app_msg_addr;

db_table_t* npd_sl_db = NULL;
hash_table_index_t* npd_sl_hash_idx = NULL; 

void npd_smart_link_send(int sock, struct sockaddr_un* to, char* data, unsigned int length)
{
    int	ret = 0;
    int byte_send = 0;

    if (sock < 0)
    {
        npd_syslog_err("Socket isnot init...!\n");
        return;
    }

    while (length != byte_send)
    {
        ret = sendto(sock, data + byte_send, length - byte_send, 0,
                (struct sockaddr *)to, sizeof(*to));

        if (ret < 0)
        {
            if (errno == EINTR)/*send() be interrupted.*/
            {
                npd_syslog_err("sendto was interrupted.\n");
                continue;
            }
            else
            {
                npd_syslog_err("send message fail, %s.\n", strerror(errno));
                break;
            }
        }
        else
        {
            byte_send += ret;
        }
    }

    return ;
}

void npd_smart_link_send_msg(char* data, unsigned int length)
{
    npd_smart_link_send(npd_sl_app_msg_sock, &npd_sl_app_msg_addr, data, length);
    return ;
}

int npd_smart_link_msg_handler(char *msg, int len)
{
    int count = 0;
    int status = 0;
    unsigned int back_id = 0;
    struct smart_link_msg_s* data = NULL;

    if (NULL == msg)
    {
        npd_syslog_err("Smart-link recv-msg handler message null.\n");
        return -1;
    }

    data = (struct smart_link_msg_s*)msg;

    if ((SL_MSG_COOKIE_ID != data->cookie)
        || (SL_MSG_COOKIE_ID_END != data->cookie_end))
    {
        npd_syslog_err("Smart-link recv-msg handler message cookie not match.\n");
        return -1;
    }

    npd_syslog_dbg("SL block port %x, unblock port %x, instance %x, back_id %x\n",\
        data->block_port, data->unblock_port, data->instance, data->back_id);

    if ((~0UL) != data->back_id)
    {
        if (data->block_port && data->instance)
        {
            status = npd_netif_get_status(data->block_port);
            for (count = 0; (1 != status) && (count < 20); count++)
            {
                usleep(1000 * 25);
                status = npd_netif_get_status(data->block_port);
            }
            npd_smart_link_stp_flag(data->block_port, data->instance, 1, NAM_STP_PORT_STATE_DISCARD_E);
        }

        if (data->unblock_port && data->instance)
        {
            status = npd_netif_get_status(data->unblock_port);
            for (count = 0; (1 != status) && (count < 20); count++)
            {
                usleep(1000 * 25);
                status = npd_netif_get_status(data->unblock_port);
            }
            npd_smart_link_stp_flag(data->unblock_port, data->instance, 1, NAM_STP_PORT_STATE_FORWARD_E);
        }

        back_id = data->back_id;
        if (back_id)
        {
            npd_smart_link_send_msg((char*)&back_id, sizeof(back_id));
        }

        if (data->block_port)
        {
            (void)npd_arp_snooping_del_by_ifindex(data->block_port);
            (void)npd_fdb_dynamic_entry_del_by_port(data->block_port);
        }
    }
    else
    {
        npd_smart_link_stp_flag(data->block_port, data->instance, 0, NAM_STP_PORT_STATE_FORWARD_E);
        npd_smart_link_stp_flag(data->unblock_port, data->instance, 0, NAM_STP_PORT_STATE_FORWARD_E);
    }

    return 0;
}

int	npd_smart_link_sock_init(int* sock)
{
    int fl_flags = 0;
    int lo_sock = -1;
    struct	sockaddr_un npd_sl_app_msg_recv_addr;

    lo_sock = socket(AF_LOCAL, SOCK_DGRAM, 0);

    if (lo_sock < 0)
    {
        npd_syslog_err("Create npd msg to smart-link socket fail\n");
        return -1;
    }

    fl_flags = fcntl(lo_sock, F_GETFL, 0);
    (void)fcntl(lo_sock, F_SETFL, fl_flags | O_NONBLOCK);

    memset(&npd_sl_app_msg_addr, 0, sizeof(npd_sl_app_msg_addr));
    npd_sl_app_msg_addr.sun_family = AF_LOCAL;
    strcpy(npd_sl_app_msg_addr.sun_path, "/tmp/npd_to_smartlink_common");

    memset(&npd_sl_app_msg_recv_addr, 0, sizeof(npd_sl_app_msg_recv_addr));
    npd_sl_app_msg_recv_addr.sun_family = AF_LOCAL;
    strcpy(npd_sl_app_msg_recv_addr.sun_path, "/tmp/smartlink_npd_common");
    
    unlink(npd_sl_app_msg_recv_addr.sun_path);
    (void)chmod(npd_sl_app_msg_recv_addr.sun_path, 0777);

    if (bind(lo_sock, (struct sockaddr *)&npd_sl_app_msg_recv_addr, sizeof(npd_sl_app_msg_recv_addr)) == -1) 
    {
        npd_syslog_err("npd msg to smart-link socket bind failed.\n");
        close(lo_sock);
        lo_sock = -1;
        return -1;
    }
    *sock = lo_sock;

    return 0;
}

int npd_smart_link_port_search(struct npd_smart_link_port_s* entry)
{
    return (0 == dbtable_hash_search(npd_sl_hash_idx, entry, NULL, entry)) ? 0 : -1;
}

int npd_smart_link_port_insert(struct npd_smart_link_port_s* entry)
{
    return (0 == dbtable_hash_insert(npd_sl_hash_idx, entry)) ? 0 : -1;
}

int npd_smart_link_port_update(struct npd_smart_link_port_s* entry)
{
    return (0 == dbtable_hash_update(npd_sl_hash_idx, entry, entry)) ? 0 : -1;
}

unsigned int npd_smart_link_port_next_get(struct npd_smart_link_port_s* entry)
{
    if (0 == entry->netif_index)
    {
        return (0 == dbtable_hash_head(npd_sl_hash_idx, NULL, entry, NULL)) ? 0 : -1;
    }
    else
    {
        return (0 == dbtable_hash_next(npd_sl_hash_idx, entry, entry, NULL)) ? 0 : -1;
    }

    return -1;
}

unsigned int npd_smart_link_hash_key(void* data)
{
    int hash_index = 0;
    unsigned int netif_index = 0;
    struct npd_smart_link_port_s* entry = (struct npd_smart_link_port_s*)data;

    netif_index = entry->netif_index;
    
    hash_index = ((netif_index & 0xFFFFFFF) >> 14);
    
    return hash_index % MAX_SWITCHPORT_PER_SYSTEM;
}

unsigned int npd_smart_link_hash_cmp(void* data1, void* data2)
{
    struct npd_smart_link_port_s* entry_1 = (struct npd_smart_link_port_s*)data1;
    struct npd_smart_link_port_s* entry_2 = (struct npd_smart_link_port_s*)data2;

    return entry_1->netif_index == entry_2->netif_index;
}

int npd_smart_link_db_init()
{
    int ret = 0;

    ret = create_dbtable("npdSmartLinkDb", MAX_SWITCHPORT_PER_SYSTEM, sizeof(struct npd_smart_link_port_s), 
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        DB_SYNC_ALL,
                        &(npd_sl_db));
    if (0 != ret)
    {
        npd_syslog_err("create npd smart-link database fail.\n");
        return -1;
    }

    ret = dbtable_create_hash_index("npdSmartLinkHash", npd_sl_db, MAX_SWITCHPORT_PER_SYSTEM,
                        npd_smart_link_hash_key,
                        npd_smart_link_hash_cmp,
                        &npd_sl_hash_idx);
    if (0 != ret)
    {
        npd_syslog_err("create npd smart-link hash index fail.\n");
        return -1;
    }

    return 0;
}

int npd_smart_link_msg_init(void)
{
    int ret = 0;
    
    if (0 != npd_smart_link_sock_init(&npd_sl_app_msg_sock))
    {
        npd_syslog_err("Create smart-link msg socket failed.\n");
        return -1;
    }

    npd_syslog_dbg("Create npd msg to smart-link socket %d ok.\n", npd_sl_app_msg_sock);

    ret = npd_app_msg_socket_register(npd_sl_app_msg_sock, "SmartLinkMsg", npd_smart_link_msg_handler, 512);
    if (0 != ret)
    {
        npd_syslog_err("resigter smart-link msg handler failed.\n");
        return -1;
    }

    ret = npd_smart_link_db_init();
    if (0 != ret)
    {
        npd_syslog_err("Create smart-link db failed.\n");
        return -1;
    }

    return 0;
}

#define NPD_SMART_LINK_DBUS_FOR_ERROR(err)          \
do {                                            \
    npd_syslog_dbg("Unable to get input args\n");   \
    if (dbus_error_is_set(&err))                \
    {                                           \
        npd_syslog_err("%s raised: %s\n", err.name, err.message);        \
        dbus_error_free(&err);                  \
    }                                           \
    return NULL;                                \
} while (0)

#define NPD_SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret)     \
do {                                                                \
    DBusMessageIter iter;                                           \
    reply = dbus_message_new_method_return(msg);                    \
    if (NULL == reply)                                              \
    {                                                               \
        npd_syslog_err("Smart-link dbus set error!\n");             \
        return reply;                                               \
    }                                                               \
    dbus_message_iter_init_append(reply, &iter);                    \
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);  \
} while (0)

DBusMessage* npd_dbus_smart_link_netif_status_get(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage* reply = NULL;
    DBusError err;
    unsigned int netif_index = 0;
    unsigned int ret = 0;

    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                            DBUS_TYPE_UINT32, &netif_index,
                            DBUS_TYPE_INVALID)))
    {
        NPD_SMART_LINK_DBUS_FOR_ERROR(err);
    }

    ret = npd_switch_port_exist_check(netif_index);
    if (0 == ret)
    {
        ret = npd_netif_get_status(netif_index);
        if ((ret != 0) && (ret != 1))
        {
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    NPD_SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* npd_dbus_smart_link_adv_vlan_list(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int netif_index = 0;
    unsigned int length = 0;
    int ret = 0;
    struct npd_smart_link_port_s entry;
    short* p_vlan_list = NULL;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &netif_index,
                    DBUS_TYPE_ARRAY, DBUS_TYPE_UINT16, &p_vlan_list, &length,
                    DBUS_TYPE_INVALID)))
    {
        NPD_SMART_LINK_DBUS_FOR_ERROR(err);
    }

    memset(&entry, 0, sizeof(entry));
    entry.netif_index = netif_index;

    if (0 == npd_smart_link_port_search(&entry))
    {
        length = (length > NPD_ADV_VLAN_LIST_MAX) ? NPD_ADV_VLAN_LIST_MAX : length;
        memcpy(entry.adv_vlan_list, p_vlan_list, length * sizeof(short));
        ret = npd_smart_link_port_update(&entry);
        if (ret < 0)
        {
            npd_syslog_err("Smart-link port update error!\n"); 
        }
    }
    else
    {
        length = (length > NPD_ADV_VLAN_LIST_MAX) ? NPD_ADV_VLAN_LIST_MAX : length;
        memcpy(entry.adv_vlan_list, p_vlan_list, length * sizeof(short));
        ret = npd_smart_link_port_insert(&entry);
        if (ret < 0)
        {
            npd_syslog_err("Smart-link port insert error!\n"); 
        }
    }

    ret = 0;

    NPD_SMART_LINK_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* npd_dbus_smart_link_adv_vlan_list_get(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int netif_index = 0;
    unsigned int ret = 0;
    struct npd_smart_link_port_s entry;
    short* p_vlan_list = NULL;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &netif_index,
                    DBUS_TYPE_INVALID)))
    {
        NPD_SMART_LINK_DBUS_FOR_ERROR(err);
    }

    memset(&entry, 0, sizeof(entry));
    entry.netif_index = netif_index;

    ret = npd_smart_link_port_next_get(&entry);

    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        npd_syslog_err("Smart-link dbus set error!\n");
        return reply;
    }

    p_vlan_list= entry.adv_vlan_list;
    dbus_message_append_args(reply,
							DBUS_TYPE_UINT32,
							&ret,
							
							DBUS_TYPE_UINT32,
							&entry.netif_index,
							
							DBUS_TYPE_ARRAY,
							DBUS_TYPE_UINT16,
							&p_vlan_list,
							NPD_ADV_VLAN_LIST_MAX,

                            DBUS_TYPE_INVALID);

    return reply;
}

int npd_smart_link_advertise_check(unsigned int netif_index, short vlan_id)
{
    int ni = 0;
    struct npd_smart_link_port_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.netif_index = netif_index;

    if (0 != npd_smart_link_port_search(&entry))
    {
        npd_syslog_dbg("smart-link port not exist.\n");
        return -1;
    }

    for (ni = 0; ni < NPD_ADV_VLAN_LIST_MAX; ni++)
    {
        if (vlan_id == entry.adv_vlan_list[ni])
        {
            return 0;
        }
    }

    return -1;
}

int npd_smart_link_packet_rx_process
(
    int packet_type,
    unsigned char* packet,
    int length,
    unsigned int netif_index,
    unsigned short vlan_id,
    char is_tagged
)
{
#define MAX_IP_COUNT 8
    short adv_vlan_id = 0;
    int nj = 0;
    int ret = 0;
    unsigned int ifindex = 0;
    unsigned int g_ifindex = 0;
    unsigned int ip_addr[MAX_IP_COUNT];
    unsigned int ip_mask[MAX_IP_COUNT];
    struct ethernet_header_s
    {
        unsigned char dmac[6];
        unsigned char smac[6];
        unsigned short type;
    };
    struct advertise_s* advertise = NULL;

    advertise = (struct advertise_s*)(packet + sizeof(struct ethernet_header_s));

    npd_syslog_dbg("SL adv-vlan (%d)!\n", ntohs(advertise->advertise_vlan));
    if (0 == npd_check_vlan_exist(ntohs(advertise->advertise_vlan)))
    {
        npd_syslog_dbg("Smart-link advertise-vlan not exist!\n");
        return -1;
    }

    if (0 != npd_smart_link_advertise_check(netif_index, ntohs(advertise->advertise_vlan)))
    {
        npd_syslog_dbg("Smart-link port does not configure advertise-vlan!\n");
        return -1;
    }

    for (adv_vlan_id = 1; adv_vlan_id < 4095; adv_vlan_id++)
    {
        if (NPD_VBMP_MEMBER(advertise->data_vlan, adv_vlan_id))
        {
            if (0 != npd_check_vlan_exist(adv_vlan_id))
            {
                if (0 != npd_fdb_dynamic_entry_del_by_vlan(adv_vlan_id))
                {
                    npd_syslog_dbg("Smart-link delete dynamic fdb entry failed!\n");
                    continue;
                }

                ifindex = 0;
                ret = npd_intf_exist_check(npd_netif_vlan_index(adv_vlan_id), &ifindex);
                if ((NPD_TRUE != ret) || (0 == ifindex))
                {
                    npd_syslog_dbg("Smart-link get l3-ifindex failed. ret(%d), ifindex(%d)!\n", ret, ifindex);
                    continue;
                }

                if (NPD_TRUE != npd_intf_gindex_exist_check(npd_netif_vlan_index(adv_vlan_id), &g_ifindex))
                {
                    npd_syslog_dbg("Smart-link get l3-g-ifindex failed.\n");
                    continue;
                }

                memset(ip_addr, 0, sizeof(ip_addr));
                memset(ip_mask, 0, sizeof(ip_mask));
                if (NPD_TRUE == npd_intf_addr_ip_get(g_ifindex, ip_addr, ip_mask))
                {
                    for (nj = 0; (nj < MAX_IP_COUNT) && ip_addr[nj]; nj++)
                    {
                        npd_syslog_dbg("SL ip = %x, mask = %x\n", ip_addr[nj], ip_mask[nj]);
                        (void)npd_arp_snooping_del_by_network(ip_addr[nj], ip_mask[nj]);
                    }
                }
            }
            else
            {
                npd_syslog_dbg("Smart-link specially advertise vlan not exist.\n");
            }
        }
    }

    return 0;
}

#endif

