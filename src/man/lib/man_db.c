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
#include "man_db.h"

char *get_para(int argc, char *argv[])
{
    int i=0;
    int count =0;
    char *para = NULL;
    if(!argc)
    {
        return NULL;
    }
    for(; i<argc;i++)
    {
        if(!argv[i])
        {
            printf("error:argv[%d] is NULL \n",i);
            return NULL;
        }
        if(strlen(argv[i]) > PARA_ITEM_SIZE)
        {
            printf("The size of para item must be smaller that %d \n",PARA_ITEM_SIZE);
            return NULL;
        }
        count += strlen(argv[i]);
        count++;
    }
    if(count > MAX_PARA_SIZE)
    {
        printf("The total size of para must be smaller that %d \n",MAX_PARA_SIZE);
        return NULL;
    }
    para =(char*)malloc(sizeof(char)*count);
    if(!para)
    {
        printf("alloc memory for para failed \n");
        return NULL;
    }
    memset(para, 0, sizeof(char)*count);
    for(i=0; i<argc;i++)
    {
        strcat(para,argv[i]);
        if(i== argc-1)
        {
            return para;
        }
        strcat(para," ");
    }
       
}



int config_entry_delete(struct config_common_t *config_entry)
{
    int ret = 0;
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("command");
    response = app_db_table_request(db_conn, "config", "config",  DB_TABLE_ENTRY_DELETE,
                                    config_entry, sizeof(struct config_common_t));
    app_db_conn_close(db_conn);
    if (response == NULL)
    {
        //printf("Cannot delete table entry based %s\r\n", config_entry->name);
        return -1;
    }
    if (response->return_code == 0)
    {
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
int config_entry_get(struct config_common_t *config_entry)
{
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("command");
    response = app_db_table_request(db_conn, "config", "config",  DB_TABLE_ENTRY_SEARCH,
                                    config_entry, sizeof(struct config_common_t));
    app_db_conn_close(db_conn);

    if(response == NULL)
    {
        //printf("response null \n");
        return -1;
    }

    if(response->return_code == 0)
    {
        if(config_entry)
        {
            memcpy(config_entry, (struct config_common_t*)response->response_entry[i].response_entry_data,sizeof(struct config_common_t));
        }

        dbtable_response_free(response);
        return 0;
    }
    else
    {
        //printf("error:%d \n",response->return_code);
        dbtable_response_free(response);
        return -1;
    }
}

int config_entry_update(struct config_common_t *config_entry)
{
    int i = 0;
    DBTABLE_RESPONSE *response = NULL;
    DB_CONNECT_SESSION *db_conn = NULL;
    db_conn = app_db_local_open("command");
    response = app_db_table_request(db_conn, "config", "config",  DB_TABLE_ENTRY_UPDATE,
                                    config_entry, sizeof(struct config_common_t));
    app_db_conn_close(db_conn);

    if(response == NULL)
    {
        printf("Cannot update table entry \r\n");
        return -1;
    }

    if(response->return_code == 0)
    {
        dbtable_response_free(response);
        return 0;
    }
    else
    {
        printf("Error code: %d\r\n", response->return_code);
        dbtable_response_free(response);
        return -1;
    }
}


int db_update_via_para(char *name,int argc,char *para, char *extra)
{
    int ret = -1;
    struct config_common_t config_entry;
    if(!name||!strlen(name)||strlen(name)> CONFIG_NAME_SIZE)
    {
        printf("The size of config name must between 0-%d \n",CONFIG_NAME_SIZE);
        return -1;
    }
    memset(&config_entry, 0,sizeof(struct config_common_t));
    memcpy(config_entry.name,name,strlen(name));     
    ret = config_entry_get(&config_entry);
    if(ret)
    {
        printf("got entry %s failed \n",name);
        return -1;
    }
    if(!strncmp(name, SYS_TIME, strlen(SYS_TIME))&&!strncmp(config_entry.para, para,strlen(para)))
    {
        if(strlen(config_entry.extra_para))
        {
            memset(config_entry.extra_para, 0, PARA_ITEM_SIZE);
        }
        else
        {
            memcpy(config_entry.extra_para, "time", strlen("time"));
        }
    }
    memset(config_entry.para, 0, MAX_PARA_SIZE);
    if(para && strlen(para))
    {
        if(strlen(para) > MAX_PARA_SIZE)
        {
            printf("The size of para must be smaller than %d \n",MAX_PARA_SIZE);
            return -1;
        }
        memcpy(config_entry.para, para,strlen(para));
    }    
    if(extra && strlen(extra))
    {
        if(strlen(extra) > PARA_ITEM_SIZE)
        {
            printf("The size of extra para must be smaller than %d \n",PARA_ITEM_SIZE);
            return -1;
        }
        memset(config_entry.extra_para, 0, PARA_ITEM_SIZE);
        memcpy(config_entry.extra_para, extra, strlen(extra));
    }
    config_entry.argc = argc;
    ret = config_entry_update(&config_entry);
    if(ret)
    {
        printf("update entry failed \n");
    }
    return ret;
    
}

int db_update_by_name(char *name,int argc, char *argv[], char *extra)
{
    int ret = -1;
    char *para = NULL;
    para = get_para(argc,argv);
    ret=db_update_via_para(name,argc, para, extra);
    if(para)
    {
        free(para);
    }
    if(ret)
    {
        printf("update entry failed \n");
    }
    return ret;
}
int db_entry_delete_via_para(char *name,int argc, char *para, char *extra)
{
    int ret = -1;
    struct config_common_t config_entry;
    ret = db_update_via_para(name,argc,para,extra);
    if(ret)
    {
        printf("delete entry failed \n");
        return -1;
    } 
    if(!strncmp(para,"all",strlen("all")))
    {
        return 0;
    }
    memset(&config_entry, 0,sizeof(struct config_common_t));
    if(extra)
    {
        memcpy(config_entry.name,extra,strlen(extra));
    }
    strncat(config_entry.name,para,strlen(para));
    if(!config_entry_get(&config_entry))
    {
            ret = config_entry_delete(&config_entry);
            if(ret)
            {
                printf("delete entry failed \n");
        }  
    }
    return ret;
    
}

int db_entry_delete_by_para(char *name,int argc, char *argv[], char *extra)
{
    int ret = -1;
    struct config_common_t config_entry;
    if(!argc ||!argv[0] || !strlen(argv[0]))
        {
        return -1;
    }
    ret = db_update_by_name(name,argc,argv,extra);
    if(ret)
    {
        printf("delete entry failed \n");
        return -1;
    } 
    if(!strncmp(argv[0],"all",strlen("all")))
    {
        return 0;
    }
    memset(&config_entry, 0,sizeof(struct config_common_t));
    if(extra)
    {
        memcpy(config_entry.name,extra,strlen(extra));
    }
    strncat(config_entry.name,argv[0],strlen(argv[0]));
    if(!config_entry_get(&config_entry))
    {
            ret = config_entry_delete(&config_entry);
            if(ret)
            {
                printf("delete entry failed \n");
        }  
    }
    return ret;
    
}


