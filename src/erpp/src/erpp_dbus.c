
#ifdef HAVE_ERPP

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <dbus/dbus.h>

#include "sysdef/returncode.h"
#include "lib/db_app_sync.h"
#include "lib/netif_index.h"
#include "lib/chassis_man_app.h"
#include "lib/npd_bitop.h"
#include "lib/npd_database.h"
#include "nam/nam_rx.h"
#include "quagga/thread.h"

#include <erpp/erpp_main.h>
#include <erpp/erpp_dbus.h>
#include <erpp/erpp_event.h>
#include <erpp/erpp_packet.h>
#include <erpp/erpp_timer.h>
#include <erpp/erpp_log.h>

static DBusConnection *erpp_dbus_connection = NULL;
pthread_t erpp_dbus_thread;

extern int ERPP_LOCAL_MODULE_ISMASTERACTIVE;
extern pthread_mutex_t semErppMutex;

int is_active_master_board = 0;
int erpp_global_array_index = 0;
db_table_t* erpp_domain_db = NULL;
db_table_t* erpp_global_cfgtbl = NULL;
array_table_index_t* erpp_array_index = NULL;
array_table_index_t *erpp_global_array = NULL;

void erpp_master_board_set(int is_active_master)
{
    is_active_master_board = is_active_master;
    return ;
}

int erpp_domain_search(struct erpp_domain_s* entry)
{
    return (0 == dbtable_array_get(erpp_array_index, entry->domain_id, entry)) ? 0 : -1;
}

int erpp_domain_insert(struct erpp_domain_s* entry)
{
    return (0 == dbtable_array_insert_byid(erpp_array_index, entry->domain_id, entry)) ? 0 : -1;
}

int erpp_domain_update(struct erpp_domain_s* entry)
{
    return (0 == dbtable_array_update(erpp_array_index, entry->domain_id, NULL, entry)) ? 0 : -1;
}

int erpp_domain_delete(struct erpp_domain_s* entry)
{
    return (0 == dbtable_array_delete(erpp_array_index, entry->domain_id, entry)) ? 0 : -1;
}

int erpp_domain_get_next(struct erpp_domain_s* entry)
{
    do
    {
        entry->domain_id++;
    } while ((entry->domain_id <= ERPP_DOMAIN_SIZE)
        && (0 != erpp_domain_search(entry)));

    if (entry->domain_id > ERPP_DOMAIN_SIZE)
    {
        return -1;
    }

    return 0;
}

int erpp_ring_id_exist_check(unsigned int ring_id, unsigned int domain_id)
{
    int ret = -1;
    struct erpp_domain_s domain_s;

	memset(&domain_s, 0, sizeof(struct erpp_domain_s));

	domain_s.domain_id = domain_id;
	if (0 != erpp_domain_search(&domain_s))
	{
        ret = ERPP_RETURN_CODE_SUCCESS;
	}
	else
	{
        if(domain_s.ring[ring_id-1].ring_id != 0)
        {
            ret = ERPP_RETURN_CODE_RING_CONFLICT;
		}
		else
		{
			ret = ERPP_RETURN_CODE_SUCCESS;
		}
	}
	return ret;
}

void erpp_send_msg_to_npd
(
   struct erpp_port_s *port,
   unsigned int instance,
   unsigned int flush_flag,
   unsigned int is_enable
)
{
    struct erpp_msg_to_npd_s data;
    
    memset(&data, 0, sizeof(data));
    
    data.validate_byte_begin = VALIDATE_BYTE_BEGIN;
	data.port[0].netif_index = port[0].netif_index;
	data.port[1].netif_index = port[1].netif_index;
	data.port[0].erpp_port_status = port[0].erpp_port_status;
	data.port[1].erpp_port_status = port[1].erpp_port_status;
	data.instance = instance;
    data.flush_flag = flush_flag;
	data.is_enable = is_enable;
    data.validate_byte_end = VALIDATE_BYTE_END;

    erpp_send_msg((char*)&data, sizeof(data));
    erpp_syslog_dbg("Send msg to npd.\n");
    
    return ;
}
void erpp_get_ring_id_from_domain(struct erpp_domain_s* domain, int *ring_id)
{   
    int i = 0;
	for(i = 0; i < ERPP_RING_SIZE; i++)
	{
        if(domain->ring[i].ring_id != 0)
        {
			*(ring_id+i) = domain->ring[i].ring_id;
		}
	}
}

long erpp_handle_insert(void* new_entry)
{
    int flush_flag = -1;
	int count = 0;
	int ring[ERPP_RING_SIZE] = { 0 };
	
    struct erpp_domain_s* new = (struct erpp_domain_s*)new_entry;

    if (NULL == new)
    {
        return -1;
    }	

    return 0;
}

long erpp_handle_update(void* new_entry, void* old_entry)
{
    int flush_flag = -1;
	int count = 0;
	int ring[ERPP_RING_SIZE] = { 0 };
	
    struct erpp_domain_s* new = (struct erpp_domain_s*)new_entry;
    struct erpp_domain_s* old = (struct erpp_domain_s*)old_entry;

    if ((NULL == new) || (NULL == old))
    {
        return -1;
    }
	
    if(new->flush_flag == old->flush_flag)
		flush_flag = 0;
	else if(new->flush_flag == 0)
		flush_flag = 0;
	else
		flush_flag = 1;

	erpp_get_ring_id_from_domain(new, ring);
	
    if (ERPP_LOCAL_MODULE_ISMASTERACTIVE)
    {
        for(count = 0; count < ERPP_RING_SIZE; count++)
        {
            if(new->ring[count].ring_id != 0)
            {
		        if (new->ring[count].is_enable != old->ring[count].is_enable)
		        {
		            if(new->ring[count].is_enable)
		            {
		                if((new->ring[count].node.port[0].netif_index != 0 || new->ring[count].node.port[1].netif_index != 0))
		                {
							erpp_send_msg_to_npd(new->ring[count].node.port, new->protect_instance_id, flush_flag, new->ring[count].is_enable);
						}
		            }
					else
					{
		                if((new->ring[count].node.port[0].netif_index != 0 || new->ring[count].node.port[1].netif_index != 0))
		                {
							erpp_send_msg_to_npd(new->ring[count].node.port, new->protect_instance_id, flush_flag, new->ring[count].is_enable);
						}
					}
		        }
				else if (new->ring[count].is_enable)
				{
					if((new->ring[count].node.port[0].erpp_port_status != old->ring[count].node.port[0].erpp_port_status)||
					(new->ring[count].node.port[1].erpp_port_status != old->ring[count].node.port[1].erpp_port_status))
					{      
						erpp_send_msg_to_npd(new->ring[count].node.port, new->protect_instance_id, flush_flag, new->ring[count].is_enable);
					}
				}
            }
        }
    }
    return 0;
}

int erpp_db_init()
{
    int ret;
    ret = create_dbtable("erpp_domain",
                        ERPP_DOMAIN_SIZE + 1, 
                        sizeof(struct erpp_domain_s),
                        erpp_handle_update, 
                        NULL,
                        erpp_handle_insert, 
                        NULL, 
                        NULL,
                        NULL,
                        NULL, 
                        NULL,
                        NULL,
                        DB_SYNC_ALL,
                        &(erpp_domain_db));
    if (0 != ret)
    {
        erpp_syslog_err("ERPP: Failed to create erpp domain.\n");
        return -1;
    }

    ret = dbtable_create_array_index("erpp_domain_index", 
                            erpp_domain_db,
                            &erpp_array_index);

    if (0 != ret)
    {
        erpp_syslog_err("ERPP: Failed to create index for domain.\n");
        return -1;
    }	
	
	ret = create_dbtable( "erpp_global", 1, sizeof(struct erpp_global_configure_s),\
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
					&erpp_global_cfgtbl);
    if( 0 != ret )
	{
		erpp_syslog_err("ERPP: Failed to create erpp global configuration database.\n");
		return -1;
	}

	ret = dbtable_create_array_index("erpp_global", erpp_global_cfgtbl, &erpp_global_array);
    if( 0 != ret )
	{
		erpp_syslog_err("ERPP: Failed to create index for global configuration database.\n");
		return -1;
	}
	
	struct erpp_global_configure_s global_configure;
	memset(&global_configure, 0 , sizeof(struct erpp_global_configure_s));
    ret = dbtable_array_insert(erpp_global_array, (unsigned int *)&erpp_global_array_index, &global_configure);

	return ret;

}
int erpp_domain_process(unsigned int domain, unsigned int isenable)
{
    int ret = -1;
    struct erpp_domain_s domain_s;

	memset(&domain_s, 0, sizeof(struct erpp_domain_s));

	domain_s.domain_id = domain;
	
	if (0 != erpp_domain_search(&domain_s))
	{
         if(isenable)
         {
             domain_s.hello_timer = 3 * ERPP_TIMER_HELLO_DEFAULT;
			 domain_s.fail_timer = 3 * ERPP_TIMER_FAIL_DEFAULT;
			 domain_s.fault_timer = domain_s.fail_timer;
			 domain_s.timer_count[0] = domain_s.hello_timer;
			 domain_s.timer_count[1] = domain_s.fail_timer;
			 domain_s.timer_count[2] = domain_s.fault_timer;
		 	 ret = erpp_domain_insert(&domain_s);
         }
		 else
		 	 ret = ERPP_RETURN_CODE_SUCCESS;
	}
	else
	{
		if(!isenable)
            ret = erpp_domain_delete(&domain_s);
		else
			ret = ERPP_RETURN_CODE_SUCCESS;
			
	}

	return ret;
}

int erpp_domain_control_vlan_set(unsigned int domain, unsigned short vlan_id)
{
    int ret = -1;
    struct erpp_domain_s domain_s;

	memset(&domain_s, 0, sizeof(struct erpp_domain_s));

	domain_s.domain_id = domain;
	pthread_mutex_lock(&semErppMutex);
	if (0 != erpp_domain_search(&domain_s))
	{
        ret = ERPP_RETURN_CODE_ERROR;
	}
	else
	{
        domain_s.control_vlan_id[0] = vlan_id;
		//domain_s.control_vlan_id[1] = vlan_id+1;
		ret = erpp_domain_update(&domain_s);
		if(ret != 0)
			ret = ERPP_RETURN_CODE_ERROR;
	}
	pthread_mutex_unlock(&semErppMutex);
	return ret;
}

int erpp_domain_instance_set(unsigned int domain, unsigned int instance)
{
    int ret = -1;
    struct erpp_domain_s domain_s;

	memset(&domain_s, 0, sizeof(struct erpp_domain_s));

	domain_s.domain_id = domain;
	pthread_mutex_lock(&semErppMutex);
	if (0 != erpp_domain_search(&domain_s))
	{
        ret = ERPP_RETURN_CODE_ERROR;
	}
	else
	{
	    domain_s.protect_instance_id = instance;
		ret = erpp_domain_update(&domain_s);
		if(ret != 0)
			ret = ERPP_RETURN_CODE_ERROR;
	}
	pthread_mutex_unlock(&semErppMutex);
	
	return ret;
}

int erpp_domain_ring_configure_process
(
    unsigned int domain, 
    unsigned int ring_id,
    unsigned char node_mode,
    unsigned int primary_port,
    unsigned int secondary_port,
    unsigned int level
)
{
    int ret = -1;
    struct erpp_domain_s domain_s;

	memset(&domain_s, 0, sizeof(struct erpp_domain_s));

	domain_s.domain_id = domain;
	
	pthread_mutex_lock(&semErppMutex);
	if (0 != erpp_domain_search(&domain_s))
	{
        ret = ERPP_RETURN_CODE_ERROR;
	}
	else
	{
		if(node_mode == NODE_MASTER)
		{
			domain_s.ring[ring_id-1].node.port[0].erpp_port_role = PORT_PRIMARY;
			domain_s.ring[ring_id-1].node.port[0].erpp_port_status = FORWARDING;
			domain_s.ring[ring_id-1].node.port[1].erpp_port_role = PORT_SECONDARY;
			domain_s.ring[ring_id-1].node.port[1].erpp_port_status= BLOCK;
			domain_s.ring[ring_id-1].node.erpp_node_status = NODE_COMPLETE;			
			domain_s.ring[ring_id-1].node.erpp_node_role = NODE_MASTER;
		}
		else if(node_mode == NODE_TRANSMIT)
		{
			domain_s.ring[ring_id-1].node.port[0].erpp_port_role = PORT_PRIMARY;
			domain_s.ring[ring_id-1].node.port[0].erpp_port_status = FORWARDING;
			domain_s.ring[ring_id-1].node.port[1].erpp_port_role = PORT_SECONDARY;
			domain_s.ring[ring_id-1].node.port[1].erpp_port_status= FORWARDING;
			domain_s.ring[ring_id-1].node.erpp_node_status = LINK_UP;
			domain_s.ring[ring_id-1].node.erpp_node_role = NODE_TRANSMIT;
		}
		else if((node_mode == NODE_EDGE) ||(node_mode == NODE_ASSISTANT_EDGE))
		{
			domain_s.ring[ring_id-1].node.port[0].erpp_port_role = PORT_EDGE;
			domain_s.ring[ring_id-1].node.port[0].erpp_port_status= FORWARDING;
			domain_s.ring[ring_id-1].node.port[1].erpp_port_role = PORT_COMMON;
			domain_s.ring[ring_id-1].node.port[1].erpp_port_status= FORWARDING;
			domain_s.ring[ring_id-1].node.erpp_node_status = LINK_UP;
		}
		domain_s.ring[ring_id-1].ring_id = ring_id;
	    domain_s.ring[ring_id-1].node.erpp_node_role = node_mode;
	    domain_s.ring[ring_id-1].level = level;
	    domain_s.ring[ring_id-1].node.port[0].netif_index = primary_port;		
		domain_s.ring[ring_id-1].node.port[1].netif_index = secondary_port;		

		ret = erpp_domain_update(&domain_s);
		if(ret != 0)
			ret = ERPP_RETURN_CODE_ERROR;
		pthread_mutex_unlock(&semErppMutex);
	}

	return ret;
}
int erpp_domain_ring_process
(
    unsigned int domain, 
    unsigned int ring_id,
    unsigned int isenable
)
{
    int ret = -1;
    struct erpp_domain_s domain_s;

	memset(&domain_s, 0, sizeof(struct erpp_domain_s));

	domain_s.domain_id = domain;
	
	pthread_mutex_lock(&semErppMutex);
	if (0 != erpp_domain_search(&domain_s))
	{
        ret = ERPP_RETURN_CODE_ERROR;
	}
	else
	{
	    domain_s.ring[ring_id-1].is_enable = isenable;
		ret = erpp_domain_update(&domain_s);
		if(ret != 0)
			ret = ERPP_RETURN_CODE_ERROR;
	}
	pthread_mutex_unlock(&semErppMutex);

	return ret;
}

int erpp_global_process
(
    unsigned int isenable
)
{
    int ret = -1;
    struct erpp_global_configure_s global_configure;
	memset(&global_configure, 0 , sizeof(struct erpp_global_configure_s));
	
	if(0 == dbtable_array_get(erpp_global_array, 0, &global_configure))
	{
        if(isenable == global_configure.is_enable)
			return 0;
		
		global_configure.is_enable = isenable;
		ret = dbtable_array_update(erpp_global_array, 0, NULL, &global_configure);
	}

	return ret;
}

int erpp_timer_process
(
    unsigned int domain,
    unsigned int hello_value,
    unsigned int fail_value
)
{
    int ret = -1;
    struct erpp_domain_s domain_s;

	memset(&domain_s, 0, sizeof(struct erpp_domain_s));

	domain_s.domain_id = domain;
	pthread_mutex_lock(&semErppMutex);
	if (0 != erpp_domain_search(&domain_s))
	{
        ret = ERPP_RETURN_CODE_ERROR;
	}
	else
	{
		domain_s.hello_timer = hello_value;
		domain_s.timer_count[0] = domain_s.hello_timer;
		domain_s.fail_timer = fail_value;			
		domain_s.timer_count[1] = domain_s.fail_timer;
		domain_s.fault_timer = fail_value;
		domain_s.timer_count[2] = domain_s.fault_timer;
	}

	ret = erpp_domain_update(&domain_s);
	if(ret != 0)
		ret = ERPP_RETURN_CODE_ERROR;
	pthread_mutex_unlock(&semErppMutex);

	return ret;
}

DBusMessage* erpp_dbus_timer_set(DBusConnection* conn, DBusMessage* msg, void* user_data)
{

	DBusError err;
	DBusMessage *reply = NULL;
	DBusMessageIter iter = {0};
	dbus_error_init(&err);
	
	int ret = -1;
	unsigned int domain_id = 0;
	unsigned int hello_value = 0;
	unsigned int fail_value = 0;
	
	if (!(dbus_message_get_args ( msg, &err,
	    DBUS_TYPE_UINT32, &domain_id,
		DBUS_TYPE_UINT32, &hello_value,
		DBUS_TYPE_UINT32, &fail_value,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(msg);
	if (NULL == reply)
	{
		return reply;
	}
	
	ret = erpp_timer_process(domain_id, hello_value, fail_value);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	return reply;
		
}

DBusMessage* erpp_dbus_domain_info_get(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int ret = ERPP_RETURN_CODE_SUCCESS;
    struct erpp_domain_s entry;
	unsigned int ring_id = 0;

    memset(&entry, 0, sizeof(entry));
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &entry.domain_id,
                    DBUS_TYPE_UINT32, &ring_id,
                    DBUS_TYPE_INVALID)))
    {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
    }
	
    if (0 != erpp_domain_search(&entry))
    {
        erpp_syslog_err("erpp domain has not exist.\n");
        ret = ERPP_RETURN_CODE_DOMAIN_NOEXIST;
    }
	else if(ring_id != entry.ring[ring_id-1].ring_id)
	{
        erpp_syslog_err("erpp ring no exist.\n");
		ret = ERPP_RETURN_CODE_RING_NOEXIST;
	}
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        erpp_syslog_err("erpp dbus failed.\n");
        return NULL;
    }

    dbus_message_append_args(reply,
                            DBUS_TYPE_UINT32, &ret,
                            DBUS_TYPE_UINT32, &entry.domain_id,
                            DBUS_TYPE_UINT32, &entry.protect_instance_id,
                            DBUS_TYPE_UINT16, &entry.control_vlan_id[0],
                            DBUS_TYPE_UINT32, &entry.hello_timer,
                            DBUS_TYPE_UINT32, &entry.fail_timer,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].node.erpp_node_role,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].node.port[0].netif_index,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].node.port[1].netif_index,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].node.port[0].erpp_port_role,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].node.port[1].erpp_port_role,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].node.port[0].erpp_port_status,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].node.port[1].erpp_port_status,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].is_enable,
                            DBUS_TYPE_UINT32, &entry.ring[ring_id-1].level,
							DBUS_TYPE_INVALID);

    return reply;
}

DBusMessage * erpp_dbus_domain(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
    dbus_error_init(&err);
	
	int ret = -1;
	unsigned int domain = 0;
	unsigned int isenable = 0;

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &domain,
		DBUS_TYPE_UINT32, &isenable,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        return reply;
    }

    ret = erpp_domain_process(domain, isenable);

    dbus_message_iter_init_append(reply, &iter);
	
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
	
    return reply;    
}

DBusMessage * erpp_dbus_bind_control_vlan(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
    dbus_error_init(&err);
	
	int ret = -1;
	unsigned int domain = 0;
	unsigned short vlan_id = 0;

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &domain,
		DBUS_TYPE_UINT16, &vlan_id,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        return reply;
    }
	
    ret = erpp_domain_control_vlan_set(domain, vlan_id);

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;    
}

DBusMessage * erpp_dbus_bind_instance(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
    dbus_error_init(&err);
	
	int ret = -1;
	unsigned int domain = 0;
	unsigned int instance = 0;

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &domain,
		DBUS_TYPE_UINT32, &instance,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        return reply;
    }
	
    ret = erpp_domain_instance_set(domain, instance);

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;   
}

DBusMessage * erpp_dbus_ring_configure(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
    dbus_error_init(&err);
	
	int ret = ERPP_RETURN_CODE_SUCCESS;
	unsigned int domain = 0;
	unsigned int ring_id = 0;
	unsigned char node_mode = 0;
	unsigned int primary_port = 0;
	unsigned int secondary_port = 0;
    unsigned int level = 0;
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &domain,
		DBUS_TYPE_UINT32, &ring_id,
		DBUS_TYPE_BYTE,   &node_mode,
		DBUS_TYPE_UINT32, &primary_port,
		DBUS_TYPE_UINT32, &secondary_port,
		DBUS_TYPE_UINT32, &level,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        return reply;
    }

	if(ret == ERPP_RETURN_CODE_SUCCESS)
        ret = erpp_domain_ring_configure_process(domain, ring_id, node_mode, primary_port, secondary_port, level);

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;  
}

DBusMessage * erpp_dbus_ring_enable(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
    dbus_error_init(&err);
	
	int ret = -1;
	unsigned int domain = 0;
	unsigned int ring_id = 0;
    unsigned int isenable = 0;
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &domain,
		DBUS_TYPE_UINT32, &ring_id,
		DBUS_TYPE_UINT32, &isenable,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        return reply;
    }
	
    ret = erpp_domain_ring_process(domain, ring_id, isenable);

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;  
}

DBusMessage * erpp_dbus_enable(DBusConnection* conn, DBusMessage* msg, void* user_data) 
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
    dbus_error_init(&err);
	
	int ret = -1;
    unsigned int isenable = 0;
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32, &isenable,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        return reply;
    }
	
    ret = erpp_global_process(isenable);

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;  
}

DBusMessage *erpp_dbus_set_log_level
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
)
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
	int ret = -1;
	unsigned int rc = 0;
	unsigned char debug_level = 0;
    dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_BYTE,&debug_level,
		DBUS_TYPE_INVALID))) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return NULL;
	}
    switch(debug_level){
    case SYSLOG_DBG_DEF:
        erpp_syslog_dbg("ERPP : disable debug info log\n");
        break;
    case SYSLOG_DBG_DBG:
        erpp_syslog_dbg("ERPP : set log level to DEBUG\n");
        break;
    case SYSLOG_DBG_WAR:
	    erpp_syslog_dbg("ERPP : set log level to WARNING\n");
        break;
    case SYSLOG_DBG_ERR:
        erpp_syslog_dbg("ERPP : set log level to ERROR\n");
        break;
    case SYSLOG_DBG_EVT:
        erpp_syslog_dbg("ERPP : set log level to EVENT\n");
        break;
    case SYSLOG_DBG_INTERNAL:
        erpp_syslog_dbg("ERPP : set log level to INTERNAL\n");
        break;
    case SYSLOG_DBG_ALL:
        erpp_syslog_dbg("ERPP : set log level to ALL\n");
        break;
    default:
        erpp_syslog_dbg("ERPP : unknow level, disable log info log\n");
        debug_level = SYSLOG_DBG_DEF;
        break;
    }
	
    reply = dbus_message_new_method_return(msg);
    if (NULL == reply)
    {
        return reply;
    }
	
    ret = erpp_log_level_set(debug_level);
    if(ret != 0) 
	{
        rc = ERPP_RETURN_CODE_LOG_LEVEL_SET_ERR;
    } else	
    {
        rc = ERPP_RETURN_CODE_SUCCESS;
    }
	
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &rc);
    return reply;
}
   
DBusMessage *erpp_dbus_show_running
(
    DBusConnection *conn,
    DBusMessage *msg,
    void *user_data
)
{
    DBusError err;
    DBusMessage *reply = NULL;
    DBusMessageIter iter = {0};
	int ret = -1;
	int count = 0;
	unsigned int rc = 0;
	struct erpp_global_configure_s global_ctrl;
	struct erpp_domain_s domain_ctrl;
	char *show_buffer = NULL;
    char *cur_str = NULL;
    int length = 0;
	
	length = 2*1024;
	show_buffer = (char *)malloc(length);
	if(show_buffer == NULL)
	{
		rc = ERPP_RETURN_CODE_ALLOC_MEM_NULL;
	}
	
	cur_str = show_buffer;
	memset(show_buffer, 0, length);
	memset(&global_ctrl, 0, sizeof(struct erpp_global_configure_s));
	memset(&domain_ctrl, 0, sizeof(struct erpp_domain_s));
	
    ret = dbtable_array_get(erpp_global_array, 0, &global_ctrl);
	if(ret == 0)
	{
    	if(global_ctrl.is_enable != 0)
		{
			sprintf(cur_str, "frpp enable\n");
			length = strlen(cur_str);
		}
		else
		{
            length = 0;
		}
	}
	
	while(0 == erpp_domain_get_next(&domain_ctrl))
	{
	    if(domain_ctrl.domain_id != 0)
	    {
			sprintf(cur_str + length, "frpp domain %d\n", domain_ctrl.domain_id);
			length = strlen(cur_str);
			if(domain_ctrl.control_vlan_id[0] != 0)
			{
				sprintf(cur_str + length, "control-vlan %d\n", domain_ctrl.control_vlan_id[0]);
				length = strlen(cur_str);
			}
			if(domain_ctrl.protect_instance_id != 0)
			{
				sprintf(cur_str + length, "data-vlan instance %d\n", domain_ctrl.protect_instance_id);
				length = strlen(cur_str);
			}	

			for(count = 0; count < ERPP_RING_SIZE; count++)
			{			
				if(domain_ctrl.ring[count].ring_id != 0)
				{                
					char name_1[32], name_2[32];
					memset(name_1, 0, 32);
					memset(name_2, 0, 32);
					npd_netif_index_to_user_fullname(domain_ctrl.ring[count].node.port[0].netif_index, name_1);				
					npd_netif_index_to_user_fullname(domain_ctrl.ring[count].node.port[1].netif_index, name_2);
				    if(domain_ctrl.ring[count].node.erpp_node_role == NODE_MASTER)
				    {

						sprintf(cur_str + length, "ring %d node-mode master primary-port %s secondary-port %s\n", 
							domain_ctrl.ring[count].ring_id, name_1, name_2);
					    length = strlen(cur_str);
				    }
					else if(domain_ctrl.ring[count].node.erpp_node_role == NODE_TRANSMIT)
					{
						sprintf(cur_str + length, "ring %d node-mode transmit primary-port %s secondary-port %s\n", 
							domain_ctrl.ring[count].ring_id, name_1, name_2);						
					    length = strlen(cur_str);
					}
					else if(domain_ctrl.ring[count].node.erpp_node_role == NODE_EDGE)
					{
						sprintf(cur_str + length, "ring %d node-mode edge edge-port %s common-port %s\n", 
							domain_ctrl.ring[count].ring_id, name_1, name_2);						
					    length = strlen(cur_str);
					}
					else if(domain_ctrl.ring[count].node.erpp_node_role == NODE_ASSISTANT_EDGE)
					{
						sprintf(cur_str + length, "ring %d node-mode assistant-edge edge-port %s common-port %s\n", 
							domain_ctrl.ring[count].ring_id, name_1, name_2);
					    length = strlen(cur_str);
					}
					
					if(domain_ctrl.ring[count].is_enable != 0)
					{
						sprintf(cur_str + length, "ring %d enable\n", domain_ctrl.ring[count].ring_id);
						length = strlen(cur_str);
					}
				}
			}
	    }
		
		if(domain_ctrl.hello_timer != 0)
		{
			sprintf(cur_str + length, "frpp hello-time %d fail-time %d\n", domain_ctrl.hello_timer, domain_ctrl.fail_timer);
			length = strlen(cur_str);
		}	
	}
	
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &show_buffer);

    free(show_buffer);
    return reply;
}

static DBusHandlerResult erpp_dbus_message_handler 
(
    DBusConnection * connection, 
    DBusMessage * message, 
    void * user_data
)
{
    DBusMessage *reply = NULL;

    erpp_syslog_dbg("XXX: path(%s), interface(%s), method(%s)\n",
                    dbus_message_get_path(message),
                    dbus_message_get_interface(message),
                    dbus_message_get_member(message));

    if (0 == strcmp(dbus_message_get_path(message), ERPP_DBUS_OBJPATH))
    {
        if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_DOMAIN))
        {
            reply = erpp_dbus_domain(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_BIND_CONTRIL_VLAN))
        {
            reply = erpp_dbus_bind_control_vlan(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_BIND_INSTANCE))
        {
            reply = erpp_dbus_bind_instance(connection, message, user_data);
        }
		else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_RING_CONFIGURE))
        {
            reply = erpp_dbus_ring_configure(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_RING_ENABLE))
        {
            reply = erpp_dbus_ring_enable(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_ENABLE))
        {
            reply = erpp_dbus_enable(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_TIMER_SET))
        {
            reply = erpp_dbus_timer_set(connection, message, user_data);
        }
        else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_INFO_GET))
        {
            reply = erpp_dbus_domain_info_get(connection, message, user_data);
        }
		else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_LOG_SET))
		{
			reply = erpp_dbus_set_log_level(connection, message, user_data);
		}
	    else if (dbus_message_is_method_call(message, ERPP_DBUS_INTERFACE, ERPP_DBUS_METHOD_SHOW_RUNNING))
        {
            reply = erpp_dbus_show_running(connection, message, user_data);
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

void *erpp_dbus_thread_main
(
    void *arg
)
{
    while (dbus_connection_read_write_dispatch(erpp_dbus_connection, -1))
    {
    }

    return NULL;
}

int erpp_dbus_init()
{
	int ret = -1;
	DBusError dbus_error;
	DBusObjectPathVTable erpp_vtable = {NULL, &erpp_dbus_message_handler, NULL, NULL, NULL, NULL};
	dbus_connection_set_change_sigpipe(1);
	dbus_error_init(&dbus_error);
	erpp_dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);

	if (erpp_dbus_connection == NULL)
	{
		erpp_syslog_dbg("ERPP dbus_bus_get(): %s\n", dbus_error.message);
		return -1;
	}


	if (!dbus_connection_register_fallback(erpp_dbus_connection, ERPP_DBUS_OBJPATH, &erpp_vtable, NULL))
	{
		erpp_syslog_dbg("ERPP: Can't register D-BUS handlers.\n");
		return -1;
	}

	ret = dbus_bus_request_name(erpp_dbus_connection, ERPP_DBUS_BUSNAME, 0, &dbus_error);

	if (-1 == ret)
	{
		erpp_syslog_dbg("ERPP: Dbus request name err %d\n", ret);
		return -1;
	}
	else
	{
		erpp_syslog_dbg("ERPP: Dbus request name ok\n");
	}

	if (dbus_error_is_set(&dbus_error))
	{
		erpp_syslog_dbg("ERPP: Dbus_bus_request_name(): %s\n", dbus_error.message);
		return -1;
	}

	ret = pthread_create(&erpp_dbus_thread,
						 NULL,
						 (void *)erpp_dbus_thread_main,
						 NULL);
	if (0 != ret)
	{
		erpp_syslog_dbg("ERPP: Create dbus message thread fail.\n");
		return -1;
	}
	return ret;
}


#endif

