
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */

#ifdef HAVE_LLDP
#include "man_db.h"
#include "man_lldp.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <pthread.h>
#include "npd/npd_list.h"
#include "npd_database.h"
#include "db_usr_api.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>



struct lldp_port_entry *lldp_local_entry_get(struct lldp_port_entry *local_entry_get)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "local_tlv", "local_tlv_index",  DB_TABLE_ENTRY_SEARCH,
                                    local_entry_get, sizeof(struct lldp_port_entry));
    app_db_conn_close(db_conn);

    if (response == NULL)
    {
        //printf("Canot find table entry based 0x%x\r\n", netif_index);
        return NULL;
    }

    if (response->return_code == 0)
    {
        if (local_entry_get)
        {
            memcpy(local_entry_get, (struct lldp_port_entry*)response->response_entry[i].response_entry_data,sizeof(struct lldp_port_entry));
        }

        dbtable_response_free(response);
        return local_entry_get;
    }
    else
    {
        //printf("Error code: %d\r\n", response->return_code);
        dbtable_response_free(response);
        return NULL;
    }
}
struct lldp_port_entry *lldp_global_entry_get(struct lldp_port_entry *global_entry_get)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "global", "global_index",  DB_TABLE_ENTRY_SEARCH,
                                    global_entry_get, sizeof(struct lldp_port_entry));
    app_db_conn_close(db_conn);

    if (response == NULL)
    {
        return NULL;
    }

    if (response->return_code == 0)
    {
        if (global_entry_get)
        {
            memcpy(global_entry_get, (struct lldp_port_entry*)response->response_entry[i].response_entry_data,sizeof(struct lldp_port_entry));
            //get sys_description
            memset(global_entry_get->sys_desc, 0, 100);
            get_sys_desc(global_entry_get->sys_desc);
        }

        dbtable_response_free(response);
        return global_entry_get;
    }
    else
    {
        dbtable_response_free(response);
        return NULL;
    }

}

int lldp_config_entry_get(struct lldp_port_config *config_port_get)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "config", "config_index",  DB_TABLE_ENTRY_SEARCH,
                                    config_port_get, sizeof(struct lldp_port_config));
    app_db_conn_close(db_conn);

    if (response == NULL)
    {
        //printf("Canot find table entry based 0x%x\r\n", netif_index);
        return -1;
    }

    if (response->return_code == 0)
    {
        if (config_port_get)
        {
            memcpy(config_port_get, (struct lldp_port_config*)response->response_entry[i].response_entry_data,sizeof(struct lldp_port_config));
        }

        dbtable_response_free(response);
        return 0;
    }
    else
    {
        //printf("Error code: %d\r\n", response->return_code);
        dbtable_response_free(response);
        return -1;
    }
}

//config entry one by one
int lldp_config_entry_traversal(config_entry_handler traversal_handle,char *out_str)
{
    int ret = 0;
    int i = 0;
    int count=0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    struct lldp_port_config config_port_tmp;
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "config", "config_index",  DB_TABLE_ENTRY_TRAVER,
                                    &config_port_tmp, sizeof(struct lldp_port_config));
    app_db_conn_close(db_conn);

    if (response == NULL)
    {
        return -1;
    }

    if (response->return_code == 0)
    {
        count=response->entry_count;
        //printf("Response entry count=%d\r\n",count);        
        for(;i<count;i++)
        {
            memcpy(&config_port_tmp,(struct lldp_port_config*)response->response_entry[i].response_entry_data,sizeof(struct lldp_port_config));            
            traversal_handle(&config_port_tmp,out_str);            
        }
        dbtable_response_free(response);
        return count;
    }
    else
    {
        dbtable_response_free(response);
        return -1;
    }
}

int lldp_config_entry_delete(struct lldp_port_config *config_port_ref)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "config", "config_index",  DB_TABLE_ENTRY_DELETE,
                                    config_port_ref, sizeof(struct lldp_port_config));
    app_db_conn_close(db_conn);

    if (response == NULL)
    {
        return -1;
    }

    if (response->return_code == 0)
    {
        dbtable_response_free(response);
        return 0;
    }
    else
    {
        dbtable_response_free(response);
        return -1;
    }
}
int lldp_config_entry_insert(struct lldp_port_config *config_port_ref)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "config", "config_index",  DB_TABLE_ENTRY_INSERT,
                                    config_port_ref, sizeof(struct lldp_port_config));
    app_db_conn_close(db_conn);

    if (response == NULL)
    {
        return -1;
    }

    if (response->return_code == 0)
    {
        dbtable_response_free(response);
        return 0;
    }
    else
    {
        dbtable_response_free(response);
        return -1;
    }
}


int lldp_config_entry_update(struct lldp_port_config *config_port_ref)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "config", "config_index",  DB_TABLE_ENTRY_UPDATE,
                                    config_port_ref, sizeof(struct lldp_port_config));
    app_db_conn_close(db_conn);
    if(response == NULL)
    {
        return -1;
    }
    if(response->return_code == 0)
    {
        dbtable_response_free(response);
        return 0;
    }
    else
    {
        dbtable_response_free(response);
        return -1;
    }
}
struct lldp_port_entry *lldp_neighbor_entry_get(struct lldp_port_entry *neighbor_entry_get)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    /* connect to local service */
    db_conn = app_db_local_open("lldp");
    response = app_db_table_request(db_conn, "neighbor", "neighbor_index",  DB_TABLE_ENTRY_SEARCH,
                                    neighbor_entry_get, sizeof(struct lldp_port_entry));
    app_db_conn_close(db_conn);

    if (response == NULL)
    {
        //printf("Cannot find table entry based 0x%x\r\n", netif_index);
        return NULL;
    }

    if (response->return_code == 0)
    {
        if (neighbor_entry_get)
        {
            memcpy(neighbor_entry_get, (struct lldp_port_entry*)response->response_entry[i].response_entry_data,sizeof(struct lldp_port_entry));
        }

        dbtable_response_free(response);
        return neighbor_entry_get;
    }
    else
    {
        //printf("Error code: %d\r\n", response->return_code);
        dbtable_response_free(response);
        return NULL;
    }
}


int check_lldp_state()
{
    int ret =0;
    struct lldp_port_entry *port_entry=(struct lldp_port_entry*)malloc(sizeof(struct lldp_port_entry));
    if(!port_entry)
    {
        return -1;
    }
    memset(port_entry, 0,sizeof(struct lldp_port_entry));
    if(lldp_global_entry_get(port_entry))
    {
        ret =1;
    }
    free(port_entry);
    return ret;    
}


char *capability_name(uint16_t capability)
{
    switch (capability)
    {
        case SYSTEM_CAPABILITY_OTHER:
            return "Other";
        case SYSTEM_CAPABILITY_REPEATER:
            return "Repeater/Hub";
        case SYSTEM_CAPABILITY_BRIDGE:
            return "Bridge";
        case SYSTEM_CAPABILITY_WLAN:
            return "Wireless LAN";
        case SYSTEM_CAPABILITY_ROUTER:
            return "Router";
        case SYSTEM_CAPABILITY_TELEPHONE:
            return "Telephone";
        case SYSTEM_CAPABILITY_DOCSIS:
            return "DOCSIS/Cable Modem";
        case SYSTEM_CAPABILITY_STATION:
            return "Station";
        default:
            return "Unknown";
    };
}

char *decode_tlv_system_capabilities(uint16_t system_capabilities, uint16_t enabled_capabilities)
{
    int capability = 0;
    char *result = calloc(1, SYS_CAP_SIZE);
    char *supported_string = calloc(1, SYS_CAP_SIZE);
    char *capability_string = calloc(1, SYS_CAP_SIZE);
    char *result_suppot = calloc(1, SYS_CAP_SIZE*2);
    if(!result_suppot)
    {
        printf("No enough memory \r\n");
        if(result)
        {
            free(result);
        }
        if(supported_string)
        {
            free(supported_string);
        }
        if(capability_string)
        {
            free(capability_string);
        }
        return NULL;
    }
    sprintf(result_suppot, "\nSystem Capabilities supported :");
    sprintf(result, "System Capabilities enabled   :");

    for (capability = 1; capability <= SYSTEM_CAPABILITY_STATION; capability *= 2)
    {

        if ((system_capabilities & capability))
        {
            sprintf(supported_string, "\n\t\t\t\t%s", capability_name(capability));
            strncat(result_suppot, supported_string, SYS_CAP_SIZE);
            memset(supported_string, 0, SYS_CAP_SIZE);

            if (enabled_capabilities & capability)
            {
                sprintf(capability_string, "\n\t\t\t\t%s", capability_name(capability));
                strncat(result, capability_string, SYS_CAP_SIZE);
                memset(capability_string, 0, SYS_CAP_SIZE);
            }
        }
    }

    strncat(result_suppot, "\n", SYS_CAP_SIZE);
    strncat(result_suppot, result, SYS_CAP_SIZE);
    free(capability_string);
    free(supported_string);
    free(result);
    return result_suppot;
}


char *tlv_typetoname(uint8_t tlv_type)
{
    switch (tlv_type)
    {
        case CHASSIS_ID_TLV:
            return "Chassis ID";
            break;
        case PORT_ID_TLV:
            return "Port ID";
            break;
        case TIME_TO_LIVE_TLV:
            return "Time To Live";
            break;
        case PORT_DESCRIPTION_TLV:
            return "Port Description";
            break;
        case SYSTEM_NAME_TLV:
            return "System Name";
            break;
        case SYSTEM_DESCRIPTION_TLV:
            return "System Description";
            break;
        case SYSTEM_CAPABILITIES_TLV:
            return "System Capabilities";
            break;
        case MANAGEMENT_ADDRESS_TLV:
            return "Management Address";
            break;
        case ORG_SPECIFIC_TLV:
            return "Organizationally Specific";
            break;
        case END_OF_LLDPDU_TLV:
            return "End Of LLDPDU";
            break;
        default:
            return "Unknown";
    };
}


char *dot1_typetoname(uint8_t tlv_type)
{
    switch (tlv_type)
    {
        case DOT1_PVID_TLV:
            return "Port VLAN ID";
            break;
        case DOT1_PPVID_TLV:
            return "Port And Protocol VLAN ID";
            break;
        case DOT1_VNAME_TLV:
            return "VLAN Name";
            break;
        case DOT1_RESERVED:
            return "Reserved";
            break;
        default:
            return "Unknown";
    };
}


int *parse_time(int *time,int second)
{
    int rest_s = 0;
    time[0] = second/(3600*24);
    rest_s = second%(3600*24);
    time[1] = rest_s/3600;
    rest_s %= 3600;
    time[2] = rest_s/60;
    rest_s %= 60;
    time[3] = rest_s;
    return time;
}

void translate_state(int state, char *stat)
{
    memset(stat, 0, STATE_SIZE);

    if (state)
    {
        sprintf(stat,"%s","Enable");
    }
    else
    {
        sprintf(stat,"%s","disable");
    }
}

void translate_admin_state(int admin_state, char *stat)
{
    memset(stat, 0, STATE_SIZE);

    switch (admin_state)
    {
        case enabledRxTx:
            sprintf(stat,"%s","Tx_Rx");
            break;
        case enabledTxOnly:
            sprintf(stat,"%s","Tx");
            break;
        case enabledRxOnly:
            sprintf(stat,"%s","Rx");
            break;
        case disabled:
            sprintf(stat,"%s","disable");
            break;
        default:
            ;
    };
}

int lldp_config_update(uint8_t config_type, uint32_t netif_index, int arg)
{
    int ret = -1;
    struct lldp_port_config *config_entry = NULL;
    config_entry=(struct lldp_port_config*)malloc(sizeof(struct lldp_port_config));
    if(!config_entry)
    {
        printf("No enough memory \r\n");
        return -1;
    }
    memset(config_entry, 0,sizeof(struct lldp_port_config));
    config_entry->if_index = netif_index;
    ret = lldp_config_entry_get(config_entry);
    if(ret)
    {
        free(config_entry);
        return LLDP_CONFIG_DUMP_ERROR;
    }
    switch(config_type)
    {
        case LLDP_MULTIPLIER_CONFIG:
        {
            config_entry->msgTxHold = arg;
        }
        break;
        case LLDP_TX_DALAY_CONFIG:
        {
            config_entry->txDelay = arg;
        }
        break;
        case LLDP_TX_INTERVAL_CONFIG:
        {
            config_entry->msgTxInterval = arg;
        }
        break;
        case LLDP_REINIT_DELAY_CONFIG:
        {
            config_entry->reinitDelay = arg;
        }
        break;
        case LLDP_ADMIN_STATUS_CONFIG:
        {
            config_entry->adminStatus = arg;
        }
        break;
        default:
            ;
    };
    ret = lldp_config_entry_update(config_entry);
    free(config_entry);
    if(ret)
    {
        return LLDP_CONFIG_UPDATE_ERROR;
    }
    return 0;
}
int lldp_config_enable(struct lldp_port_config *config_entry, uint32_t netif_index)
{
    int ret = -1;
    config_entry->if_index = netif_index;
    config_entry->portEnabled  = 1;
    config_entry->adminStatus  = enabledRxTx;
    config_entry->reinitDelay   = 2;
    config_entry->msgTxHold     = 4;
    config_entry->msgTxInterval = 30;
    config_entry->txDelay       = 2;
    config_entry->update_time = 0;
    ret = lldp_config_entry_insert(config_entry);
    return ret;
}

#endif

