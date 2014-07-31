
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */

#ifdef HAVE_SYSLOG
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <shadow.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/un.h>
#include "man_db.h"
#include "man_syslog.h"
#include "syslog_conf.h"
#include "npd/nbm/npd_cplddef.h"

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


/* modified from user_config.c */
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



int config_syslog_telnet(char *admin_status)
{
    char *tty_name;
    char cmd[256];
    char telnet_d[10];
    tty_name=ttyname(1);
    if(!admin_status||!strlen(admin_status))
    {
        return 0;
    }
	if(tty_name == NULL)
		return -1;
    if(tty_name[5]!='p')
    {
        printf("%% Local user cannot enbale syslog to telnet terminal\n");
        return -1;
    }
    memset(telnet_d,0,10);
    memcpy(telnet_d,tty_name+9,strlen(tty_name+9));
    if(!strncmp(admin_status,"enable",strlen(admin_status)))
    {
        sprintf(cmd,"%s d_pts%s",ENABLE_DEST_SCRIPT,telnet_d);
    }
    else
    {
        sprintf(cmd,"%s d_pts%s",DISABLE_DEST_SCRIPT,telnet_d);
    }
	system(cmd);
    system(SYSLOG_RESTART_CMD);
}

int sync_add_syslog_server(char *addr,int port)
{
    int ret =-1;
    char para[64];
    sprintf(para,"%s %d",addr,port);
    ret=db_update_via_para(ADD_SERVER, 2, para, NULL);
    if(ret)
    {
        printf("%%Add syslog server %s failed \r\n");
        return -1;
    }
    return 0;    
    
}

int sync_delete_syslog_server(char *addr)
{
    int ret =-1;
    ret=db_entry_delete_via_para(DEL_SERVER, 1, addr, SYSLOG_EXTRA);
    if(ret)
    {
        printf("%%Delete syslog server %s failed \r\n",addr);
        return -1;
    }
    return 0;    
    
}

int sync_syslog_dest_level(int dest,char *lvl)
 {
    char para[32];
    char config_name[16];
    int ret =-1;
    if(dest<dest_server||dest>dest_console)
    {
        printf("%%Wrong syslog destination number\r\n");
        return -1;
    }
    memset(config_name,0,16);
    if(dest==dest_server)
    {
        memcpy(config_name,SERVER_LEVEL,strlen(SERVER_LEVEL));
        sprintf(para,"server %s",lvl);
    }
    if(dest==dest_telnet)
    {
        memcpy(config_name,TELNET_LEVEL,strlen(TELNET_LEVEL));
        sprintf(para,"telnet %s",lvl);
    }
    if(dest==dest_console)
    {
        memcpy(config_name,CONSOLE_LEVEL,strlen(CONSOLE_LEVEL));
        sprintf(para,"console %s",lvl);
    }    
    ret=db_update_via_para(config_name, 2, para, NULL);
    if(ret)
    {
        printf("%%Config %s failed \r\n",config_name);
        return -1;
    }
    return 0;   
    
    
 }

int get_syslog_module(int number,char *module_name)
{
    char filter[10];
    int num =0;
    int ret= -1;
    FILE *fp = NULL;  
    fp =fopen("/etc/syslog-ng/module.tmp","r");
    if(!fp)
    {
        /*printf("%% Open file:/etc/syslog-ng/module.tmp failed\n");*/
        return -1;
    }
    memset(filter, 0, 10);
    fscanf(fp,"%s",filter);
    while(strlen(filter))
    { 
        num++;
        if(num==number)
        {
            memcpy(module_name,filter,strlen(filter));
            ret=0;
            break;
        }
        memset(filter, 0, 10);
        fscanf(fp,"%s",filter);
    }
    fclose(fp);
    return ret;
}

int get_syslog_level(char *lvl)
{
    char level[10];
    int ret=0;
	FILE *fp = NULL;	
    fp =fopen("/etc/syslog-ng/level.tmp","r");
    if(!fp)
    {
        /*printf("%% Open file:/etc/syslog-ng/level.tmp failed\n");*/
        return -1;
    }
    memset(level, 0, 10);
    fscanf(fp,"%s",level);
    ret =strlen(level);
    if(ret == 0)
    {
        printf("%% Cannot get level\n");
        return -1;
    }
    memcpy(lvl,level,strlen(level));
    fclose(fp);
    return 0;
}

int get_syslog_dest_level(char *srv_lvl,char *teln_lvl,char *cons_lvl)
{
    char level[10];
    int ret=0;
	FILE *fp = NULL;	
    fp =fopen("/etc/syslog-ng/level.tmp","r");
    if(!fp)
    {
        /*printf("%% Open file:/etc/syslog-ng/level.tmp failed\n");*/
        return -1;
    }
    memset(level, 0, 10);
    fscanf(fp,"%s",level);
    ret =strlen(level);
    if(ret == 0)
    {
        printf("%% Cannot get syslog server level\n");
        return -1;
    }
    memcpy(srv_lvl,level,strlen(level));
    memset(level, 0, 10);
    fscanf(fp,"%s",level);
    ret =strlen(level);
    if(ret == 0)
    {
        printf("%% Cannot get syslog telnet level\n");
        return -1;
    }
    memcpy(teln_lvl,level,strlen(level));
    memset(level, 0, 10);
    fscanf(fp,"%s",level);
    ret =strlen(level);
    if(ret == 0)
    {
        printf("%% Cannot get syslog console level\n");
        return -1;
    }
    memcpy(cons_lvl,level,strlen(level));
    fclose(fp);
    return 0;
}

void get_syslog_dest_state(char *server_state,char *console_state,char *telnet_state)
{
    char state[10];
    char tmp[256];
    char *tty_name;
    int ret=-1;
    FILE *fp = NULL;    
    /* read the state of destination */
    fp =fopen("/etc/syslog-ng/state.tmp","r");
    if(!fp)
    {
        /*printf("%% Open file:/etc/syslog-ng/state.tmp failed\n");*/
        return;
    }
    memset(state, 0, 10);
    fscanf(fp,"%s",state);
    ret =strlen(state);
    if(ret == 0)
    {
        printf("%% Cannot get the state of destination\n");
        fclose(fp);
        return;
    }
    memcpy(console_state,state,strlen(state));
    
    memset(state, 0, 10);
    fscanf(fp,"%s",state);
    ret =strlen(state);
    if(ret == 0)
    {
        printf("%% Cannot get the state of destination\n");
        fclose(fp);
        return;
    }
    memcpy(server_state,state,strlen(state));
    fclose(fp);

    /* get the state of logging to telnet */
    memset(tmp,0,256);
    memcpy(tmp,"/etc/syslog-ng/telnet_d_pts",strlen("/etc/syslog-ng/telnet_d_pts"));
    tty_name=ttyname(1);
	if(tty_name)
        strcat(tmp,tty_name+9);
    fp =fopen(tmp,"r");
    if(!fp)
    {
        memcpy(telnet_state,"disable",strlen("disable"));
    }
    else
    {
        memset(state, 0, 10);
        fscanf(fp,"%s",state);
        ret =strlen(state);
        if(ret == 0)
        {
            printf("%% Cannot get the state of destination:telnet terminal\n");
            fclose(fp);
            return;
        }
        memcpy(telnet_state,state,strlen(state));
        fclose(fp);
    }    
    
    
}

int get_syslog_server(int number,char *srv_addr,int *srv_port,char *srv_proto)
{
    char proto[10];
    char addr[20];
    char port[10];
    int num =0;
    int pt;
    int ret= -1;
    FILE *fp = NULL;  
    fp =fopen("/etc/syslog-ng/conf.tmp","r");
    if(!fp)
    {
        /*printf("%% Open file:/etc/syslog-ng/conf.tmp failed\n");*/
        return -1;
    }
    memset(proto, 0, 10);
    fscanf(fp,"%s",proto);
    while(strlen(proto))
    {
        num++;
        memset(addr,0,20);
        fscanf(fp,"%s",addr);
        memset(port,0,10);
        fscanf(fp,"%s",port);
        pt= atoi(port);
        if(num==number)
        {
            memcpy(srv_addr,addr,strlen(addr));
            memcpy(srv_proto,proto,strlen(proto));
            *srv_port=pt;
            ret=0;
            break;
            
        }
        memset(proto, 0, 10);
        fscanf(fp,"%s",proto);
    }
    fclose(fp);
    return ret;
    
}

SYNC_CONFIG(sync_local_udp_port, LOCAL_UDP_PORT, 1, NULL)
SYNC_CONFIG(sync_syslog_server_state, CONTROL_SERVER, 1, NULL)
#endif

