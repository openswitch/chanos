/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* command_db_sync.c
*
*
* CREATOR:
*		zhanwei@autelan.com
*
* DESCRIPTION:
*		sync db for quagga commands and dcli
*
* DATE:
*		8/09/2010	
*UPDATE:
*08/17/2010              pangxf@autelan.com            Using DB. Bug shooting.
*09/15/2010              pangxf@autelan.com            Re-define the structs in following classes: 
*                                                                                     'Golable configuration', 'Local port configuration'
*                                                                                     'Remote port information', 'State machine'.
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
#include <linux/tipc.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/if_tunnel.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "command_db_sync.h"
#include "npd/npd_list.h"
#include "npd_database.h"
#include "shadow.h"
#include "syslog_conf.h"
#include "ntp_conf.h"

extern int SYS_LOCAL_MODULE_ISMASTER;
extern hash_table_index_t *config_index;
#ifdef HAVE_DIFF_OS
int command_sync_printf_enable=1;
#else
int command_sync_printf_enable=0;
#endif
#define command_sync_printf if(command_sync_printf_enable) printf

char *argv_concat (const char **argv, int argc, int shift)
{
  int i;
  size_t len;
  char *str;
  char *p;
  len = 0;
  for (i = shift; i < argc; i++)
  {
    len += strlen(argv[i])+1;
  }
  if (!len)
  {
    return NULL;
  }
  p = str = calloc(1, len);
  for (i = shift; i < argc; i++)
    {
      size_t arglen;
      memcpy(p, argv[i], (arglen = strlen(argv[i])));
      p += arglen;
      *p++ = ' ';
    }
  *(p-1) = '\0';
  return str;
}

#ifdef HAVE_DIFF_OS

int get_system_time(char *sys_time)
{
    struct tm *tm_ptr;
    time_t now_time;
    (void)time(&now_time);
    tm_ptr = localtime(&now_time);
    strftime(sys_time, 30, "%Y/%m/%d %H:%M:%S", tm_ptr);
    return 0;
}

int get_dns_str(char **name)
{
	FILE *fp=NULL;
	char *ptr=NULL;
	int i=0;
	fp = fopen("/etc/resolv.conf","r");
	if(!fp)
		return -1;
	ptr=malloc(128);
	if(!ptr)
		{
		fclose(fp);
		return -1;
	}
	while(fgets(ptr,128,fp))
	{
		if(!strncmp(ptr,"nameserver ",11))
		{
			sprintf(*(name+i),ptr+11);
			i++;
		}
	}
	free(ptr);
	fclose(fp);
	return i;
}

int set_dns_str(const char **name,int num)
{
	FILE * fp=NULL;
	char *ptr=NULL;
	int i=0;
	fp = fopen("/etc/resolv.conf","w");
	if(!fp)
		return -1;
	ptr=malloc(128);
	if(!ptr)
	{
		fclose(fp);
		return -1;
	}
	for(i=0;i<num;i++)
	{
			sprintf(ptr,"nameserver %s\n",*(name+i));
			fputs(ptr,fp);
	}
	free(ptr);
	fclose(fp);
	return 0;
}

int is_user_self(char* name)
{
	int uid;
	struct passwd *passwd;

	uid = geteuid();
	passwd = getpwnam(name);
	if(!passwd)
		return -1;
	if(uid == passwd->pw_uid)
		return 1;
	else
		return 0;
}

int get_pam_status(char *rad_state,char *tac_state)
{
    int ret =0;
    FILE *fp=NULL; 
    fp = fopen(PAM_STATE_FILE,"r");
    if(!fp)
    {
        /*printf("%% Open file %s failed\r\n",PAM_STATE_FILE);*/
        return -1;
    }
    fscanf(fp,"%s",rad_state);
    fscanf(fp,"%s",tac_state);
    fclose(fp);
    return ret;   
}

 int execute_dcli_shell (const char *command)
{		
	return system(command);

}

int is_user_exsit(char* name)
{
	struct passwd *passwd;

	passwd = getpwnam(name);
	if(passwd)
		return 1;
	else
		return 0;

}

int dcli_user_add_sh(const char* name,const char* password,char* enable,char* sec)
{
	char command[128];
	if(!name || !password || !enable || !sec)
		return -3;
	sprintf(command,"useradd.sh %s \'%s\' %s %s",name,password,enable,sec);
	return execute_dcli_shell(command);

}

int set_cli_syslog_str(int num)
{
    FILE * fp=NULL;
    char *ptr=NULL;
    int i=0;
    fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"w");

    if(!fp)
        return -1;

    ptr=malloc(8);

    if(!ptr)
    {
        fclose(fp);
        return -1;
    }

    sprintf(ptr,"%d\n",num);
    fputs(ptr,fp);
    free(ptr);
    fclose(fp);
    return 0;
}

int set_idle_timeout(int minutes)
{
    FILE* fp=NULL;
    char ptr[64];
    memset(ptr,0,64);
    fp = fopen("/var/run/idle_timeout.conf","w");
    if(!fp)
    {
        return -1;
    }
    sprintf(ptr,"idle_timeout %d\n",minutes);
    fputs(ptr,fp);
    fclose(fp);
    return 0;
}

int get_syslog_local_port()
{
    char port_str[8];
    FILE *fp =fopen(SYSLOG_LOCAL_PORT,"r");
    if(!fp)
    {
        /*printf("%%Open file %s failed \r\n",SYSLOG_LOCAL_PORT);*/
        return -1;
    }
    memset(port_str,0,8);
    fgets(port_str,8,fp);
    fclose(fp);
    return atoi(port_str);
    
}

char* get_sys_location(char *location)
{
	FILE* fp=NULL;
	char ptr[SYS_LOCATION_STR_LEN];

	memset(ptr,0,SYS_LOCATION_STR_LEN);
	fp = fopen(SYS_LOCATION_CONFIG_FILE,"r");
	
	if(!fp)
	{
		return NULL;
	}
	while(fgets(ptr,SYS_LOCATION_STR_LEN,fp))	
	{
		if(!strncmp(ptr,SYS_LOCATION_PREFIX,strlen(SYS_LOCATION_PREFIX)))
		{
			sprintf(location,ptr+strlen(SYS_LOCATION_PREFIX));
			fclose(fp);
			return location;

		}
	}
	fclose(fp);
	return NULL;
}

char* get_net_element(char *net_element)
{
	FILE* fp=NULL;
	char ptr[NET_ELEMENT_STR_LEN];

	memset(ptr,0,NET_ELEMENT_STR_LEN);
	fp = fopen(NET_ELEMENT_CONFIG_FILE,"r");
	
	if(!fp)
	{
		return NULL;
	}
	while(fgets(ptr,NET_ELEMENT_STR_LEN,fp))	
	{
		if(!strncmp(ptr,NET_ELEMENT_PREFIX,strlen(NET_ELEMENT_PREFIX)))
		{
			sprintf(net_element,ptr+strlen(NET_ELEMENT_PREFIX));
			fclose(fp);
			return net_element;

		}
	}
	fclose(fp);
	return NULL;
}

int set_net_element(char *net_element)
{
	FILE* fp=NULL;
	char ptr[NET_ELEMENT_STR_LEN];

	if(!net_element)
		return -1;
	memset(ptr,0,NET_ELEMENT_STR_LEN);
	fp = fopen(NET_ELEMENT_CONFIG_FILE,"w");
	if(!fp)
		return -1;
	
	sprintf(ptr,"%s%s\n",NET_ELEMENT_PREFIX,net_element);
	fputs(ptr,fp);
	fclose(fp);
	return 0;
}

int get_idle_time(char *idle_str)
{
    FILE* fp=NULL;
    char ptr[64];
    char get_idle=0;
    int idle_time;
    memset(ptr,0,64);
    fp = fopen("/var/run/idle_timeout.conf","r");
    if(fp)
    {
        while(fgets(ptr,64,fp))
        {
            if(!strncmp(ptr,"idle_timeout ",13))
            {
                get_idle=1;
                break;
            }
        }
        fclose(fp);
    }
    if(get_idle)
    {
        memcpy(idle_str,ptr+13,strlen(ptr)-13);
    }
    else
    {
        sprintf(idle_str,"%d",10);
    }
    return 0;
}

int get_sys_desc(char *desc)
{
	FILE * fp=NULL;
	fp = fopen("/etc/sys_desc","r");
	if(!fp)
	{
        //printf("%% Open file:/etc/sys_desc failed\r\n");
		return -1;
	}
	fgets(desc,256,fp);
    fclose(fp);    
	return 0;
}

int set_sys_desc(char *desc)
{
	FILE * fp=NULL;
	fp = fopen(SYSTEM_DESC_FILE,"w");
	if(!fp)
	{
		return -1;
	}
	fputs(desc,fp);
	fclose(fp);
	return 0;
}

void get_sys_contact(char *contact)
{
    FILE *fp=fopen(SYS_CONTACT_FILE,"r");
    if(!fp)
    {
        memcpy(contact,"undefined",strlen("undefined"));
    }
    else
    {
        fgets(contact,256,fp);
        fclose(fp);
    }
    return;
    
}

int set_sys_contact(char *contact)
{
    FILE *fp=fopen(SYS_CONTACT_FILE,"w");
    if(!fp)
    {
        printf("%%Cannot open file %s",SYS_CONTACT_FILE);
        return -1;
    }
    else
    {
        fputs(contact,fp);
        fclose(fp);
    }
    return 0;
}

int set_sys_location(char *location)
{
	FILE* fp=NULL;
	char ptr[SYS_LOCATION_STR_LEN];

	if(!location)
		return -1;
	memset(ptr,0,SYS_LOCATION_STR_LEN);
	fp = fopen(SYS_LOCATION_CONFIG_FILE,"w");
	if(!fp)
		return -1;
	
	sprintf(ptr,"%s%s\n",SYS_LOCATION_PREFIX,location);
	fputs(ptr,fp);
	fclose(fp);
	return 0;
}

int get_cli_syslog_str(void)
{
    FILE *fp=NULL;
    char *ptr=NULL;
    int ret=0;
    fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"r");

    if(!fp)
        return 0;

    ptr=malloc(8);
    memset(ptr,0,8);

    if(!ptr)
    {
        fclose(fp);
        return 0;
    }

    fgets(ptr,8,fp);
    ret = atoi(ptr);
    free(ptr);
    fclose(fp);
    return ret;
}

int set_offset_time(int offset_time)
{
	FILE *fp;
	char  buffer[100];
	char* tmpbuf;
	int ret =0;

	fp = fopen("/var/run/offset_time","w");
	if(NULL == fp)
	{
		return -1;
	}
	fprintf(fp,"%d",offset_time);

	fclose(fp);
	return ret;
	
}

void get_syslog_dest_name(char *dest_name, char *addr)
{
    int i=0;
    int index = 0;
    for(i=0;i< strlen(addr);i++)
    {
        index+=addr[i];
    }
    sprintf(dest_name,"srv_%d",index);    
}

int set_ripd_socket_buffer(unsigned int entry)
{
    FILE* confp=NULL;
	char* buf = NULL;
    char tmp[128];
    int ret=-1,buflen=0;
    memset(tmp,0,128);
    confp = fopen(RIPD_SOCKET_BUF_FILE,"w");

    if(!confp)
    {
        fprintf(stdout,"Can't open %s file\n",RIPD_SOCKET_BUF_FILE);
        return ret;
    }

    buflen = entry*42;/*42=41600/(40*25)*/
    sprintf(tmp,"%s%d\n",RIPD_MAX_SOCKET_BUF,buflen);
    ret = fputs(tmp,confp);

    if(EOF == ret)
    {
        fprintf(stdout,"Write file %s error\n",RIPD_SOCKET_BUF_FILE);
        return -1;
    }

    fclose(confp);
    return ret;
}

#endif

int get_offset_time(char *buffer)
{
    int ret = -1;
	FILE *fp;
	fp = fopen("/var/run/offset_time","r");
	if(NULL == fp)
	{
		return ret;
	}
	if(!fgets(buffer,OFFSET_BUFFER_SIZE,fp))
	{
		command_sync_printf("got offset_time failed \n");
        ret = -1;
	}
	fclose(fp);
	return ret;
}



DECLARE_CONFIG_HANDLER(net_element_func)
{
    char* net_element;
    int ret =0;	
	net_element = argv_concat(argv, argc, 0);
    if(!net_element)
    {
        return 0;
    }
	if (strlen(net_element) <= 0)
	{
		return 0;
	}	
	ret = set_net_element(net_element);
    if(ret)
    {
        command_sync_printf("set net_element failed \n");
        free(net_element);
        return -1;
    }
    free(net_element);
    command_sync_printf("set net_element success \n");
	return 0;
}
DECLARE_CONFIG_HANDLER(sys_location_func)
{
    char *location;
    int ret =0;	
	location = argv_concat(argv, argc, 0);
    if(!location)
    {
        return 0;
    }
	if (strlen(location) <= 0)
	{
		return 0;
	}
	ret = set_sys_location(location);
    if(ret)
    {
        command_sync_printf("set sys_location failed \n");
        free(location);
        return -1;
    } 
    free(location);
    command_sync_printf("set sys_location success \n");
    return 0;
}
DECLARE_CONFIG_HANDLER(sys_time_func)
{
	char cmdbuf[CMD_SIZE];
    if(argc == 2)
    {
        sprintf(cmdbuf,"%s %s %s",SET_TIME_SCRIPT,argv[0],argv[1]);
    }
    if(argc == 3)
    {
        sprintf(cmdbuf,"%s %s %s %s",SET_TIME_SCRIPT,argv[0],argv[1], argv[2]);
    }
    if(!system(cmdbuf))
    {
        command_sync_printf("set sys_time success \n");
        return 0;
    }
    else
    {
        command_sync_printf("set sys_time error \n");
        return -1;
    }
   
}


DECLARE_CONFIG_HANDLER(offset_func)
{
	int offset_time = atoi(argv[0]);
	if(offset_time >= 0 && set_offset_time(offset_time) == 0)
	{
		command_sync_printf("Set offset time success\n");
        return 0;
	}
	else
	{
		command_sync_printf("Set offset time %d error\n",offset_time);
        return -1;
	}	
}

#ifdef HAVE_WEBMNG

DECLARE_CONFIG_HANDLER(lighttpd_proxy_mode_func)
{
    char cmd[64];
    if(!argc)
    {
        return 0;
    }
    if(!SYS_LOCAL_MODULE_ISMASTER)
    {
        return 0;
    }
    sprintf(cmd,"%s %s",LIGHTTPD_PROXY_MODE,argv[0]);
    system(cmd);
    system(RESTART_LIGHTTPD_CMD);
	return 0;
}

DECLARE_CONFIG_HANDLER(add_lighttpd_proxy_ip_func)
{
    char cmd[64];
    if(!argc)
    {
        return 0;
    }
    if(!SYS_LOCAL_MODULE_ISMASTER)
    {
        return 0;
    }
    sprintf(cmd,"%s %s %s",ADD_LIGHTTPD_PROXY_SCRIPT,argv[0],argv[1]);
    system(cmd);
    system(SHOW_LIGHTTPD_PROXY_SCRIPT);
    system(RESTART_LIGHTTPD_CMD);
	return 0;
}

DECLARE_CONFIG_HANDLER(add_lighttpd_proxy_tipc_func)
{
    char cmd[64];
    if(!argc)
    {
        return 0;
    }
    if(!SYS_LOCAL_MODULE_ISMASTER)
    {
        return 0;
    }
    sprintf(cmd,"%s %s %s",ADD_LIGHTTPD_PROXY_TIPC_SCRIPT,argv[0],argv[1]);
    system(cmd);
    system(SHOW_LIGHTTPD_PROXY_SCRIPT);
    system(RESTART_LIGHTTPD_CMD);
	return 0;
}

DECLARE_CONFIG_HANDLER(del_lighttpd_proxy_path_func)
{
    char cmd[64];
    if(!argc)
    {
        return 0;
    }
    if(!SYS_LOCAL_MODULE_ISMASTER)
    {
        return 0;
    }
    sprintf(cmd,"%s %s",DEL_LIGHTTPD_PROXY_SCRIPT,argv[0]);
    system(cmd);
    system(SHOW_LIGHTTPD_PROXY_SCRIPT);
    system(RESTART_LIGHTTPD_CMD);
	return 0;
}

DECLARE_CONFIG_HANDLER(lighttpd_backend_tcp_func)
{
    char cmd[64];
    sprintf(cmd,"%s tcp",LIGHTTPD_BACKEND_MODE);
    system(cmd);
    system(RESTART_LIGHTTPD_CMD);
	return 0;
}

DECLARE_CONFIG_HANDLER(lighttpd_backend_tipc_func)
{
    char cmd[64];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s %s",LIGHTTPD_BACKEND_MODE,argv[0]);
    system(cmd);
    system(RESTART_LIGHTTPD_CMD);
	return 0;
}

int is_tipc_addr(char *addr)
{
    int t=atoi(addr);
    if(t>=0x1001001)
    {
        return t&0xFFF;
    }
    else
    {
        return 0;
    }
}

int backup_webmng_config()
{
    int ret =-1;
    char path[64];
    char addr[32];
    char type[2];
    char mode[16];
    char para[64];
    char proxy_mod[8];
    int t=0;
    FILE *fp=NULL;
    fp=fopen(PROXY_PATH_TMP_FILE,"r");
    if(fp)
    {
        memset(path, 0, 64);
        fscanf(fp,"%s",path);
        ret =strlen(path);
        while(ret)
        {
            memset(addr, 0, 32);
            fscanf(fp,"%s",addr);
            t=is_tipc_addr(addr);
            if(!t)
            {
                memset(para,0,64);
                sprintf(para,"%s %s",path,addr);
                ret=config_item(path, 2, para, add_lighttpd_proxy_ip_func, NULL);
            }          
            memset(path, 0, 64);
            fscanf(fp,"%s",path);
            ret =strlen(path);
        }
        fclose(fp);
    }

    fp=fopen(PROXY_TYPE_FILE,"r");
    memset(proxy_mod,0,8);
    if(fp)
    {
         memset(type, 0, 2);
         fscanf(fp,"%s",type);
         fclose(fp);
         t=atoi(type);
         if(t)
         {
            memcpy(proxy_mod,"tipc",strlen("tipc"));
         }
         else
         {
            memcpy(proxy_mod,"tcp",strlen("tcp"));
         }
    }
    else
    {
        memcpy(proxy_mod,"tcp",strlen("tcp"));
    }
    ret=config_item(LIGHT_PROXY_MODE, 1, proxy_mod, lighttpd_proxy_mode_func, NULL);

    fp=fopen(BACKEND_MODE_TMP_FILE,"r");
    ret=config_item(LIGHT_BACKEND_TIPC, 0, NULL, lighttpd_backend_tipc_func, NULL);
    ret=config_item(LIGHT_BACKEND_TCP, 0, NULL, lighttpd_backend_tcp_func, NULL);
    if(fp)
    {
         memset(mode, 0, 16);
         fscanf(fp,"%s",mode);
         fclose(fp);
         if(strncmp(mode,"tcp",strlen("tcp")))
         {
            ret=config_item(LIGHT_BACKEND_TIPC, 1, mode, lighttpd_backend_tipc_func, NULL);
         }      
    } 
    config_item(ADD_LIGHT_PROXY_IP, 0, NULL, add_lighttpd_proxy_ip_func, NULL);
    config_item(ADD_LIGHT_PROXY_TIPC, 0, NULL, add_lighttpd_proxy_tipc_func, NULL);
    config_item(DEL_LIGHT_PROXY_PATH, 0, NULL, del_lighttpd_proxy_path_func, NULL);
    return 0;
}
#endif
#ifdef HAVE_SYSLOG

DECLARE_CONFIG_HANDLER(add_syslog_server_func)
{
    char cmd[256];
    char dest_name[DEST_NAME_SIZE];
    int port =514;
    if(!argc)
    {
        return 0;
    }
    if(argc==2)
    {
        port =atoi(argv[1]);
    }
    memset(dest_name,0,DEST_NAME_SIZE);
    get_syslog_dest_name(dest_name,argv[0]);       
    sprintf(cmd,"%s %s udp %s %d",ADD_SERVER_SCRIPT,dest_name,argv[0],port);
    system(cmd); 
    memset(cmd,0,256);
    sprintf(cmd, "%s",SHOW_SERVER_SCRIPT);
    system(cmd);
    system(SYSLOG_RESTART_CMD);   
    return 0;
}
DECLARE_CONFIG_HANDLER(del_syslog_server_func)
{
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s udp %s",DEL_SERVER_SCRIPT,argv[0]);
    system(cmd);
    memset(cmd,0,256);
    sprintf(cmd, "%s",SHOW_SERVER_SCRIPT);
    system(cmd);
    memset(cmd,0,256);
    sprintf(cmd, "%s",SHOW_STATE_SCRIPT);
    system(cmd);
    system(SYSLOG_RESTART_CMD);    
    return 0;
}
DECLARE_CONFIG_HANDLER(control_syslog_server_func)
{
    char cmd[256];
	int op =0;
    if(!argc)
    {
        return 0;
    }
    if (!strncmp(argv[0], "enable", strlen(argv[0])))
	{
		op =1;
	}
    if(op)
    {
        system(ENABLE_SERVER_SCRIPT);
    }
    else
    {
        system(DISABLE_SERVER_SCRIPT);
    }
    memset(cmd,0,256);
    sprintf(cmd, "%s",SHOW_STATE_SCRIPT);
    system(cmd);
    system(SYSLOG_RESTART_CMD);    
    return 0;
}
DECLARE_CONFIG_HANDLER(config_dest_func)
{
    char dest[30];
    char cmd[256];
	int op =0;
    if(!argc)
    {
        return 0;
    }
    if (!strncmp(argv[0], "enable", strlen(argv[0])))
	{
		op =1;
	}  
    memset(dest, 0, 10);
		memcpy(dest,"d_cons",strlen("d_cons"));
      
		if(op)
        {
        sprintf(cmd,"%s %s",ENABLE_DEST_SCRIPT,dest);
        }
        else
        {
        sprintf(cmd,"%s %s",DISABLE_DEST_SCRIPT,dest);
	}   
    system(cmd);
    memset(cmd,0,256);
    sprintf(cmd, "%s",SHOW_STATE_SCRIPT);
    system(cmd);
    system(SYSLOG_RESTART_CMD);    
    return 0;
}
DECLARE_CONFIG_HANDLER(add_module_func)
{
    char filter_name[20];
    char cmd[256];
    int ret = -1;
    if(!argc)
    {
        return 0;
    }
    memset(filter_name, 0, 20);
    if (!strncmp(argv[0], "all", strlen(argv[0])))
    {
       
        memcpy(filter_name,"all",strlen("all"));        
    }
    else
        {
	    sprintf(filter_name,"f_%s",argv[0]);
    }
    
    memset(cmd,0,256);
    sprintf(cmd,"%s %s",ADD_MODULE_SCRIPT,filter_name);
    system(cmd);
    memset(cmd,0,256);
    if (!strncmp(argv[0], "all", strlen(argv[0])))
    {           
        sprintf(cmd, "%s all",SHOW_MODULE_SCRIPT);       
    }
    else
    {
        sprintf(cmd, "%s",SHOW_MODULE_SCRIPT);
    }
    system(cmd);
	system(SYSLOG_RESTART_CMD);    
    return 0;
}
DECLARE_CONFIG_HANDLER(del_module_func)
{
    char filter_name[20];
    char cmd[256];
    int ret = -1;
    if(!argc)
    {
        return 0;
    }
        memset(filter_name, 0, 20);
    if (!strncmp(argv[0], "all", strlen(argv[0])))
    {           
        memcpy(filter_name,"all",strlen("all"));        
    }
    else
    {
    sprintf(filter_name,"f_%s",argv[0]);
    }
    sprintf(cmd,"%s %s",DEL_MODULE_SCRIPT,filter_name);
    system(cmd);
    if (strncmp(argv[0], "all", strlen(argv[0])))
    {           
        memset(cmd,0,256);
        sprintf(cmd, "%s",SHOW_MODULE_SCRIPT);
        system(cmd);
    }    
	system(SYSLOG_RESTART_CMD);  
    return 0;
}
DECLARE_CONFIG_HANDLER(set_level_func)
{
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s %s %s",SET_LEVEL_SCRIPT,argv[1],argv[0]);
    system(cmd);
    memset(cmd,0,256);
    sprintf(cmd, "%s",SHOW_LEVEL_SCRIPT);
    system(cmd);
    system(SYSLOG_RESTART_CMD);    
    return 0;
}

DECLARE_CONFIG_HANDLER(set_local_udp_port_func)
{
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s %s",SET_SYSLOG_LOCAL_PORT,argv[0]);
    system(cmd);
    memset(cmd,0,256);
    sprintf(cmd, "%s",SHOW_SYSLOG_LOCAL_PORT);
    system(cmd);
    system(SYSLOG_RESTART_CMD);    
    return 0;
}

int init_syslog_config()
{
    system(SHOW_LEVEL_SCRIPT);
    system(SHOW_MODULE_SCRIPT);
    system(SHOW_SYSLOG_LOCAL_PORT);
    system(SHOW_SERVER_SCRIPT);  
    system(SHOW_STATE_SCRIPT);   
    return 0;
}
int backup_syslog_config()
{
    int ret = -1;
    char cmd[256];
    char para1[10];
    char para2[20];
    char para3[10];
    char state[10];
    char udp_port[10];
    int udp_p;
    char server_item[30];
    FILE *fp = NULL;
    init_syslog_config();
    ret= get_syslog_local_port();
    if(ret==-1)
    {
        command_sync_printf("%% Get syslog local udp port failed\n");
        udp_p=514;
    }
    else
    {
        udp_p=ret;
    }
    sprintf(udp_port,"%d",udp_p);
    config_item(LOCAL_UDP_PORT, 1, udp_port, set_local_udp_port_func, NULL);
    fp = fopen(LEVEL_TMP_FILE,"r");
    if(!fp)
    {
        command_sync_printf("%% Open file:%s failed\n",LEVEL_TMP_FILE);
        return -1;
    }
    memset(para1, 0, 10);
    fscanf(fp,"%s",para1);
    ret =strlen(para1);
    if(ret == 0)
    {
        command_sync_printf("%% Cannot get server level \n");
        config_item(SERVER_LEVEL, 0, NULL, set_level_func, NULL);
    }
    sprintf(para2,"server %s",para1);
    config_item(SERVER_LEVEL, 0, para2, set_level_func, NULL);
    memset(para1, 0, 10);
    fscanf(fp,"%s",para1);
    ret =strlen(para1);
    if(ret == 0)
    {
        command_sync_printf("%% Cannot get telnet level \n");
        config_item(TELNET_LEVEL, 0, NULL, set_level_func, NULL);
    }
    memset(para2,0,20);
    sprintf(para2,"telnet %s",para1);
    config_item(TELNET_LEVEL, 0, para2, set_level_func, NULL);
    memset(para1, 0, 10);
    fscanf(fp,"%s",para1);
    ret =strlen(para1);
    if(ret == 0)
    {
        command_sync_printf("%% Cannot get console level \n");
        config_item(CONSOLE_LEVEL, 0, NULL, set_level_func, NULL);
    }
    memset(para2,0,20);
    sprintf(para2,"console %s",para1);
    config_item(CONSOLE_LEVEL, 0, para2, set_level_func, NULL);
    fclose(fp);
    /* back up servers */   
    fp = fopen(CONFIG_TMP_FILE,"r");
    if(!fp)
    {
        command_sync_printf("%% Open file:%s failed\n",CONFIG_TMP_FILE);
        return -1;
    }
    memset(para1, 0, 10);
    fscanf(fp,"%s",para1);
    ret =strlen(para1);
    if(ret == 0)
    {
        command_sync_printf("No server configured\n");
    }
    while(ret)
    {
        memset(para2,0,20);
        fscanf(fp,"%s",para2);
        fscanf(fp,"%s",para3);
        memset(server_item,0,30);
        sprintf(server_item,"%s%s",SYSLOG_EXTRA,para2);
        config_item(server_item, 1, para2, add_syslog_server_func, NULL);
        memset(para1, 0, 10);
        fscanf(fp,"%s",para1);
        ret =strlen(para1);
    }
    fclose(fp);
    /* back up modules */
    fp = fopen(MODULE_TMP_FILE,"r");
    if(!fp)
    {
        command_sync_printf("%% Open file:%s failed\n",MODULE_TMP_FILE);
        return -1;
    }
    memset(para1, 0, 10);
    fscanf(fp,"%s",para1);
    ret =strlen(para1);
    if(ret == 0)
    {
        command_sync_printf("No module configured\n");
    }
    while(ret)
    {
        config_item(para1, 1, para1, add_module_func, NULL);
        memset(para1, 0, 10);
        fscanf(fp,"%s",para1);
        ret =strlen(para1);
    }
    fclose(fp);
    /* back up the state of destinations */
    fp =fopen(STATE_TMP_FILE,"r");
    if(!fp)
    {
        command_sync_printf("%% Open file:%s failed\n",STATE_TMP_FILE);
        return -1;
    }
    /* read the console */
    memset(state, 0, 10);
    fscanf(fp,"%s",state);
    ret =strlen(state);
    if(ret == 0)
    {
        command_sync_printf("%% Cannot get the state of destination\n");
        return -1;
    }
    config_item(CONFIG_DEST, 1, state, config_dest_func, NULL);
    /* read the server*/
    memset(state, 0, 10);
    fscanf(fp,"%s",state);
    ret =strlen(state);
    if(ret == 0)
    {
        command_sync_printf("%% Cannot get the state of destination\n");
        return -1;
    }
    config_item(CONTROL_SERVER, 1, state, control_syslog_server_func, NULL);
    fclose(fp);

    config_item(ADD_SERVER, 0, NULL, add_syslog_server_func, NULL);
    config_item(ADD_MODULE, 0, NULL, add_module_func, NULL);
    config_item(DEL_MODULE, 0, NULL, del_module_func, NULL);
    config_item(DEL_SERVER, 0, NULL, del_syslog_server_func, NULL);
    return 0;
}
#endif

#ifdef HAVE_NTP


/* ntp call back functions */

DECLARE_CONFIG_HANDLER(add_ntp_server_func)
{
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
	
    memset(cmd, 0, 256);
    sprintf(cmd, "%s %s", NTP_ADD_SERVER_SCRIPT, argv[0]);
    system(cmd); 
	
    memset(cmd, 0, 256);
    sprintf(cmd, NTP_SHOW_SERVER_SCRIPT);
    system(cmd);
	
	memset(cmd, 0, 256);
	sprintf(cmd, NTP_RESTART_CMD);
    system(cmd);   
    return 0;
}

DECLARE_CONFIG_HANDLER(del_ntp_server_func)
{
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd, "%s %s", NTP_DEL_SERVER_SCRIPT, argv[0]);
    system(cmd); 
	
    memset(cmd, 0, 256);
    sprintf(cmd, NTP_SHOW_SERVER_SCRIPT);
    system(cmd);
	
	memset(cmd, 0, 256);
	sprintf(cmd, NTP_RESTART_CMD);
    system(cmd);   
    return 0;
}

DECLARE_CONFIG_HANDLER(ntp_server_mode_func)
{
    char cmd[256];  

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "sudo echo 1 > /var/run/ntp_server.tmp");
    system(cmd); 
	
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, NTP_SERVER_MODE_SCRIPT);
    system(cmd); 
	
    memset(cmd,0,256);
    sprintf(cmd, NTP_SHOW_SERVER_SCRIPT);
    system(cmd);
  
    return 0;
}



int back_up_ntp_servers()
{
    char addr[20];
	char item[32] = {0};
    int ret = 0;
	FILE *fp = NULL;

    ret = config_item(ADD_NTP_SERVER, 0, NULL, add_ntp_server_func, NULL);
    ret = config_item(DEL_NTP_SERVER, 0, NULL, del_ntp_server_func, NULL);
    ret = config_item(NTP_SERVER_MODE, 0, NULL, ntp_server_mode_func, NULL);
    fp = fopen(NTP_TMP_FILE, "r");
    if(!fp)
    {
        //printf("%% Open file:%s failed\n", NTP_TMP_FILE);
        return -1;
    }	
	memset(addr, 0, 20);
    fscanf(fp, "%s", addr);
	ret = strlen(addr);
    while(ret)
    {        
        memset(item, 0, 32);
        sprintf(item,"%s%s", NTP_EXTRA, addr);
        if(config_item(item, 1, addr, add_ntp_server_func, NULL))
        {
            fclose(fp);
            command_sync_printf("add NTP server failed \n");
            return -1;
        }
		memset(addr, 0, 20);
        fscanf(fp, "%s", addr);
        ret = strlen(addr);
    }
    fclose(fp);
    return 0;


}
#endif


DECLARE_CONFIG_HANDLER(set_host_name_func)
{
    char cmd[64];
    if(!argc)
    {
        system("hostname.sh SYSTEM");
    }
    else
    {
        sprintf(cmd,"hostname.sh %s",argv[0]);
        system(cmd);    
    }
    command_sync_printf("set hostname success \n");
     return 0;
}
DECLARE_CONFIG_HANDLER(set_dns_server_func)
{
    char *dnsstr[MAX_DNS_SERVER];
    int ret,i;
    if(!argc)
    {
        return 0;
    }
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        dnsstr[i] = malloc(128);
        if(!dnsstr[i])
        {
            goto ret_err;
        }
        memset(dnsstr[i],0,128);
    }
    ret = get_dns_str(&dnsstr);
    if(ret<0)
    {
        command_sync_printf("Can't get system dns setting\n");
        goto ret_err;
    }
    else if(ret >= MAX_DNS_SERVER)
    {
        command_sync_printf("The system has %d dns,can't set again\n",MAX_DNS_SERVER);
        goto ret_err;
    }
    else
    {
        int i;
        for(i=0; i<ret; i++)
        {
            if(!strncmp(argv[0],dnsstr[i],strlen(argv[0])))
            {
                break;
            }
        }
        if(i==ret)
        {
            sprintf(dnsstr[ret],(char*)argv[0]);
        }
        else
        {
            command_sync_printf("The dns server %s is exist,can't set again\n",argv[0]);
            goto ret_err;
        }
        if(set_dns_str(&dnsstr,ret+1))
        {
            command_sync_printf("Set system dns error\n");
            goto ret_err;
        }
    }
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        if(dnsstr[i])
        {
            free(dnsstr[i]);
        }
    }
    command_sync_printf("set dns success \n");
    return ret;
ret_err:
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        if(dnsstr[i])
        {
            free(dnsstr[i]);
        }
    }
    return -1;
}
DECLARE_CONFIG_HANDLER(no_dns_server_func)
{
    char *dnsstr[MAX_DNS_SERVER];
    int ret,i;
    if(!argc)
    {
        return 0;
    }
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        dnsstr[i] = malloc(128);
        if(!dnsstr[i])
        {
            goto ret_err;
        }
        memset(dnsstr[i],0,128);
    }
    ret = get_dns_str(&dnsstr);
    if(ret<0 || ret > MAX_DNS_SERVER)
    {
        command_sync_printf("Can't get system dns seting\n");
        goto ret_err;
    }
    else
    {
        int i=0;
        for(i=0; i<ret; i++)
        {
            if(!strncmp(argv[0],dnsstr[i],strlen(argv[0])))
            {
                int j ;
                if(i<ret-1)
                {
                    for(j=i; j<ret-1; j++)
                    {
                        sprintf(dnsstr[j],dnsstr[j+1]);
                    }
                }
                break;
            }
        }
        if(i >= ret)
        {
            command_sync_printf("Can't get the dns %s\n",argv[0]);
            goto ret_err;
        }
        if(set_dns_str(&dnsstr,ret-1))
        {
            command_sync_printf("Delete system dns error\n");
            goto ret_err;
        }
    }
    command_sync_printf("delete dns success \n");
    return ret;
ret_err:
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        if(dnsstr[i])
            free(dnsstr[i]);
    }
    return -1;
}
#ifdef HAVE_RIP
DECLARE_CONFIG_HANDLER(rip_buffer_func)
{
    int ret= 0;
    int max_rip_entry  = atoi(argv[0]);   
    ret=set_ripd_socket_buffer(max_rip_entry);
    if(ret==-1)
    {
        command_sync_printf("Set entry value is error\n");
        return ret;
    }    
    command_sync_printf("set rip buffer success \n");
    return 0;
}
#endif
DECLARE_CONFIG_HANDLER(set_idle_time_func)
{
    int ret = 0;
    int idle_time =0;
    if(!argc)
    {
        ret = set_idle_timeout(IDLE_TIME_DEFAULT);
    }
    else
    {
        idle_time   = atoi(argv[0]);
        ret = set_idle_timeout(idle_time);
    }
    if(ret)
    {
        command_sync_printf("Set idle timeout value ERROR!\n");
    }
    else
    {
        command_sync_printf("set idle time success \n");
    }
    return ret;
}
DECLARE_CONFIG_HANDLER(erase_memory_func)
{
    char cmd[64];
    if(!extra)
    {
        return 0;
    }
    memset(cmd,0,64);
    sprintf(cmd,"sudo earse.sh %s \n",extra);
    system(cmd);
    return 0;
}

DECLARE_CONFIG_HANDLER(set_sysdesc_func)
{ 
    int ret = 0;
    char *sys_desc=argv_concat(argv, argc, 0);
    if(!argc||!sys_desc)
    {
        ret = set_sys_desc("Undefined");
    }
    else
    {
        ret = set_sys_desc(sys_desc);
    }    
    if(ret)
    {
        command_sync_printf("set system description failed \n");
        free(sys_desc);
        return -1;
    } 
    free(sys_desc);
    command_sync_printf("set system description success \n");
    return 0;
}

DECLARE_CONFIG_HANDLER(set_sys_contact_func)
{ 
    int ret = 0;
    char *sys_contact=argv_concat(argv, argc, 0);
    if(!argc||!sys_contact)
    {
        ret = set_sys_contact("Undefined");
    }
    else
    {
        ret = set_sys_contact(sys_contact);
    }    
    if(ret)
    {
        command_sync_printf("set system contact failed \n");
        free(sys_contact);
        return -1;
    } 
    free(sys_contact);
    command_sync_printf("set system contact success \n");
    return 0;
}

DECLARE_CONFIG_HANDLER(cli_log_func)
{
    int ret = 0;
    int status = 0;
    if(!strncmp(argv[0],"on",2))
    {
        status = 1;
    }
    ret = set_cli_syslog_str(status);
    if(ret)
    {
        command_sync_printf("set cli log failed \n");
    }
    else
    {
        command_sync_printf("set cli log success \n");
    }
    return ret;
}

int get_console_pwd(char *pwd_str)
{
    FILE * fp=NULL;
	fp = fopen(CONSOLEPWDFILE,"r");
	if(!fp)
	{
		return -1;
	}
	fputs(pwd_str,fp);
	fclose(fp);
	return 0;
}


DECLARE_CONFIG_HANDLER(set_console_pwd_func)
{
    return 0;
	char cmd[128];
    if(!argc)
    {
        sprintf(cmd,"sudo rm %s",CONSOLEPWDFILE);
    }
    else
    {
	sprintf(cmd,"sudo set_console_pwd.sh \'%s\' %s",argv[0],extra);
    }
	system(cmd);
    command_sync_printf("set console pwd success \n");
	return 0;
}


DECLARE_CONFIG_HANDLER(change_pwd_func)
{
    int ret = 0;
	char command[64];
    if(!argc)
    {
        return 0;
    }
	sprintf(command,"chpass.sh %s \'%s\' %s",argv[0],argv[1],extra);
	ret = execute_dcli_shell(command);
	if(ret != 0)
	{
		command_sync_printf("Change user %s password error\n",argv[0]);
	}
	return ret;
}
DECLARE_CONFIG_HANDLER(change_role_func)
{
    int ret = 0;
	char command[64];
	char userrole[8];
    if(!argc)
    {
        return 0;
    }
	if(!strncmp("enable",argv[1],strlen(argv[1])))
	{
		sprintf(userrole,"enable");
	}
	else
	{
		sprintf(userrole,"view");
	}
	sprintf(command,"userrole.sh %s %s",argv[0],userrole);
    ret = execute_dcli_shell(command);
	if(ret != 0)
    {
		command_sync_printf("Change user %s role error\n",argv[0]);
	}
	return ret;
}
DECLARE_CONFIG_HANDLER(add_user_func)
{
    int ret =0;
	char userrole[32];
    if(!argc)
    {
        return 0;
    }
    if(is_user_exsit(argv[0]))		
	{
    	char command[64];
		command_sync_printf("the user %s exist already, will change the password.\n",argv[0]);
		
    	sprintf(command,"chpass.sh %s \'%s\' %s",argv[0],argv[1],extra);
    	ret = execute_dcli_shell(command);
    	if(ret != 0)
    	{
    		command_sync_printf("Change user %s password error\n",argv[0]);
    	}
    	return ret;
    }
	if(!strncmp("enable",argv[2],strlen(argv[2])))
	{
		sprintf(userrole,"enable");
	}
	else
	{
		sprintf(userrole,"view");
	}
	ret = dcli_user_add_sh(argv[0],argv[1],userrole,extra);
	if(0 != ret)
	{
		command_sync_printf("Add user %s error\n",argv[0]);
	}
	return ret;
}
DECLARE_CONFIG_HANDLER(del_user_func)
{
    int ret = 0;
	char command[64];
    if(!argc)
    {
        return 0;
    }
	sprintf(command,"userdel.sh %s",argv[0]);
	ret = execute_dcli_shell(command);
	if(ret != 0)
    {
		command_sync_printf("Delete user %s error\n",argv[0]);
	}
	return ret;
}

#ifdef HAVE_PAM
DECLARE_CONFIG_HANDLER(config_pam_func)
{
    int ret =0;
    char cmd[256];
    char module[10];
    if(!argc)
    {
        return 0;
    }
    memset(module,0,10);
    if(!strncmp("radius",argv[0],strlen(argv[0])))
	{
        system(ENABLE_RADIUS_SCRIPT);
        system(DISABLE_TACPLUS_SCRIPT);
	}
    if(!strncmp("tacplus",argv[0],strlen(argv[0])))
    {
		 system(ENABLE_TACPLUS_SCRIPT);
         system(DISABLE_RADIUS_SCRIPT);
    }
    if(!strncmp("local",argv[0],strlen(argv[0])))
	{
		 system(DISABLE_RADIUS_SCRIPT);
         system(DISABLE_TACPLUS_SCRIPT);
	}
    system(SHOW_PAM_SCRIPT); 
	return ret;
}
DECLARE_CONFIG_HANDLER(add_radius_func)
{
    int ret =0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s %s %s",ADD_RADSERVER_SCRIPT,argv[0],argv[1]);
    system(cmd);
    system(SHOW_RADIUS_SCRIPT);
	return ret;
}
DECLARE_CONFIG_HANDLER(del_radius_func)
{
    int ret =0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s %s",DEL_RADSERVER_SCRIPT,argv[0]);
    system(cmd);
    system(SHOW_RADIUS_SCRIPT);
	return ret;
}
DECLARE_CONFIG_HANDLER(add_tacplus_func)
{
    int ret =0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s %s",ADD_TACSERVER_SCRIPT,argv[0]);
    system(cmd);
    system(SHOW_TACPLUS_SCRIPT);
	return ret;
}
DECLARE_CONFIG_HANDLER(del_tacplus_func)
{
    int ret =0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    sprintf(cmd,"%s %s",DEL_TACSERVER_SCRIPT,argv[0]);
    system(cmd);
    system(SHOW_TACPLUS_SCRIPT);
	return ret;
}
int backup_pam_config()
{
    char rad_state[10];
    char tac_state[10];
    char para[30];
    char addr[20];
    char key[20];
    char server_item[30];
    FILE *fp=NULL;
    int ret =0;
    memset(rad_state,0,10);
    memset(tac_state,0,10);
    system(SHOW_PAM_SCRIPT);
    get_pam_status(rad_state, tac_state);
	if(!strcmp(rad_state,"enable"))
	{
		config_item("user-auth", 1, "radius", config_pam_func, NULL);
	}
	else if(!strcmp(tac_state,"enable"))
	{
		config_item("user-auth", 1, "tacplus", config_pam_func, NULL);
	}
	else
	{
		config_item("user-auth", 1, "local", config_pam_func, NULL);
	}    
    system(SHOW_RADIUS_SCRIPT);
    fp = fopen(RADIUS_ADDR_FILE,"r");
    if(!fp)
    {
        command_sync_printf("%% Open file %s faile\r\n",RADIUS_ADDR_FILE);
        return -1;
    }
    memset(addr,0,20);
    fscanf(fp,"%s",addr);
    while(strlen(addr))
    {
        memset(key,0,20);
        fscanf(fp,"%s",key);
        memset(para,0,30);
        sprintf(para,"%s %s",addr,key);
        memset(server_item,0,30);
        sprintf(server_item,"%s%s",RADIUS_EXTRA,addr);
        config_item(server_item, 2, para, add_radius_func, NULL);
        memset(addr,0,20);
        fscanf(fp,"%s",addr);
    }
    fclose(fp);
    /* tacplus client configuration */
    system(SHOW_TACPLUS_SCRIPT);
    fp= fopen(TACPLUS_CLIENT_FILE,"r");
    if(!fp)
    {
        command_sync_printf("%% Open file %s faile\r\n",TACPLUS_CLIENT_FILE);
        return -1;
    }
    memset(addr,0,20);
    fscanf(fp,"%s",addr);
    while(strlen(addr))
    {
        memset(server_item,0,30);
        sprintf(server_item,"%s%s",TACPLUS_EXTRA,addr);
        config_item(server_item, 1, addr, add_tacplus_func, NULL);
        memset(addr,0,20);
        fscanf(fp,"%s",addr);
    }
    fclose(fp);   
    ret = config_item(CONFIG_PAM, 0, NULL, config_pam_func, NULL);
    ret = config_item(ADD_RADIUS, 0, NULL, add_radius_func, NULL);
    ret = config_item(ADD_TACPLUS, 0, NULL, add_tacplus_func, NULL);
    ret = config_item(DEL_RADIUS, 0, NULL, del_radius_func, NULL);
    ret = config_item(DEL_TACPLUS, 0, NULL, del_tacplus_func, NULL);
    return 0;
    
}
int init_pam_config()
{
    system(SHOW_PAM_SCRIPT);
    system(SHOW_RADIUS_SCRIPT);
    system(SHOW_TACPLUS_SCRIPT);
    return 0;
}

#endif
#ifdef HAVE_LOGIN_FILTER
DECLARE_CONFIG_HANDLER(set_telnetd_filter_func)
{
#if 0
    int ret = 0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    printf("set_telnetd_filter_func=======%s, %s\n", argv[0], argv[1]);
    if(!strncmp(argv[0],"permit",strlen(argv[0])))
    {
        sprintf(cmd,"%s in.telnetd %s permit",LOGIN_FILTER_SCRIPT,argv[1]);
    }
    else
    {
        sprintf(cmd,"%s in.telnetd %s deny",LOGIN_FILTER_SCRIPT,argv[1]);
    }
    system(cmd);
    return ret;
#endif
    char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo add_iptables_rules.sh telnet %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;
}
DECLARE_CONFIG_HANDLER(del_telnetd_filter_func)
{
#if 0
    int ret = 0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    printf("del_telnetd_filter_func=======%s, %s\n", argv[0], argv[1]);
    if(!strncmp(argv[0],"permit",strlen(argv[0])))
    {
        sprintf(cmd,"%s in.telnetd %s permit",DEL_LOGIN_FILTER_SCRIPT,argv[1]);
    }
    else
    {
        sprintf(cmd,"%s in.telnetd %s deny",DEL_LOGIN_FILTER_SCRIPT,argv[1]);
    }
    system(cmd);
    return ret;
#endif
	char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo del_iptables_rules.sh telnet %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;
}
DECLARE_CONFIG_HANDLER(set_sshd_filter_func)
{
#if 0
    int ret = 0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    if(!strncmp(argv[0],"permit",strlen(argv[0])))
    {
        sprintf(cmd,"%s sshd %s permit",LOGIN_FILTER_SCRIPT,argv[1]);
    }
    else
    {
        sprintf(cmd,"%s sshd %s deny",LOGIN_FILTER_SCRIPT,argv[1]);
    }
    system(cmd);
    return ret;
#endif
    char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo add_iptables_rules.sh ssh %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;	
}
DECLARE_CONFIG_HANDLER(del_sshd_filter_func)
{
#if 0
    int ret = 0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }
    if(!strncmp(argv[0],"permit",strlen(argv[0])))
    {
        sprintf(cmd,"%s sshd %s permit",DEL_LOGIN_FILTER_SCRIPT,argv[1]);
    }
    else
    {
        sprintf(cmd,"%s sshd %s deny",DEL_LOGIN_FILTER_SCRIPT,argv[1]);
    }
    system(cmd);
    return ret;
#endif
    char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo del_iptables_rules.sh ssh %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;
}

/*********************************************************/

DECLARE_CONFIG_HANDLER(add_www_server_rules_func)
{
    char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo add_iptables_rules.sh http %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;
}


DECLARE_CONFIG_HANDLER(del_www_server_rules_func)
{
     char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo del_iptables_rules.sh http %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;
}

DECLARE_CONFIG_HANDLER(add_snmp_server_rules_func)
{
    char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo add_iptables_rules.sh snmp %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;
}


DECLARE_CONFIG_HANDLER(del_snmp_server_rules_func)
{
     char cmd[256];
    if(argc != 2)
    {
        return 0;
    }
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd,"sudo del_iptables_rules.sh snmp %s %s\n", argv[0], argv[1]);
    system(cmd); 
    return 0;
}

/*********************************************************/

int backup_login_filter_config()
{
    //config_item(CONFIG_TELNETD, 0, NULL, set_telnetd_filter_func, NULL);  
    //config_item(CONFIG_SSHD, 0, NULL, set_sshd_filter_func, NULL);
    //config_item(DEL_TELNETD, 0, NULL, del_telnetd_filter_func, NULL);  
   // config_item(DEL_SSHD, 0, NULL, del_sshd_filter_func, NULL);
    config_item(ADD_TELNETD_RULES, 0, NULL, set_telnetd_filter_func, NULL);  
    config_item(ADD_SSHD_RULES, 0, NULL, set_sshd_filter_func, NULL);
    config_item(DEL_TELNETD_RULES, 0, NULL, del_telnetd_filter_func, NULL);  
    config_item(DEL_SSHD_RULES, 0, NULL, del_sshd_filter_func, NULL);
    config_item(ADD_HTTPD_RULES, 0, NULL, add_www_server_rules_func, NULL);  
    config_item(DEL_HTTPD_RULES, 0, NULL, del_www_server_rules_func, NULL);
    config_item(ADD_SNMPD_RULES, 0, NULL, add_snmp_server_rules_func, NULL);  
    config_item(DEL_SNMPD_RULES, 0, NULL, del_snmp_server_rules_func, NULL);
    backup_hosts_file_config(HOSTS_ALLOW_FILE);
    backup_hosts_file_config(HOSTS_DENY_FILE);
    return 0;
}
int backup_hosts_file_config(char *file_name)
{
    FILE * fp= NULL;
    char filter[64];
    char addr[32];
    char service[16];
    char para[256];
    char item[128];
    fp= fopen(file_name,"r");
    if(fp)
    {
        memset(filter,0,64);
        fscanf(fp,"%s",filter);
        while(strlen(filter))
        {
            memset(service,0,16);
            memset(addr,0,32);
            resolve_hosts_file_item(filter,service,addr); 
            memset(para,0,256);
            memset(item,0,128);
            if(!strncmp(file_name,HOSTS_ALLOW_FILE,strlen(HOSTS_ALLOW_FILE)))
            {
                sprintf(para,"permit %s",addr);
                sprintf(item,"%s%s:%s",PERMIT_EXTRA,service,addr);
            }
            else
            {
                sprintf(para,"deny %s",addr);
                sprintf(item,"%s%s:%s",DENY_EXTRA,service,addr);
            }
            if(!strncmp(service,TELNETD_EXTRA,strlen(service)))
            {
                if(strlen(addr)>3)
                {
                    config_item(item, 2, para, set_telnetd_filter_func, NULL); 
                }
            }
            if(!strncmp(service,SSHD_EXTRA,strlen(service)))
            {
                if(strlen(addr)>3)
                {
                    config_item(item, 2, para, set_sshd_filter_func, NULL);
                }
            }            
            memset(filter,0,64);
            fscanf(fp,"%s",filter);
        }
        fclose(fp);
    }
    return 0;
}

#endif

DECLARE_CONFIG_HANDLER(set_pwd_aging)
{
    int ret = 0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }   
    sprintf(cmd,"%s aging %s",SET_PWD_SCRIPT,argv[0]);    
    system(cmd);
    system(SHOW_PWD_SCRIPT);
    return ret;
}

DECLARE_CONFIG_HANDLER(set_pwd_retry)
{
    int ret = 0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }   
    sprintf(cmd,"%s retry %s",SET_PWD_SCRIPT,argv[0]);    
    system(cmd);
    system(SHOW_PWD_SCRIPT);
    return ret;
}

DECLARE_CONFIG_HANDLER(manufacture_test)
{
    int ret = 0;
    char cmd[256];
    if(!argc)
    {
        return 0;
    }   
    //sprintf(cmd,"%s %s",MANUFACTURE_TEST_SH,argv[0]);    
    //system(cmd);
    return ret;
}

int resolve_hosts_file_item(char *item,char *service,char *addr)
{
    if(!strncmp(item,"in.telnetd",strlen("in.telnetd")))
    {
        memcpy(service,"in.telnetd",strlen("in.telnetd"));
        memcpy(addr,item+strlen("in.telnetd")+1,strlen(item)-strlen("in.telnetd")-1);
    }
    if(!strncmp(item,"sshd",strlen("sshd")))
    {
        memcpy(service,"sshd",strlen("sshd"));
        memcpy(addr,item+strlen("sshd")+1,strlen(item)-strlen("sshd")-1);
    }
    return 0;
}


void init_password_config()
{
    system(SHOW_PWD_SCRIPT);
}

int backup_password_config()
{
    FILE *fp;
    char tmp[16];
    int ret=-1;
    fp=fopen(PWD_INFO_FILE,"r");
    if(!fp)
    {
        command_sync_printf("%%Open file %s failed\r\n",PWD_INFO_FILE);
        return -1;        
    }
    memset(tmp,0,16);
    fscanf(fp,"%s",tmp);
    ret=config_item(PWD_AGING_DAY, 1, tmp, set_pwd_aging, NULL);
    if(ret)
    {
        fclose(fp);   
        command_sync_printf("back up %s failed \n",PWD_AGING_DAY);
        return -1;
    }
    memset(tmp,0,16);
    fscanf(fp,"%s",tmp);
    ret = config_item(PWD_RETRY, 1, tmp, set_pwd_retry, NULL);
    if(ret)
    {
        fclose(fp);   
        command_sync_printf("back up %s failed \n",PWD_RETRY);
        return -1;
    }
    fclose(fp);   
    return 0;
    
}

int del_slave_users()
{
    int ret,i;
	struct group *grentry = NULL;
	char *ptr;
	grentry = getgrnam(VIEWGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
		{
            ret = is_user_self(ptr);
        	if(ret == 1)
        	{
        		continue;
        	}
            config_item(DEL_USER, 1,ptr,del_user_func,NULL);
		}
	}
	return 0;
}
int back_up_all_users()
{
    int ret,i;
	struct group *grentry = NULL;
    struct spwd *passwd = NULL;
	char *ptr;
    char para[MAX_PARA_SIZE];
	grentry = getgrnam(ADMINGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
		{
            memset(para, 0, MAX_PARA_SIZE);
            passwd = getspnam(ptr);
            sprintf(para,"%s %s enable",ptr,passwd->sp_pwdp);
            ret = config_item(ptr, 3, para,add_user_func,"security");
            if(ret)
            {
                command_sync_printf("add user:%s failed \n",ptr);
                return -1;
            }
		}
	}
	grentry = getgrnam(VIEWGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
		{
            memset(para, 0, MAX_PARA_SIZE);
            passwd = getspnam(ptr);
            sprintf(para,"%s %s view",ptr,passwd->sp_pwdp);
            ret = config_item(ptr, 3, para,add_user_func,"security");
            if(ret)
            {
                command_sync_printf("add user:%s failed \n",ptr);
                return -1;
            }
		}
	}
    ret = config_item(ADD_USER,0, NULL, add_user_func, NULL);
    ret = config_item(DEL_USER,0, NULL, del_user_func, NULL);
    ret = config_item(CHANGE_PWD,0,NULL, change_pwd_func, NULL);
    ret = config_item(CHANGE_ROLE, 0,NULL, change_role_func, NULL);
	return 0;
}
int del_slave_dns_servers()
{
    //just need to let the file empty
    FILE *fp=NULL;
    fp = fopen("/etc/resolv.conf","w");
    if(!fp)
    {
        return -1;
    }
    fclose(fp);
    return 0;
    
        }

int back_up_dns_servers()
{
    char *dnsstr[MAX_DNS_SERVER];
    char dns_item[30];
    int ret,i;
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        dnsstr[i] = malloc(128);
        if(!dnsstr[i])
        {
            goto ret_err;
        }
        memset(dnsstr[i],0,128);
    }
    ret = get_dns_str(&dnsstr);
    if(ret<0)
    {
        command_sync_printf("Can't get system dns setting\n");
        goto ret_err;
    }
    else
    {
        for(i=0; i<ret; i++)
        {
            memset(dns_item,0,30);
            sprintf(dns_item,"%s%s",DNS_EXTRA,dnsstr[i]);
            if(config_item(dns_item, 1,dnsstr[i], set_dns_server_func, NULL))
            {
                command_sync_printf("add dns server:dnsstr[%d] %s failed \n",i,dnsstr[i]);
                return -1;
            }
        }
    }
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        if(dnsstr[i])
        {
            free(dnsstr[i]);
        }
    }
    
    ret = config_item(SET_DNS_SERVER,0, NULL, set_dns_server_func, NULL);
    ret = config_item(NO_DNS_SERVER, 0,NULL, no_dns_server_func, NULL);
    return 0;
ret_err:
    for(i=0; i<MAX_DNS_SERVER; i++)
    {
        if(dnsstr[i])
        {
            free(dnsstr[i]);
        }
    }

        return -1;


}

int config_item(char *config_name, int argc,char *para, config_handler user_handler, char *extra)
{
    int ret = 0;
    struct config_common_t user_entry;
    if(!config_name||!strlen(config_name)||strlen(config_name)> CONFIG_NAME_SIZE)
    {
        command_sync_printf("The size of config name must between 0-%d \n",CONFIG_NAME_SIZE);
        return -1;
    }
    memset(&user_entry, 0,sizeof(struct config_common_t));
    memcpy(user_entry.name,config_name,strlen(config_name));
    if(para && strlen(para))
    {
        if(strlen(para) > MAX_PARA_SIZE)
        {
            command_sync_printf("The size of para must be smaller than %d \n",MAX_PARA_SIZE);
            return -1;
        }
    memcpy(user_entry.para,para,strlen(para));
    }
    user_entry.handler = user_handler;
    if(extra && strlen(extra))
    {
        if(strlen(extra) > PARA_ITEM_SIZE)
        {
            command_sync_printf("The size of extra para must be smaller than %d \n",PARA_ITEM_SIZE);
            return -1;
        }
        memcpy(user_entry.extra_para,extra,strlen(extra));
    }
    user_entry.argc = argc;
    ret = dbtable_hash_insert(config_index, &user_entry);
    if(ret)
    {
        command_sync_printf("insert %s failed \r\n",config_name);
    }
    command_sync_printf("insert %s \n",config_name);
    return ret;
}
int systime_syn()
{
    int ret = -1;
    char para[MAX_PARA_SIZE];
    memset(para, 0, MAX_PARA_SIZE);
    ret = get_system_time(para);
    if(ret)
    {
        command_sync_printf("get systime failed \n");
        return ret;
    }
    ret =config_item(SYS_TIME,2, para, sys_time_func, NULL);
    if(ret)
    {
        command_sync_printf("systime_syn failed \n");
    }
    return ret;
}
int default_config()
{
    int ret = -1;
    char para[MAX_PARA_SIZE];
    memset(para, 0, MAX_PARA_SIZE);
    ret = get_system_time(para);
    ret =config_item(SYS_TIME,2, para, sys_time_func, NULL);
	
#ifdef HAVE_DIFF_OS

#else
    memset(para, 0, MAX_PARA_SIZE);
    ret = get_offset_time(para);
    ret =config_item(OFFSET, 1,para, offset_func, NULL);
    memset(para, 0, MAX_PARA_SIZE);
    get_sys_location(para);
    ret =config_item(SYS_LOCATION, 1,para, sys_location_func, NULL);
    memset(para, 0, MAX_PARA_SIZE);
    get_net_element(para);
    ret =config_item(NET_ELEMENT, 1,para, net_element_func, NULL);

    memset(para, 0, MAX_PARA_SIZE);
    ret = config_item(MANUFACTURE_TEST, 0, para, manufacture_test, NULL);
    memset(para, 0, MAX_PARA_SIZE);
    gethostname(para,MAX_PARA_SIZE);
    ret = config_item(SET_HOST_NAME, 1, para, set_host_name_func, NULL); 
    ret = config_item(ERASE_MEMORY,0, NULL, erase_memory_func, NULL); 
    memset(para, 0, MAX_PARA_SIZE);
    ret = get_idle_time(para);
    ret = config_item(SET_IDLE_TIME, 1,para, set_idle_time_func, NULL);
    
    memset(para, 0, MAX_PARA_SIZE);
    ret = get_cli_syslog_str();
    sprintf(para,"%d",ret);
    ret = config_item(CLI_LOG, 1, para,cli_log_func, NULL);

    memset(para, 0, MAX_PARA_SIZE);
    ret = get_sys_desc(para);
    ret =config_item(SYS_DESC,1, para, set_sysdesc_func, NULL);
    memset(para, 0, MAX_PARA_SIZE);
    get_sys_contact(para);
    ret =config_item(SYS_CONTACT,1, para, set_sys_contact_func, NULL);
#endif	
    return 0;
}
#ifdef __cplusplus
}
#endif


