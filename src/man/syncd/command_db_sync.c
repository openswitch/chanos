
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* command_db_sync.c
*
*
* CREATOR:
*		pangxf@autelan.com
*
* DESCRIPTION:
*		sync db for quagga commands and dcli
*
* DATE:
*		8/09/2010
*UPDATE:
*  FILE REVISION NUMBER:
*  		$Revision: 1.06 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "npd/npd_list.h"
#include <pthread.h>
#include <linux/tipc.h>
#include "tipc_api/tipc_api.h"
#include "npd_database.h"
#include <sys/un.h>
#include <sys/socket.h>
#include "chassis_man_app.h"
#include "command_db_sync.h"
#include "db_usr_api.h"
#include "npd/npd_netif_event.h"


db_table_t *config_table = NULL;
hash_table_index_t *config_index = NULL;
OSAL_DB_CTRL *command_db = NULL;

int SYS_LOCAL_MODULE_ISMASTERACTIVE = 0;
int SYS_LOCAL_MODULE_ISMASTERSTANDBY = 0;
int SYS_LOCAL_MODULE_ISMASTER=0;
int SYS_LOCAL_MODULE_SLOT_INDEX = 2;
int SYS_LOCAL_MODULE_SLAVE_INDPT = 0;

int is_active = 0;

int db_sync_flag=0;
extern int command_sync_printf_enable;

#ifndef command_sync_printf
#define command_sync_printf if(command_sync_printf_enable) printf
#endif


char *redundancy_by_cfgfile_module[] = 
{
	"rip",
	"ospf",
	"bgp",
	"ripng",
	"ospf6",
	"pim",
	"dvmrp",
	"snmp",
	"ntp",
	"igmp",
	""
};
char *redundancy_execute_cfgfile_module[] =
{
    "ospf",
    ""
};

/* 此为临时解决RIP等模块的双主配置冗余 */
int back_up_cfg_modules(int slot_id)
{
	int cfg_module_id = 0;
	char cfg_module_file_name[64] = {0};

	while(strlen(redundancy_by_cfgfile_module[cfg_module_id]) != 0)
	{
		char sys_cmd_line[256] = {0};
		sprintf(cfg_module_file_name, "/var/run/%s.conf", redundancy_by_cfgfile_module[cfg_module_id]);

		sprintf(sys_cmd_line, "test -f %s && sudo file_client -n %d %s > /mnt/down.log 2>&1\n", 
			cfg_module_file_name, slot_id, cfg_module_file_name);
		system(sys_cmd_line);
		cfg_module_id++;
	}	
    
	return 0;
}

void command_db_sync_debug_enable()
{
    command_sync_printf_enable = 1;
}

void command_db_sync_debug_disable()
{
    command_sync_printf_enable = 0;
}

//the utilities below are used to resolve cli para

char** resolve_para(char **argv, int argc, char *para)
{
    int i =0;
    int str_len= 0;
    int pos=0;
    int count=0;
    if(!argc)
    {
        return NULL;
    }

    argv=(char**)malloc(sizeof(char*)*argc);

    if (!argv)
    {
        command_sync_printf("alloc memory for **argv failed \n");
        return NULL;
    }

    for (; i<argc-1; i++)
    {
        argv[i] = (char*)malloc(sizeof(char)*PARA_ITEM_SIZE);

        if (!argv[i])
        {
            command_sync_printf("alloc memory for argv[%d] failed \n",i);
            goto error;
        }

        memset(argv[i], 0, sizeof(char)*PARA_ITEM_SIZE);

        for (str_len =0; para[pos+str_len]!=' '&& para[pos+str_len]!='\0'; str_len++)
        {
        }

        strncpy(argv[i],&para[pos],str_len);
        pos+=str_len;
        pos+=1;
    }

    argv[argc-1] = (char*)malloc(sizeof(char)*PARA_ITEM_SIZE);

    if (!argv[argc-1])
    {
        command_sync_printf("alloc memory for argv[%d] failed \n",argc-1);
        return NULL;
    }

    memset(argv[argc-1], 0, sizeof(char)*PARA_ITEM_SIZE);
    strncpy(argv[argc-1],&para[pos],strlen(para)-pos);
    return argv;
    error:
        for(;count<i;count++)
        {
            if(argv[count])
            {
                free(argv[count]);
            }
        }
        free(argv);
        return NULL;
}
void free_argv(int argc, char *argv[])
{
    int i =0;

    if (!argc)
    {
        return;
    }

    for (; i< argc; i++)
    {
        if (argv[i])
        {
            free(argv[i]);
        }
    }

    free(argv);
}

int command_dbtable_slot_event(int event, int service_type, int instance)
{
    int ret = 0;

    if (event == TIPC_PUBLISHED)
    {
        command_sync_printf("published \r\n");

        if (service_type != COMMAND_SERVICE)
        {
            return -1;
        }

        if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
        {
			int slot_id = INSTANCE_TO_SLOT(instance);
			int standby_id = 0;
			
            command_sync_printf("active master send syn msg \n");
            db_sync_flag=1;
            ret = systime_syn();
            ret = back_up_all_users();
            if(ret)
            {
                command_sync_printf("backup users failed \r\n");
            }
            ret = back_up_dns_servers();
            if(ret)
            {
                command_sync_printf("backup dns failed \r\n");
            }

#ifdef HAVE_SYSLOG
            ret = backup_syslog_config();
            if(ret)
            {
                command_sync_printf("backup syslog failed \r\n");
            }

#endif
#ifdef HAVE_NTP
			ret = back_up_ntp_servers();
			if(ret)
            {
                command_sync_printf("backup ntp servers failed \r\n");
            }
#endif
#ifdef HAVE_PAM
            ret = backup_pam_config();
            if(ret)
            {
                command_sync_printf("backup pam failed \r\n");
            }

#endif
#ifdef HAVE_LOGIN_FILTER
            ret = backup_login_filter_config();
#endif
            ret= backup_password_config();
#ifdef HAVE_WEBMNG
            ret=backup_webmng_config();
#endif
             /*对于暂时还没有作主备冗余的模块，主备倒换后只能整体把自己的配置执行一遍，需要把各模块的配置同步到备板相
                 应的文件, 可以在此时生成并进行传输
                 */
			standby_id = app_sbymaster_slot_get();
			if (standby_id > 0 && standby_id==slot_id)
            {
                int cfg_module_id = 0;
               	char cfg_module_file_name[64] = {0};
				{
   					char sys_cmd_line[256] = {0};
					
                    int flag;
					system("cd /var/run/");
                   	while(strlen(redundancy_by_cfgfile_module[cfg_module_id]) != 0)
                   	{
                   	    sprintf(cfg_module_file_name, "/var/run/%s.conf", 
							redundancy_by_cfgfile_module[cfg_module_id]);	
    
    					sprintf(sys_cmd_line, "/opt/bin/vtysh -c 'show running-config %s' > %s\n", 
								redundancy_by_cfgfile_module[cfg_module_id], cfg_module_file_name);
    					system(sys_cmd_line);
                        system("sync");
                         if (chmod (cfg_module_file_name, 0777) != 0)
                         {
                             fprintf (stdout,"%% Can't chmod configuration file %s: %s (%d)\n",
                   							    	  cfg_module_file_name, safe_strerror(errno), errno);
                         }
                   	     cfg_module_id++;
                   	}
				    ret = back_up_cfg_modules(slot_id);
				}
			}
            ret = dbtable_slot_online_insert(slot_id);
 			if (standby_id > 0 && standby_id==slot_id)
            {
                int cfg_module_id = 0;
#ifdef HAVE_OSPF_GR
                sleep(5);
            	while(strlen(redundancy_execute_cfgfile_module[cfg_module_id]) != 0)
            	{
            		  char sys_cmd_line[256] = {0};
                      char cfg_module_file_name[64] = {0};
                	  sprintf(cfg_module_file_name, "/var/run/%s.noconf", redundancy_execute_cfgfile_module[cfg_module_id]);
                	  memset(sys_cmd_line,0,sizeof(sys_cmd_line));
                      sprintf(sys_cmd_line, "sudo remote_exec_client %d '/opt/bin/vtysh -bf %s'\n", slot_id, cfg_module_file_name);
              	      system(sys_cmd_line);
                	  system("sync");
                	  sprintf(cfg_module_file_name, "/var/run/%s.conf", redundancy_execute_cfgfile_module[cfg_module_id]);
                	  memset(sys_cmd_line,0,sizeof(sys_cmd_line));
                      sprintf(sys_cmd_line, "sudo remote_exec_client %d '/opt/bin/vtysh -bf %s'\n", slot_id, cfg_module_file_name);
              	      system(sys_cmd_line);
                	  system("sync");
                	  cfg_module_id++;
            	}
#endif
                cfg_module_id = 0;
                while(strlen(redundancy_by_cfgfile_module[cfg_module_id]) != 0)
               	{
                    /*对于需要在备用主控时就执行配置的不用在主备倒换的时候再做*/
            		char sys_cmd_line[256] = {0};
                    char cfg_module_file_name[64] = {0};
                    int cfg_exec_module_id = 0;
                    int unneed = 0;
#ifdef HAVE_OSPF_GR      
                    while(strlen(redundancy_execute_cfgfile_module[cfg_exec_module_id]))
                    {
                        if(0 == strcmp(redundancy_by_cfgfile_module[cfg_module_id],
                                        redundancy_execute_cfgfile_module[cfg_exec_module_id]))
                        {
                            unneed = 1;
                            break;
                        }
                        cfg_exec_module_id++;
                    }
                    if(unneed)
                    {
                        cfg_module_id++;
                        continue;
                    }
#endif
					memset(sys_cmd_line, 0, 256);
               	    sprintf(cfg_module_file_name, "/var/run/%s.noconf", redundancy_by_cfgfile_module[cfg_module_id]);
                    sprintf(sys_cmd_line, "sudo remote_exec_client %d '/opt/bin/vtysh -bf %s'\n", slot_id, cfg_module_file_name);
              	    system(sys_cmd_line);
                	system("sync");
                	cfg_module_id++;
               	}                
                
            }
            
            db_sync_flag=0;
        }
    }
    else if (event == TIPC_WITHDRAWN)
    {
        command_sync_printf("withdraw \r\n");
        return 0;
    }

    return 0;
}

int command_dbtable_sync(char *buffer, int len, unsigned int sync_flag, int slot_index)
{
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
	npd_sync_msg_header_t *header = NULL;
    int op_ret = 0;
    if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        if (sync_flag & DB_SYNC_ALL)
        {
	        header = (npd_sync_msg_header_t*)buffer;
			
	        npd_dbtable_header_hton(header);	
	
            op_ret = tipc_client_sync_send(COMMAND_SERVICE, -1, buffer, len);
        }
    }
#endif
    return 0;
}

int command_dbtable_init()
{
    if(SYS_LOCAL_MODULE_SLAVE_INDPT != 1)
        return app_db_init(command_dbtable_sync, NULL,SYS_LOCAL_MODULE_ISMASTERACTIVE);
}
int command_dbtable_recv(int fd, char* buf, int len, void *private_data)
{
    if (len <= 0)
    {
        is_active = app_act_master_running();

        if (SYS_LOCAL_MODULE_ISMASTERSTANDBY == 1 )
        {
            command_sync_printf("standby becomes the active master \n");
            is_active =1;
            SYS_LOCAL_MODULE_ISMASTERACTIVE = 1;
            SYS_LOCAL_MODULE_ISMASTERSTANDBY = 0;
            return command_dbtable_init();
        }

        return 0;
    }

    dbtable_recv(fd, buf, len, private_data);
    return 0;
}


void switch_over_handler(unsigned int netif_index, int event, char *private, int len)
{
    if(!SYS_LOCAL_MODULE_ISMASTER)
    {
        return ;
    }
    if (event == NOTIFIER_SWITCHOVER)
    {
        is_active = app_act_master_running();

        if (is_active)
        {
            SYS_LOCAL_MODULE_ISMASTERACTIVE = 1;
            SYS_LOCAL_MODULE_ISMASTERSTANDBY = 0;
            command_sync_printf("switch_over_handler:standby becomes active \n");

			/*恢复各只能通过配置作主备冗余的模块配置*/
			{
                int cfg_module_id = 0;
              	char cfg_module_file_name[64] = {0};
				char sys_cmd_line[256] = {0};
				sprintf(sys_cmd_line, "sudo chmod 777 /var/run/*.conf\n");
				system(sys_cmd_line);

				sleep(1);
               	while(strlen(redundancy_by_cfgfile_module[cfg_module_id]) != 0)
               	{
#ifdef HAVE_OSPF_GR				
                    /*对于需要在备用主控时就执行配置的不用在主备倒换的时候再做*/
                    int cfg_exec_module_id = 0;
                    int unneed = 0;
                    while(strlen(redundancy_execute_cfgfile_module[cfg_exec_module_id]))
                    {
                        if(0 == strcmp(redundancy_by_cfgfile_module[cfg_module_id],
                                        redundancy_execute_cfgfile_module[cfg_exec_module_id]))
                        {
                            unneed = 1;
                            break;
                        }
                        cfg_exec_module_id++;
                    }
					
                    if(unneed)
                    {
                        cfg_module_id++;
                        continue;
                    }
#endif                    
					memset(sys_cmd_line, 0, 256);
               	    sprintf(cfg_module_file_name, "/var/run/%s.conf", redundancy_by_cfgfile_module[cfg_module_id]);

					sprintf(sys_cmd_line, "/opt/bin/vtysh -bf %s\n", cfg_module_file_name);
					system(sys_cmd_line);
					cfg_module_id++;
               	}
            }
			
        }
        else
        {
            SYS_LOCAL_MODULE_ISMASTERACTIVE = 0;
            SYS_LOCAL_MODULE_ISMASTERSTANDBY = 1;
        	int cfg_module_id = 0;
        	char cfg_module_file_name[64] = {0};

            sleep(1);
        	while(strlen(redundancy_execute_cfgfile_module[cfg_module_id]) != 0)
        	{
    		  char sys_cmd_line[256] = {0};
        	  sprintf(cfg_module_file_name, "/var/run/%s.noconf", redundancy_execute_cfgfile_module[cfg_module_id]);
        	  memset(sys_cmd_line,0,sizeof(sys_cmd_line));
              sprintf(sys_cmd_line, "sudo /opt/bin/vtysh -bf %s\n", cfg_module_file_name);
      	      system(sys_cmd_line);
        	  system("sync");
#ifdef HAVE_OSPF_GR    
        	  sprintf(cfg_module_file_name, "/var/run/%s.conf", redundancy_execute_cfgfile_module[cfg_module_id]);
        	  memset(sys_cmd_line,0,sizeof(sys_cmd_line));
              sprintf(sys_cmd_line, "sudo /opt/bin/vtysh -bf %s\n", cfg_module_file_name);
      	      system(sys_cmd_line);
        	  system("sync");
#endif			  
        	  cfg_module_id++;
        	}
            command_sync_printf("switch_over_handler:active becomes standby \n");
        }

        command_dbtable_init();
    }
}

int command_dbtable_thread_main()
{
    int ret = -1;
    SYS_LOCAL_MODULE_SLOT_INDEX = app_local_slot_get();
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)	
    if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        ret = tipc_server_init(COMMAND_SERVICE, SYS_LOCAL_MODULE_SLOT_INDEX, command_dbtable_recv, NULL, NULL);

        if (ret == 0)
        {
            command_sync_printf("sync active master init:start server\n");
        }

        ret = tipc_client_init(COMMAND_SERVICE, command_dbtable_slot_event);

        if (!ret)
        {
            command_sync_printf("sync active master init:start client \n");
        }
    }
    else if (SYS_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        ret = tipc_server_init(COMMAND_SERVICE, SYS_LOCAL_MODULE_SLOT_INDEX, command_dbtable_recv, NULL, NULL);

        if (ret == 0)
        {
            command_sync_printf("sync master standby init:start server \n");
        }

        ret = tipc_client_init(COMMAND_SERVICE, command_dbtable_slot_event);

        if (!ret)
        {
            command_sync_printf("sync master standby init:start client \r\n");
        }
    }
    else
    {
        ret = tipc_server_init(COMMAND_SERVICE, SYS_LOCAL_MODULE_SLOT_INDEX, command_dbtable_recv, NULL, NULL);

        if (ret == 0)
        {
            command_sync_printf("sync service init:start server \r\n");
        }
    }

    osal_thread_master_run(COMMAND_SERVICE);
#else
    while (1)
    {
        sleep(86400);
    }
#endif

    return 0;
}

int generate_index(char *name)
{
    int i =0;
    int index =0;
    int len =0;

    if (!name || !strlen(name))
    {
        command_sync_printf("Illegal config name \n");
        return -1;
    }

    len = strlen(name);

    for (; i< len; i++)
    {
        index+= name[i];
    }

    index%=ENTRY_NUM;
    return index;
}

unsigned int config_index_get(void *config_struct)
{
    struct config_common_t *config_entry = (struct config_common_t*)config_struct;
    int index = generate_index(config_entry->name);
    return index;
}



/* boot entry handler */
unsigned int cmp_config_index(void *config_struct_a, void *config_struct_b)
{
    struct config_common_t *config_a = (struct config_common_t*)config_struct_a;
    struct config_common_t *config_b = (struct config_common_t*)config_struct_b;
    return !strcmp(config_a->name,config_b->name);
}

long handle_config_update(void *data_new, void *data_old)

{
    char **argv = NULL;
    struct config_common_t *new_config = (struct config_common_t*)data_new;
    if(db_sync_flag)
    {
        return 0;
    }
    argv= resolve_para(argv,  new_config->argc, new_config->para);
#ifdef HAVE_DIFF_OS	
	if(0 == strncmp(new_config->name, SYS_TIME, strlen(SYS_TIME)))
	{
		sys_time_func(new_config->argc,argv,new_config->extra_para);
	}	
#else
	if(new_config->handler)
        new_config->handler(new_config->argc,argv,new_config->extra_para);
#endif
	
    free_argv(new_config->argc,argv);
    return 0;
}
//init db

long handle_config_insert(void *data_new)
{
    char **argv = NULL;
    struct config_common_t *new_config = (struct config_common_t*)data_new;

    if (SYS_LOCAL_MODULE_ISMASTERACTIVE)
    {
        return 0;
    }

    argv= resolve_para(argv, new_config->argc, new_config->para);

#ifdef HAVE_DIFF_OS	
	if(0 == strncmp(new_config->name, SYS_TIME, strlen(SYS_TIME)))
	{
		sys_time_func(new_config->argc,argv,new_config->extra_para);
	}	
#else
	if(new_config->handler)
        new_config->handler(new_config->argc,argv,new_config->extra_para);
#endif
	
    free_argv(new_config->argc,argv);
}

int handle_config_ntoh(void *data_new)
{	
	struct config_common_t *new_config = (struct config_common_t*)data_new;

	new_config->argc = ntohl(new_config->argc);

	return 0;
}

int handle_config_hton(void *data_new)
{
	struct config_common_t *new_config = (struct config_common_t*)data_new;
	
	new_config->argc = htonl(new_config->argc);

    return 0;
}

void handle_cmd_segfault()
{
    fprintf(stderr, "[FATAL] SIGSEGV  (Segmentation Fault)!\n");
    fflush(stderr);
    fflush(stdout);
    exit(-1);
}


int delete_config_entry(hash_table_index_t *index,void *data, unsigned int flag)
{
    int ret = -1;
    ret = dbtable_hash_delete(index, data, data);
    return ret;
}
void clean_cmd_db()
{
    int count =0;
    count = dbtable_hash_traversal(config_index,0, NULL,NULL,delete_config_entry);

    if (config_table)
    {
        free(config_table);
		config_table = NULL;
    }

    if (config_index)
    {
        free(config_index);
		config_index = NULL;
    }

    if (command_db)
    {
        char command_db_path[DB_PATH_SIZE];
        memset(command_db_path, 0, DB_PATH_SIZE);
        sprintf(command_db_path, "%s%s", DB_LOCAL_SERVICE_PATH_PREFIX, command_db->name);
        unlink(command_db_path);

        if (command_db->local_listen)
        {
            free(command_db->local_listen);
        }

        if (command_db->tipc_listen)
        {
            free(command_db->tipc_listen);
        }

        free(command_db);
		command_db = NULL;
    }

    exit(0);
}

int db_init()
{
    int ret = 0; 
    
    create_dbtable(CONFIG_TABLE_NAME,ENTRY_NUM, sizeof(struct config_common_t),
                   handle_config_update,
                   NULL,
                   handle_config_insert,
                   NULL,NULL,NULL,
                   NULL, handle_config_ntoh, handle_config_hton, DB_SYNC_ALL, &config_table);
                   
    dbtable_create_hash_index(CONFIG_INDEX_NAME, config_table, ENTRY_NUM, config_index_get,
                              cmp_config_index, &config_index);
    system(SHOW_PWD_SCRIPT);
    if(is_active)
    {
        ret = default_config();
        ret = back_up_all_users();
        if(ret)
        {
            command_sync_printf("backup users failed \r\n");
        }
        ret = back_up_dns_servers();
        if(ret)
        {
            command_sync_printf("backup dns failed \r\n");
        }

#ifdef HAVE_SYSLOG
        ret = backup_syslog_config();
        if(ret)
        {
            command_sync_printf("backup syslog failed \r\n");
        }

#endif
#ifdef HAVE_NTP
        ret = back_up_ntp_servers();
		if(ret)
        {
            command_sync_printf("backup ntp servers failed \r\n");
        }
#endif
#ifdef HAVE_PAM
        ret = backup_pam_config();
        if(ret)
        {
            command_sync_printf("backup pam failed \r\n");
        }

#endif
#ifdef HAVE_LOGIN_FILTER
        ret = backup_login_filter_config();
#endif
        ret =backup_password_config();
#ifdef HAVE_WEBMNG
        ret=backup_webmng_config();
#endif
    }
    else
    {
#ifdef HAVE_DIFF_OS
		ret = default_config();
#else
        init_password_config();
#ifdef HAVE_SYSLOG
        ret=init_syslog_config();
#endif
#ifdef HAVE_PAM
        ret=init_pam_config();
#endif
#endif
    }

    command_db = create_db("command", COMMAND_LOCAL_SERVICE);

    if (command_db == NULL)
    {
        command_sync_printf("Create db (%s:%d) failed!!!\n", "command", COMMAND_LOCAL_SERVICE);
        ret =-1;
    }

    if (ret)
    {
        command_sync_printf("db init failed \n");
    }

    signal(SIGTERM, clean_cmd_db);
    signal(SIGINT, clean_cmd_db);
    signal(SIGQUIT, clean_cmd_db);
    signal(SIGSEGV, handle_cmd_segfault);
    signal(SIGPIPE, SIG_IGN);
    return ret;
}



int  main(int argc, char *argv[])
{
    if (argc == 2)
    {
        command_sync_printf_enable = atoi(argv[1]);
    }
    db_table_init();
	
#ifdef HAVE_DIFF_OS
	SYS_LOCAL_MODULE_ISMASTER = 0;
	SYS_LOCAL_MODULE_SLAVE_INDPT = 0;
#else
    SYS_LOCAL_MODULE_ISMASTER = app_slot_work_mode_get();
    SYS_LOCAL_MODULE_SLAVE_INDPT = app_slave_indpnt_runget();
#endif

	if(SYS_LOCAL_MODULE_ISMASTER)
    {
        is_active = app_act_master_running();

        if (is_active == -1)
        {
            command_sync_printf("Get board running status failed \n");
            return -1;
        }

        if (is_active)
        {
            SYS_LOCAL_MODULE_ISMASTERACTIVE = 1;
        }
        else
        {
            SYS_LOCAL_MODULE_ISMASTERSTANDBY = 1;
        }
    }
    if (db_init())
    {
        command_sync_printf("init db table failed \n");
        return -1;
    }

    netif_app_event_init(SYNCD_SWITCH_OVER_INSTANCE);
    netif_app_event_op_register(switch_over_handler, NULL);
    osal_thread_create(NULL, osal_thread_master_run, 80, 0x4000, (void *)TIPC_APP_EVENT_SERVICE);

    command_dbtable_init();
    command_dbtable_thread_main();
    return 0;
}
#ifdef __cplusplus
}
#endif
