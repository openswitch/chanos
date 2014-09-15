
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */

#ifdef HAVE_MAN_SNMP
#include "lib/osinc.h"
#include "man_snmpd_engine.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>
#include <ctype.h>

#include <dbus/dbus.h>
#include "npd_dbus_def.h"
#include "sysdef/npd_sysdef.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef.h"

#include <net/if.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mcheck.h>
#include "man_product.h"





extern DBusConnection *dcli_dbus_connection;
DBusConnection *ccgi_dbus_connection;
STSNMPSummary *gpstSnmpSummary = NULL;



#if 0
static int load_xml_config(STSNMPSummary *pstSummary, char *file_path);

static int add_sysinfo_node(xmlNodePtr root, STSNMPSysInfo *pstSnmpSysInfo);
static int add_community_node(xmlNodePtr comm_root, STCommunity *pstCommunity, int num);
static int add_v3user_node(xmlNodePtr v3user_root, STSNMPV3User *pstV3User, int num);
static int add_trap_receiver_node(xmlNodePtr recv_root, STSNMPTrapReceiver *pstTrapReceiver, int num);

static int load_sysinfo_node(xmlNodePtr root, STSNMPSysInfo *pstSnmpSysInfo);
static int load_community_node(xmlNodePtr comm_root_node, STCommunity *pstCommunity, int max_num);
static int load_v3user_node(xmlNodePtr v3usr_root_node, STSNMPV3User *pstV3User, int max_num);
static int load_trap_receiver_node(xmlNodePtr recv_root_node, STSNMPTrapReceiver *pstTrapReceiver, int max_num);

static int write_xml_config(STSNMPSummary *pstSummary, char *file_path);
#endif
static int write_snmp_config(STSNMPSummary *pstSummary, char *file_path);

static int get_sys_non_conf_info(STSNMPSysInfo *pstSnmpSysInfo);
static int init_sys_info_to_default(STSNMPSysInfo *pstSnmpSysInfo);


char* get_token(char *str_tok, unsigned int *pos)
{
    unsigned int temp = *pos;

    while (isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }

    while (!isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }

    while (isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }

    *pos = temp;
    return (char *)(str_tok+temp);
}



int snmp_conf_init()
{
    int iRet;

    if (NULL != gpstSnmpSummary)
    {
    	memset(gpstSnmpSummary, 0, sizeof(STSNMPSummary));
    	load_snmp_conf(CONF_FILE_PATH, gpstSnmpSummary);
        iRet = get_sys_non_conf_info(&(gpstSnmpSummary->snmp_sysinfo));
        return SE_WARNING_INITIALIZED;
    }

    gpstSnmpSummary = (STSNMPSummary *)malloc(sizeof(STSNMPSummary));

    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_MALLOC_FAILED;
    }

    memset(gpstSnmpSummary, 0, sizeof(STSNMPSummary));
    {
		iRet = load_snmp_conf(CONF_FILE_PATH, gpstSnmpSummary);
        iRet = get_sys_non_conf_info(&(gpstSnmpSummary->snmp_sysinfo));
        //Generate_default_snmpd_option(gpstSnmpSummary);
    }
	return iRet;
error:
    free(gpstSnmpSummary);
    gpstSnmpSummary = NULL;
    return iRet;
}



STSNMPSummary* get_glSnmpStuct()
{
    return gpstSnmpSummary;
}



int snmp_conf_release()
{
    if (NULL != gpstSnmpSummary)
    {
        free(gpstSnmpSummary);
        gpstSnmpSummary = NULL;
    }

    return SE_OK;
}

int snmp_conf_save()
{
    int iRet;
    iRet = write_snmp_config(gpstSnmpSummary, CONF_FILE_PATH);
    system("sudo chmod 777 "CONF_FILE_PATH);
	system("cp "CONF_FILE_PATH" /mnt");
	system("sync");
    return iRet;
}

int start_snmp_service()
{
    int iRet,status;
    status = system(SCRIPT_PATH" "SCRIPT_PARAM_RESTART);
    iRet = WEXITSTATUS(status);
    return iRet;
}

int restart_snmp_service()
{
    int iRet=0,status;
	FILE *fp;	
	char snmp_status[8] = {0};
	fp = fopen(STATUS_FILE_PATH, "r");
	if(!fp)
    {
        return 0;
    }
	fscanf(fp, "%s", snmp_status);
	if (!strncmp(snmp_status, "start", strlen(snmp_status)))
	{
        status = system(SCRIPT_PATH" "SCRIPT_PARAM_RESTART);
    	iRet = WEXITSTATUS(status);
	}
	fclose(fp);
    return iRet;
}


int stop_snmp_service()
{
    int iRet,status;
    status = system(SCRIPT_PATH" "SCRIPT_PARAM_STOP);
    iRet = WEXITSTATUS(status);
    return iRet;
}


int get_sysinfo(STSNMPSysInfo *pstSnmpSysInfo)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstSnmpSysInfo)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(pstSnmpSysInfo , &(gpstSnmpSummary->snmp_sysinfo) , sizeof(STSNMPSysInfo));
    return 0;
}



int set_sysinfo(STSNMPSysInfo *pstSnmpSysInfo)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstSnmpSysInfo)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(&(gpstSnmpSummary->snmp_sysinfo) , pstSnmpSysInfo , sizeof(STSNMPSysInfo));
    return 0;
}


int get_community_num()
{
    if (NULL == gpstSnmpSummary)
    {
        return 0;
    }

    //fprintf(stderr,"community_num=%d",gpstSnmpSummary->community_num);
    return gpstSnmpSummary->community_num;
}

int add_community(STCommunity *pstCommunity)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstCommunity)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (gpstSnmpSummary->community_num >= MAX_COMMUNITY_NUM)
    {
        return SE_ERROR_COMMU_MAX_LIMIT;
    }

    memcpy(&(gpstSnmpSummary->community[gpstSnmpSummary->community_num]),
           pstCommunity,
           sizeof(STCommunity));
    gpstSnmpSummary->community_num ++;
    return SE_OK;
}

int get_community(STCommunity *pstCommunity, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstCommunity || index >= gpstSnmpSummary->community_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(pstCommunity,
           &(gpstSnmpSummary->community[index]),
           sizeof(STCommunity));
    return SE_OK;
}


int modify_community(STCommunity *pstCommunity, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstCommunity || index >= gpstSnmpSummary->community_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(&(gpstSnmpSummary->community[index]),
           pstCommunity,
           sizeof(STCommunity));
    return SE_OK;
}


int del_community(int index)
{
	int i= 0;
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (index >= gpstSnmpSummary->community_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

	if((!strcmp(gpstSnmpSummary->community[index].community,"public"))||(!strcmp(gpstSnmpSummary->community[index].community,"private")))
	{
		return SE_ERROR_ERR_INPUT;
	}		
	for(i=0;i<gpstSnmpSummary->access_num;i++)
	{
		if(!strcmp(gpstSnmpSummary->community[index].community,gpstSnmpSummary->access[i].accommunity))
		{
			del_access(i);
		}
	}

    if (index < gpstSnmpSummary->community_num - 1)
    {
        memcpy(&(gpstSnmpSummary->community[index]),
               &(gpstSnmpSummary->community[index+1]),
               sizeof(STCommunity)*(gpstSnmpSummary->community_num - index - 1));
    }

    gpstSnmpSummary->community_num --;
    return SE_OK;
}


int del_community_by_name(char *name)
{
    int index_of_name = -1;
    int i = 0;

    for (; i< gpstSnmpSummary->community_num; i++)
    {
        if (!strcmp(gpstSnmpSummary->community[i].community, name))
        {
            index_of_name = i;
            break;
        }
    }

    if (index_of_name >= 0)
    {
        return del_community(index_of_name);
    }

    return SE_OK;
}

int modify_community_by_name(STCommunity *pstCommunity, char *name)
{
    int index_of_name = -1;
    int i = 0;

    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    for (; i< gpstSnmpSummary->community_num; i++)
    {
        if (!strncmp(gpstSnmpSummary->community[i].community, name, strlen(name)))
        {
            index_of_name = i;
            break;
        }
    }

    if (index_of_name < 0)
    {
        return SE_OK;
    }

    if (NULL == pstCommunity)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(&(gpstSnmpSummary->community[index_of_name]),
           pstCommunity,
           sizeof(STCommunity));
    return SE_OK;
}
int get_deny_ip_num()
{
    if (NULL == gpstSnmpSummary)
    {
        return 0;
    }

    return gpstSnmpSummary->deny_ip_num;
}
int get_deny_ip(STSNMPIP *pstip, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstip || index >= gpstSnmpSummary->deny_ip_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(pstip,
           &(gpstSnmpSummary->deny_ip[index]),
           sizeof(STSNMPIP));
    return SE_OK;
}

int add_deny_ip(STSNMPIP *stsnmpip)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == stsnmpip)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (gpstSnmpSummary->deny_ip_num >= MAX_RISTRICT_IP_NUM)
    {
        return SE_ERROR_COMMU_MAX_LIMIT;
    }

    memcpy(&(gpstSnmpSummary->deny_ip[gpstSnmpSummary->deny_ip_num]),
           stsnmpip,
           sizeof(STSNMPIP));
    gpstSnmpSummary->deny_ip_num ++;
    return SE_OK;
}

int del_deny_ip(char *ip_addr)
{
    int index_of_ip= -1;
    int i = 0;

    for (; i< gpstSnmpSummary->deny_ip_num; i++)
    {
        if (!strncmp(gpstSnmpSummary->deny_ip[i].ip_addr, ip_addr, strlen(ip_addr)))
        {
            index_of_ip = i;
            break;
        }
    }
    if (index_of_ip >= 0)
    {
        return del_ip(index_of_ip);
    }

    return SE_OK;
}

int del_ip(int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (index >= gpstSnmpSummary->deny_ip_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (index < gpstSnmpSummary->deny_ip_num - 1)
    {
        memcpy(&(gpstSnmpSummary->deny_ip[index]),
               &(gpstSnmpSummary->deny_ip[index+1]),
               sizeof(STSNMPIP)*(gpstSnmpSummary->deny_ip_num - index - 1));
    }

    gpstSnmpSummary->deny_ip_num --;
    return SE_OK;
}
int get_v3user_num()
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    return gpstSnmpSummary->v3user_num;
}



int get_v3user(STSNMPV3User *pstV3user, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstV3user || index >= gpstSnmpSummary->v3user_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(pstV3user,
           &(gpstSnmpSummary->v3user[index]),
           sizeof(STSNMPV3User));
    //fprintf( stderr,"namex=%s",pstV3user->name );
    return SE_OK;
}


int add_v3user(STSNMPV3User *pstV3User)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstV3User)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (gpstSnmpSummary->v3user_num >= MAX_SNMPV3_USER_NUM)
    {
        return SE_ERROR_COMMU_MAX_LIMIT;
    }

    memcpy(&(gpstSnmpSummary->v3user[gpstSnmpSummary->v3user_num]),
           pstV3User,
           sizeof(STSNMPV3User));
    gpstSnmpSummary->v3user_num ++;
    return SE_OK;
}


int modify_v3user(STSNMPV3User *pstV3User, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstV3User || index >= gpstSnmpSummary->v3user_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(&(gpstSnmpSummary->v3user[index]),
           pstV3User,
           sizeof(STSNMPV3User));
    return SE_OK;
}


int del_v3user(int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (index >= gpstSnmpSummary->v3user_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (index < gpstSnmpSummary->v3user_num - 1)
    {
        memcpy(&(gpstSnmpSummary->v3user[index]),
               &(gpstSnmpSummary->v3user[index+1]),
               sizeof(STSNMPV3User)*(gpstSnmpSummary->v3user_num - index - 1));
    }

    gpstSnmpSummary->v3user_num --;
    return SE_OK;
}


int modify_v3user_by_name(STSNMPV3User *pstV3User, char *name)
{
    int index_of_name = -1;
    int i = 0;

    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstV3User)
    {
        return SE_ERROR_ERR_INPUT;
    }

    for (; i< gpstSnmpSummary->v3user_num; i++)
    {
        if (!strncmp(gpstSnmpSummary->v3user[i].name, name, strlen(name)))
        {
            index_of_name = i;
            break;
        }
    }

    if (index_of_name >= 0)
    {
        memcpy(&(gpstSnmpSummary->v3user[index_of_name]),
               pstV3User,
               sizeof(STSNMPV3User));
    }

    return SE_OK;
}


int del_v3user_by_name(char *name)
{
    int index_of_name = -1;
    int i = 0;

    for (; i< gpstSnmpSummary->v3user_num; i++)
    {
        if (!strncmp(gpstSnmpSummary->v3user[i].name, name, strlen(name)))
        {
            index_of_name = i;
            break;
        }
    }

    if (index_of_name >= 0)
    {
        return del_v3user(index_of_name);
    }

    return SE_OK;
}



int get_trap_receiver_num()
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    return gpstSnmpSummary->receiver_num;
}



int check_trap_index_single(int index)
{
    if (index < 0)
    {
        return -2;/*input is wrong*/
    }

    int num = gpstSnmpSummary->receiver_num;
    int i;

    for (i=0; i<num; i++)
    {
        if (gpstSnmpSummary->receiver[i].index == index)
        {
            return -1;/*has been existed!*/
        }
    }

    return 0;/*OK*/
}



int add_trap_receiver(STSNMPTrapReceiver *pstTrapReceiver)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstTrapReceiver)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (gpstSnmpSummary->receiver_num >= MAX_TRAPRECEIVER_NUM)
    {
        return SE_ERROR_TRAP_MAX_LIMIT;
    }

    memcpy(&(gpstSnmpSummary->receiver[gpstSnmpSummary->receiver_num]),
           pstTrapReceiver,
           sizeof(STSNMPTrapReceiver));
    gpstSnmpSummary->receiver_num ++;
    return SE_OK;
}


int modify_trap_receiver(STSNMPTrapReceiver *pstTrapReceiver, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstTrapReceiver || index >= gpstSnmpSummary->receiver_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(&(gpstSnmpSummary->receiver[index]),
           pstTrapReceiver,
           sizeof(STSNMPTrapReceiver));
    return SE_OK;
}

int modify_trap_receiver_by_index(STSNMPTrapReceiver *pstTrapReceiver, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstTrapReceiver)
    {
        return SE_ERROR_ERR_INPUT;
    }

    int num = gpstSnmpSummary->receiver_num;
    int i;

    for (i=0; i<num; i++)
    {
        if (gpstSnmpSummary->receiver[i].index == index)
        {
            memcpy(&(gpstSnmpSummary->receiver[i]),
                   pstTrapReceiver,
                   sizeof(STSNMPTrapReceiver));
            return SE_OK;
        }
    }

    return SE_OK;
}


int get_trap_receiver(STSNMPTrapReceiver *pstTrapReceiver, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstTrapReceiver || index >= gpstSnmpSummary->receiver_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(pstTrapReceiver,
           &(gpstSnmpSummary->receiver[index]),
           sizeof(STSNMPTrapReceiver));
    return SE_OK;
}

int get_trap_receiver_by_index(STSNMPTrapReceiver *pstTrapReceiver, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstTrapReceiver)
    {
        return SE_ERROR_ERR_INPUT;
    }

    int num = gpstSnmpSummary->receiver_num;
    int i;

    for (i=0; i<num; i++)
    {
        if (gpstSnmpSummary->receiver[i].index == index)
        {
            memcpy(pstTrapReceiver,
                   &(gpstSnmpSummary->receiver[i]),
                   sizeof(STSNMPTrapReceiver));
            return SE_OK;
        }
    }

    return SE_OK;
}




int del_trap_receiver(int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (index >= gpstSnmpSummary->receiver_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (index < gpstSnmpSummary->receiver_num - 1)
    {
        memcpy(&(gpstSnmpSummary->receiver[index]),
               &(gpstSnmpSummary->receiver[index+1]),
               sizeof(STSNMPTrapReceiver)*(gpstSnmpSummary->receiver_num - index - 1));
    }

    gpstSnmpSummary->receiver_num --;
    return SE_OK;
}


int del_trap_receiver_by_index(int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    int num = gpstSnmpSummary->receiver_num;
    int i;

    for (i=0; i<num; i++)
    {
        if (gpstSnmpSummary->receiver[i].index == index)
        {
            memcpy(&(gpstSnmpSummary->receiver[i]),
                   &(gpstSnmpSummary->receiver[i+1]),
                   sizeof(STSNMPTrapReceiver)*(num-i));
            gpstSnmpSummary->receiver_num --;
            return SE_OK;
        }
    }

    return SE_OK;
}


int modify_trap_receiver_by_name(STSNMPTrapReceiver *pstTrapReceiver, char *name)
{
    int num = gpstSnmpSummary->receiver_num;
    int i;

    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstTrapReceiver)
    {
        return SE_ERROR_ERR_INPUT;
    }

    for (i=0; i<num; i++)
    {
        if (!strncmp(gpstSnmpSummary->receiver[i].name, name, strlen(name)))
        {
            memcpy(&(gpstSnmpSummary->receiver[i]),
                   pstTrapReceiver,
                   sizeof(STSNMPTrapReceiver));
            break;
        }
    }

    return SE_OK;
}

int del_trap_receiver_by_name(char *name)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    int num = gpstSnmpSummary->receiver_num;
    int i;

    for (i=0; i<num; i++)
    {
        if (!strncmp(gpstSnmpSummary->receiver[i].name, name, strlen(name)))
        {
            memcpy(&(gpstSnmpSummary->receiver[i]),
                   &(gpstSnmpSummary->receiver[i+1]),
                   sizeof(STSNMPTrapReceiver)*(num-i));
            gpstSnmpSummary->receiver_num --;
            return SE_OK;
        }
    }

    return SE_OK;
}

int get_v1_status_enable(void)
{
    if (NULL == gpstSnmpSummary)
    {
        return 0;
    }

    return gpstSnmpSummary->snmp_sysinfo.v1_status;
}

int get_v2c_status_enable(void)
{
    if (NULL == gpstSnmpSummary)
    {
        return 0;
    }

    return gpstSnmpSummary->snmp_sysinfo.v2c_status;
}

int get_v3_status_enable(void)
{
    if (NULL == gpstSnmpSummary)
    {
        return 0;
    }

    return gpstSnmpSummary->snmp_sysinfo.v3_status;
}



int get_view_num()
{
    if (NULL == gpstSnmpSummary)
    {
        return 0;
    }

    return gpstSnmpSummary->view_num;
}


int add_view(STSNMPView *pstView)
{
    int i = 0;
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstView)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (gpstSnmpSummary->view_num>= MAX_SNMP_VIEW_NUM)
    {
        return SE_ERROR_COMMU_MAX_LIMIT;
    }

    for (i = 0; i < gpstSnmpSummary->view_num; i++)
    {
        if(!strcmp(gpstSnmpSummary->view[i].name, pstView->name))
            return SE_ERROR_ERR_INPUT;
    }

    memcpy(&(gpstSnmpSummary->view[gpstSnmpSummary->view_num]),
           pstView,
           sizeof(STSNMPView));
    gpstSnmpSummary->view_num++;
    return SE_OK;
}



int del_view(int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (index >= gpstSnmpSummary->view_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

	if(!strcmp(gpstSnmpSummary->view[index].name,"all"))
	{
		return SE_ERROR_ERR_INPUT;
	}


    if (index < gpstSnmpSummary->view_num - 1)
    {
        memcpy(&(gpstSnmpSummary->view[index]),
               &(gpstSnmpSummary->view[index+1]),
               sizeof(STSNMPView)*(gpstSnmpSummary->view_num - index - 1));
    }

    gpstSnmpSummary->view_num--;
    return SE_OK;
}


int get_view(STSNMPView *pstView, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstView || index >= gpstSnmpSummary->view_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(pstView,
           &(gpstSnmpSummary->view[index]),
           sizeof(STSNMPView));
    return SE_OK;
}

int del_view_by_name(char *name)
{
    int index_of_name = -1;
    int i = 0;

    for (; i< gpstSnmpSummary->view_num; i++)
    {
        if (!strncmp(gpstSnmpSummary->view[i].name, name, strlen(name)))
        {
            index_of_name = i;
            break;
        }
    }

    if (index_of_name >= 0)
    {
        return del_view(index_of_name);
    }

    return SE_OK;
}


int add_access(STSNMPAccess *pstAccess)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstAccess)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (gpstSnmpSummary->access_num>= MAX_SNMP_ACCESS_NUM)
    {
        return SE_ERROR_COMMU_MAX_LIMIT;
    }

    memcpy(&(gpstSnmpSummary->access[gpstSnmpSummary->access_num]),
           pstAccess,
           sizeof(STSNMPAccess));
    gpstSnmpSummary->access_num++;
    return SE_OK;
}



int del_access_by_index(STSNMPAccess *paccess)
{
    int index_of_name = -1;
    int i = 0;

    for (; i< gpstSnmpSummary->access_num; i++)
    {
        if (!strncmp(gpstSnmpSummary->access[i].accommunity, paccess->accommunity, strlen(gpstSnmpSummary->access[i].accommunity))
            && !strncmp(gpstSnmpSummary->access[i].acview, paccess->acview, strlen(gpstSnmpSummary->access[i].acview)))
        {
            index_of_name = i;
            break;
        }
    }

    if (index_of_name >= 0)
    {
        return del_access(index_of_name);
    }

    return SE_OK;
}

int del_access(int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (index >= gpstSnmpSummary->access_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    if (index < gpstSnmpSummary->access_num- 1)
    {
        memcpy(&(gpstSnmpSummary->access[index]),
               &(gpstSnmpSummary->access[index+1]),
               sizeof(STSNMPAccess)*(gpstSnmpSummary->access_num - index - 1));
    }

    gpstSnmpSummary->access_num--;
    return SE_OK;
}


int get_access(STSNMPAccess *pstAccess, int index)
{
    if (NULL == gpstSnmpSummary)
    {
        return SE_ERROR_NOT_INITIAL;
    }

    if (NULL == pstAccess || index >= gpstSnmpSummary->access_num)
    {
        return SE_ERROR_ERR_INPUT;
    }

    memcpy(pstAccess,
           &(gpstSnmpSummary->access[index]),
           sizeof(STSNMPAccess));
    return SE_OK;
}

int get_access_num()
{
    if (NULL == gpstSnmpSummary)
    {
        return 0;
    }
    return gpstSnmpSummary->access_num;
}
#if 0
static int add_sysinfo_node(xmlNodePtr root, STSNMPSysInfo *pstSnmpSysInfo)
{
    xmlNodePtr sysinfo_node = NULL;
    char content[128]="";
#if 0
    char name[MAX_SYSTEM_NAME_LEN+1],description[MAX_SYSTEM_DESCRIPTION+1],sys_oid[MAX_OID_LEN+1];
    memset(name, 0, MAX_SYSTEM_NAME_LEN+1);
    memset(description, 0, MAX_SYSTEM_DESCRIPTION+1);
    memset(sys_oid, 0, MAX_OID_LEN+1);
    sprintf(name, "%s", pstSnmpSysInfo->sys_name);
    sprintf(description, "%s", pstSnmpSysInfo->sys_description);
    sprintf(sys_oid, "%s", pstSnmpSysInfo->sys_oid);
#endif

    if (NULL == root || NULL == pstSnmpSysInfo)
    {
        return SE_ERROR_ERR_INPUT;
    }

    //fprintf(stderr,"des=%s",pstSnmpSysInfo->sys_description);
    sysinfo_node = xmlNewNode(NULL, BAD_CAST(SE_XML_NODE_SYSINFO));
    xmlAddChild(root, sysinfo_node);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_SYSINFO_NAME), BAD_CAST(pstSnmpSysInfo->sys_name));
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_SYSINFO_DSCR), BAD_CAST(pstSnmpSysInfo->sys_description));
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_SYSINFO_OID), BAD_CAST(pstSnmpSysInfo->sys_oid));
    snprintf(content, sizeof(content)-1, "%u", pstSnmpSysInfo->agent_port);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_SYSINFO_APORT), BAD_CAST(content));
    snprintf(content, sizeof(content)-1, "%u", pstSnmpSysInfo->trap_port);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_SYSINFO_TPORT), BAD_CAST(content));
    snprintf(content, sizeof(content)-1, "%u", pstSnmpSysInfo->v1_status);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_V1_STATUS), BAD_CAST(content));
    snprintf(content, sizeof(content)-1, "%u", pstSnmpSysInfo->v2c_status);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_V2C_STATUS), BAD_CAST(content));
    snprintf(content, sizeof(content)-1, "%u", pstSnmpSysInfo->v3_status);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_V3_STATUS), BAD_CAST(content));
    //memset(content, 0, 128);
    //snprintf( content, sizeof(content)-1, "%u", pstSnmpSysInfo->mtThreadSwitch );
    //xmlNewChild( sysinfo_node, NULL, BAD_CAST (SE_XML_SYS_THREADSWITCH), BAD_CAST (content) );
    memset(content, 0, 128);
    snprintf(content, sizeof(content)-1, "%u", pstSnmpSysInfo->logSwitch);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_SYS_LOGSWITCH), BAD_CAST(content));
    memset(content, 0, 128);
    snprintf(content, sizeof(content)-1, "%u", pstSnmpSysInfo->cache_time);
    xmlNewChild(sysinfo_node, NULL, BAD_CAST(SE_XML_SYS_CACHETIME), BAD_CAST(content));
    return SE_OK;
}



static int add_community_node(xmlNodePtr comm_root, STCommunity *pstCommunity, int num)
{
    xmlNodePtr comm_node = NULL;
    char content[128]="";
    int i;

    if (NULL == comm_root || NULL == pstCommunity)
    {
        return SE_ERROR_ERR_INPUT;
    }

    for (i=0; i<num; i++)
    {
        comm_node = xmlNewNode(NULL, BAD_CAST(SE_XML_NODE_COMMUNITY));
        xmlAddChild(comm_root, comm_node);
        //fprintf( stderr, "add_community_node pstCommunity[%d].community = %s\n", i, pstCommunity[i].community );
        xmlNewChild(comm_node, NULL, BAD_CAST(SE_XML_COMM_KEY), BAD_CAST(pstCommunity[i].community));
        xmlNewChild(comm_node, NULL, BAD_CAST(SE_XML_COMM_IP), BAD_CAST(pstCommunity[i].ip_addr));
        xmlNewChild(comm_node, NULL, BAD_CAST(SE_XML_COMM_MASK), BAD_CAST(pstCommunity[i].ip_mask));
        snprintf(content, sizeof(content)-1, "%u", pstCommunity[i].access_mode);
        xmlNewChild(comm_node, NULL, BAD_CAST(SE_XML_COMM_ACCESS), BAD_CAST(content));
    }

    return SE_OK;
}


static int add_v3user_node(xmlNodePtr v3user_root, STSNMPV3User *pstV3User, int num)
{
    xmlNodePtr v3user_node = NULL;
    char content[128]="";
    int i;

    //fprintf( stderr, "11111" );
    if (NULL == v3user_root || NULL == pstV3User)
    {
        return SE_ERROR_ERR_INPUT;
    }

    //fprintf( stderr, "2222" );
    for (i=0; i<num; i++)
    {
        v3user_node = xmlNewNode(NULL, BAD_CAST(SE_XML_NODE_V3USER));
        xmlAddChild(v3user_root, v3user_node);
        //fprintf( stderr, "add_v3user_node pstV3User[%d].community = %s\n", i, pstV3User[i].name );
        xmlNewChild(v3user_node, NULL, BAD_CAST(SE_XML_V3USER_NAME), BAD_CAST(pstV3User[i].name));
        snprintf(content, sizeof(content)-1, "%u", pstV3User[i].access_mode);
        xmlNewChild(v3user_node, NULL, BAD_CAST(SE_XML_V3USER_ACCESS), BAD_CAST(content));
        snprintf(content, sizeof(content)-1, "%u", pstV3User[i].authentication.protocal);
        xmlNewChild(v3user_node, NULL, BAD_CAST(SE_XML_V3USER_AUTHPRO), BAD_CAST(content));
        xmlNewChild(v3user_node, NULL, BAD_CAST(SE_XML_V3USER_AUTHPW), BAD_CAST(pstV3User[i].authentication.passwd));
        snprintf(content, sizeof(content)-1, "%u", pstV3User[i].privacy.protocal);
        xmlNewChild(v3user_node, NULL, BAD_CAST(SE_XML_V3USER_PRIVPRO), BAD_CAST(content));
        xmlNewChild(v3user_node, NULL, BAD_CAST(SE_XML_V3USER_PRIVPW), BAD_CAST(pstV3User[i].privacy.passwd));

    }

    return SE_OK;
}

static int add_trap_receiver_node(xmlNodePtr recv_root, STSNMPTrapReceiver *pstTrapReceiver, int num)
{
    xmlNodePtr recv_node = NULL;
    char content[128]="";
    int i;

    if (NULL == recv_root || NULL == pstTrapReceiver)
    {
        return SE_ERROR_ERR_INPUT;
    }

    for (i=0; i<num; i++)
    {
        recv_node = xmlNewNode(NULL, BAD_CAST(SE_XML_NODE_RECV));
        xmlAddChild(recv_root, recv_node);
        snprintf(content, sizeof(content)-1, "%u", pstTrapReceiver[i].index);
        xmlNewChild(recv_node, NULL, BAD_CAST(SE_XML_TRAP_INDEX), BAD_CAST(content));
        xmlNewChild(recv_node, NULL, BAD_CAST(SE_XML_TRAP_NAME), BAD_CAST(pstTrapReceiver[i].name));
        xmlNewChild(recv_node, NULL, BAD_CAST(SE_XML_TRAP_IP), BAD_CAST(pstTrapReceiver[i].ip_addr));
        xmlNewChild(recv_node, NULL, BAD_CAST(SE_XML_TRAP_PORTNO), BAD_CAST(pstTrapReceiver[i].portno));
        snprintf(content, sizeof(content)-1, "%u", pstTrapReceiver[i].status);
        xmlNewChild(recv_node, NULL, BAD_CAST(SE_XML_TRAP_STATUS), BAD_CAST(content));
    }

    return SE_OK;
}

static int write_xml_config(STSNMPSummary *pstSummary, char *file_path)
{
    xmlDocPtr doc = NULL;			/* document pointer */
    char content[128]="";
    xmlNodePtr root_node = NULL;/* node pointers */
    xmlNodePtr comm_root = NULL;
    xmlNodePtr v3user_root = NULL;
    xmlNodePtr recv_root = NULL;
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST SE_XML_NODE_ROOT);
    xmlDocSetRootElement(doc, root_node);
    xmlNewChild(root_node, NULL, BAD_CAST(SE_XML_FILE_INFO), BAD_CAST(pstSummary->info));
    snprintf(content, sizeof(content)-1, "%u", pstSummary->version);
    xmlNewChild(root_node, NULL, BAD_CAST(SE_XML_SYS_VERSION), BAD_CAST(content));
    add_sysinfo_node(root_node, &(pstSummary->snmp_sysinfo));
    comm_root = xmlNewNode(NULL, BAD_CAST(SE_XML_NODE_COMM_ARRAY));
    xmlAddChild(root_node, comm_root);
    add_community_node(comm_root, pstSummary->community, pstSummary->community_num);
    v3user_root = xmlNewNode(NULL, BAD_CAST(SE_XML_NODE_V3USER_ARRAY));
    xmlAddChild(root_node, v3user_root);
    add_v3user_node(v3user_root, pstSummary->v3user, pstSummary->v3user_num);
    recv_root = xmlNewNode(NULL, BAD_CAST(SE_XML_NODE_RECV_ARRAY));
    xmlAddChild(root_node, recv_root);
    add_trap_receiver_node(recv_root, pstSummary->receiver, pstSummary->receiver_num);
    xmlSaveFormatFileEnc(file_path, doc, "UTF-8", 1);
    /*free the document */
    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlMemoryDump();	  //debug memory for regression tests
    return SE_OK;
}



static int load_sysinfo_node(xmlNodePtr root, STSNMPSysInfo *pstSnmpSysInfo)
{
    xmlNode *node = NULL;
    char *content;

    //char * sys_name = get_product_name();
    for (node=root->children; NULL!=node; node=node->next)
    {
        //fprintf(stderr,"hhhkk=%s",(char *)node->name);
        content = (char *)xmlNodeGetContent(node);

        if ((content == NULL)||(strlen(content) == 0))
        {
            if (NULL != content) xmlFree(content);

            continue;
        }

        if (strcmp((char *)node->name,SE_XML_SYSINFO_NAME) == 0)
        {
            strncpy(pstSnmpSysInfo->sys_name, content, sizeof(pstSnmpSysInfo->sys_name)-1);
        }
        else if (strcmp((char *)node->name,SE_XML_SYSINFO_DSCR) == 0)
        {
            memcpy(pstSnmpSysInfo->sys_description, content, sizeof(pstSnmpSysInfo->sys_description)-1);
        }
        else if (strcmp((char *)node->name,SE_XML_SYSINFO_OID) == 0)
        {
            strncpy(pstSnmpSysInfo->sys_oid, content, sizeof(pstSnmpSysInfo->sys_oid)-1);
        }
        else if (strcmp((char *)node->name,SE_XML_SYSINFO_APORT) == 0)
        {
            //sscanf( content, "%d", &(pstSnmpSysInfo->agent_port) );
            pstSnmpSysInfo->agent_port = SNMP_AGENG_PORT;
        }
        else if (strcmp((char *)node->name,SE_XML_SYSINFO_TPORT) == 0)
        {
            sscanf(content, "%d", &(pstSnmpSysInfo->trap_port));
        }
        else if (strcmp((char *)node->name,SE_XML_V1_STATUS) == 0)
        {
            sscanf(content, "%d", &(pstSnmpSysInfo->v1_status));
        }
        else if (strcmp((char *)node->name,SE_XML_V2C_STATUS) == 0)
        {
            sscanf(content, "%d", &(pstSnmpSysInfo->v2c_status));
        }
        else if (strcmp((char *)node->name,SE_XML_V3_STATUS) == 0)
        {
            sscanf(content, "%d", &(pstSnmpSysInfo->v3_status));
        }
        //else if( strcmp( (char *)node->name,SE_XML_SYS_THREADSWITCH ) == 0 )
        //{
        //	sscanf( content, "%d", &(pstSnmpSysInfo->mtThreadSwitch) );
        //}
        else if (strcmp((char *)node->name,SE_XML_SYS_LOGSWITCH) == 0)
        {
            sscanf(content, "%d", &(pstSnmpSysInfo->logSwitch));
        }
        else if (strcmp((char *)node->name,SE_XML_SYS_CACHETIME) == 0)
        {
            sscanf(content, "%d", &(pstSnmpSysInfo->cache_time));
        }

        xmlFree(content);
    }

    return SE_OK;
}


static int load_community_node(xmlNodePtr comm_root_node, STCommunity *pstCommunity, int max_num)
{
    xmlNode *node = NULL;
    xmlNode *comm_node = NULL;
    char *content;
    int i;

    for (comm_node=comm_root_node->children,i=0; (NULL!=comm_node)&&(i<max_num); comm_node=comm_node->next)
    {
        if (strcmp((char *)comm_node->name,SE_XML_NODE_COMMUNITY) != 0)
            continue;

        for (node = comm_node->children; NULL!=node; node=node->next)
        {
            content = (char *)xmlNodeGetContent(node);

            if ((content == NULL)||(strlen(content) == 0))
            {
                if (NULL != content) xmlFree(content);

                continue;
            }

            if (strcmp((char *)node->name,SE_XML_COMM_KEY) == 0)
            {
                strncpy(pstCommunity[i].community, content, sizeof(pstCommunity[i].community)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_COMM_IP) == 0)
            {
                strncpy(pstCommunity[i].ip_addr, content, sizeof(pstCommunity[i].ip_addr)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_COMM_MASK) == 0)
            {
                strncpy(pstCommunity[i].ip_mask, content, sizeof(pstCommunity[i].ip_mask)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_COMM_ACCESS) == 0)
            {
                sscanf(content, "%d", &(pstCommunity[i].access_mode));
            }
            else if (strcmp((char *)node->name,SE_XML_COMM_STATUS) == 0)
            {
                sscanf(content, "%d", &(pstCommunity[i].status));
            }

            //fprintf(stderr,"name=%s--i=%d\n",pstCommunity[i].community,i);
            xmlFree(content);
        }

        i++;
    }

    //fprintf(stderr,"ii=%d",i);
    return i;
}


static int load_v3user_node(xmlNodePtr v3usr_root_node, STSNMPV3User *pstV3User, int max_num)
{
    xmlNode *node = NULL;
    xmlNode *v3user_node = NULL;
    char *content;
    int i;

    for (v3user_node=v3usr_root_node->children,i=0; (NULL!=v3user_node)&&(i<max_num); v3user_node=v3user_node->next)
    {
        if (strcmp((char *)v3user_node->name,SE_XML_NODE_V3USER) != 0)
            continue;

        for (node = v3user_node->children; NULL!=node; node=node->next)
        {
            content = (char *)xmlNodeGetContent(node);

            if ((content == NULL)||(strlen(content) == 0))
            {
                if (NULL != content) xmlFree(content);

                continue;
            }

            if (strcmp((char *)node->name,SE_XML_V3USER_NAME) == 0)
            {
                strncpy(pstV3User[i].name, content, sizeof(pstV3User[i].name)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_V3USER_ACCESS) == 0)
            {
                sscanf(content, "%d", &(pstV3User[i].access_mode));
            }
            else if (strcmp((char *)node->name,SE_XML_V3USER_AUTHPRO) == 0)
            {
                sscanf(content, "%d", &(pstV3User[i].authentication.protocal));
            }
            else if (strcmp((char *)node->name,SE_XML_V3USER_AUTHPW) == 0)
            {
                strncpy(pstV3User[i].authentication.passwd, content, sizeof(pstV3User[i].authentication.passwd)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_V3USER_PRIVPRO) == 0)
            {
                sscanf(content, "%d", &(pstV3User[i].privacy.protocal));
            }
            else if (strcmp((char *)node->name,SE_XML_V3USER_PRIVPW) == 0)
            {
                strncpy(pstV3User[i].privacy.passwd, content, sizeof(pstV3User[i].privacy.passwd)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_V3USER_STATUS) == 0)
            {
                sscanf(content, "%d", &(pstV3User[i].status));
            }

            //fprintf(stderr,"name=%s\n",pstV3User->name);
            xmlFree(content);
        }

        i++;
        //fprintf(stderr,"namet=%s\n",gpstSnmpSummary->v3user[i].name);
    }

    return i;
}

static int load_trap_receiver_node(xmlNodePtr recv_root_node, STSNMPTrapReceiver *pstTrapReceiver, int max_num)
{
    xmlNode *node = NULL;
    xmlNode *recv_node = NULL;
    char *content;
    int i;

    for (recv_node=recv_root_node->children,i=0; (NULL!=recv_node)&&(i<max_num); recv_node=recv_node->next)
    {
        if (strcmp((char *)recv_node->name,SE_XML_NODE_RECV) != 0)
            continue;

        for (node = recv_node->children; NULL!=node; node=node->next)
        {
            content = (char *)xmlNodeGetContent(node);

            if ((content == NULL)||(strlen(content) == 0))
            {
                if (NULL != content) xmlFree(content);

                continue;
            }

            if (strcmp((char *)node->name,SE_XML_TRAP_INDEX) == 0)
            {
                sscanf(content, "%d", &(pstTrapReceiver[i].index));
            }

            if (strcmp((char *)node->name,SE_XML_TRAP_NAME) == 0)
            {
                strncpy(pstTrapReceiver[i].name, content, sizeof(pstTrapReceiver[i].name)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_TRAP_IP) == 0)
            {
                strncpy(pstTrapReceiver[i].ip_addr, content, sizeof(pstTrapReceiver[i].ip_addr)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_TRAP_PORTNO) == 0)
            {
                strncpy(pstTrapReceiver[i].portno, content, sizeof(pstTrapReceiver[i].portno)-1);
            }
            else if (strcmp((char *)node->name,SE_XML_TRAP_STATUS) == 0)
            {
                sscanf(content, "%d", &(pstTrapReceiver[i].status));
            }

            xmlFree(content);
        }

        i++;
    }

    return i;
}

static int load_xml_config(STSNMPSummary *pstSummary, char *file_path)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *node = NULL;
    char *content;

    if (0 != is_file_exist(file_path))
    {
        return SE_ERROR_FILE_NOT_EXIST;
    }

    doc = xmlReadFile(file_path, NULL, 0);

    if (doc == NULL)
    {
        return SE_ERROR_XML_FILE_ERR;
    }

    root_element = xmlDocGetRootElement(doc);

    for (node = root_element->children; node; node = node->next)
    {
        content = (char *)xmlNodeGetContent(node);

        if ((content == NULL)||(strlen(content) == 0))
        {
            if (NULL != content)
                xmlFree(content);

            continue;
        }

        if (strcmp((char *)node->name,SE_XML_FILE_INFO) == 0)
        {
            //if((strlen((char *)content) == 0)||(content == NULL))
            //continue;
            strncpy(pstSummary->info, content, sizeof(pstSummary->info)-1);
        }
        else if (strcmp((char *)node->name,SE_XML_SYS_VERSION) == 0)
        {
            //if((strlen((char *)content) == 0)||(content == NULL))
            //continue;
            sscanf(content, "%d", &(pstSummary->version));
        }
        else if (strcmp((char *)node->name,SE_XML_NODE_SYSINFO) == 0)
        {
            load_sysinfo_node(node, &(pstSummary->snmp_sysinfo));
        }
        else if (strcmp((char *)node->name,SE_XML_NODE_COMM_ARRAY) == 0)
        {
            pstSummary->community_num = load_community_node(node, pstSummary->community, sizeof(pstSummary->community));
        }
        else if (strcmp((char *)node->name,SE_XML_NODE_V3USER_ARRAY) == 0)
        {
            pstSummary->v3user_num = load_v3user_node(node, pstSummary->v3user, sizeof(pstSummary->v3user));
        }
        else if (strcmp((char *)node->name,SE_XML_NODE_RECV_ARRAY) == 0)
        {
            pstSummary->receiver_num = load_trap_receiver_node(node, pstSummary->receiver, sizeof(pstSummary->receiver));
        }

        xmlFree(content);
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlMemoryDump();
    return SE_OK;
}

#endif

static int write_snmp_config(STSNMPSummary *pstSummary, char *file_path)
{
    int i=0;
	int j = 0;
    FILE * fp;
    char sys_oid[MAX_MIB_SYSNAME];
    char File_content[MAX_FILE_CONTENT],syscommand[500],sec_name[50],group_name[50],user_name[50],auth[10],privacy[10],access_command[10],comm_type[10],mib_sys_name[MAX_MIB_SYSNAME],no_auth[10];
	char deny_ip[32] = {0};
	char access_mode[32] = {0};
    memset(File_content,0,MAX_FILE_CONTENT);
    memset(syscommand,0,500);
    memset(sec_name,0,50);
    memset(group_name,0,50);
    memset(user_name,0,50);
    memset(auth,0,10);
    memset(privacy,0,10);
    memset(access_command,0,10);
    memset(comm_type,0,10);
    memset(mib_sys_name , 0 , MAX_MIB_SYSNAME);
    memset(no_auth, 0, 10);
    memset(sys_oid , 0 , MAX_MIB_SYSNAME);

    //int flag1=0,flag2=0,flag3=0;
    if (NULL == file_path || NULL == pstSummary)
    {
        return SE_ERROR;
    }
    if ((fp = fopen(file_path,"w+"))==NULL)
        return SE_ERROR_NEW_FILE_ERR;
    for (i=0 ; i < pstSummary->deny_ip_num; i++)
    {
            sprintf(syscommand , "com2sec no_option %s public \n", pstSummary->deny_ip[i].ip_addr);
            strcat(File_content, syscommand);
            memset(syscommand, 0, 500);
			sprintf(syscommand , "com2sec no_option %s private \n", pstSummary->deny_ip[i].ip_addr);
            strcat(File_content, syscommand);
            memset(syscommand, 0, 500);
			strcat(File_content,"\n\n");      
    }	
    #if 0
	sprintf(syscommand , "group NoOptionGroup v1   no_option \n");
    strcat(File_content, syscommand);
    memset(syscommand, 0, 500);
	sprintf(syscommand , "group NoOptionGroup v2c  no_option \n");
    strcat(File_content, syscommand);
    memset(syscommand, 0, 500);
	sprintf(syscommand , "group NoOptionGroup usm  no_option \n");
    strcat(File_content, syscommand);
    memset(syscommand, 0, 500);
	sprintf(syscommand , "access NoOptionGroup ""     any       noauth    exact  none  none   none \n");
    strcat(File_content, syscommand);
    memset(syscommand, 0, 500);
	strcat(File_content,"\n");	
    #endif
    
    memset(syscommand,0,500);
    sprintf(syscommand , "group_status v1 %s \n",pstSummary->snmp_sysinfo.v1_status ? "enable":"disable");
    strcat(File_content , syscommand);

    memset(syscommand,0,500);
    sprintf(syscommand , "group_status v2c %s \n",pstSummary->snmp_sysinfo.v2c_status ? "enable":"disable");
    strcat(File_content , syscommand);

    memset(syscommand,0,500);
    sprintf(syscommand , "group_status usm %s \n",pstSummary->snmp_sysinfo.v3_status ? "enable":"disable");
    strcat(File_content , syscommand);
    
    strcat(File_content,"\n\n");
    
    for (i=0 ; i < pstSummary->community_num ; i++)
    {
        //////////////com2sec/////////////////////////////////////////////////
        if (pstSummary->community[i].access_mode == ACCESS_MODE_RO)
        {
            sprintf(sec_name , "%s_ReadOnly",pstSummary->community[i].community);
            //strcpy(comm_type , "public" );
        }
        else if (pstSummary->community[i].access_mode == ACCESS_MODE_RW)
        {
            sprintf(sec_name , "%s_ReadWrite",pstSummary->community[i].community);
            //strcpy(comm_type , "private" );
        }

        sprintf(syscommand , "com2sec %s %s/%s %s \n",sec_name,pstSummary->community[i].ip_addr,pstSummary->community[i].ip_mask,pstSummary->community[i].community);
        strcat(File_content, syscommand);
        //////////////group///////////////////////////////////////////////////
        memset(syscommand,0,500);

        if (pstSummary->community[i].access_mode == ACCESS_MODE_RO)
            sprintf(group_name , "%s_ro_grp",pstSummary->community[i].community);
        else if (pstSummary->community[i].access_mode == ACCESS_MODE_RW)
            sprintf(group_name , "%s_rw_grp",pstSummary->community[i].community);

        if (pstSummary->snmp_sysinfo.v1_status == ENABLE_STATUS)
        {
            sprintf(syscommand , "group %s v1 %s \n",group_name,sec_name);
            strcat(File_content , syscommand);
        }

        if (pstSummary->snmp_sysinfo.v2c_status == ENABLE_STATUS)
        {
            sprintf(syscommand , "group %s v2c %s \n",group_name,sec_name);
            strcat(File_content , syscommand);
        }

        if (pstSummary->snmp_sysinfo.v3_status == ENABLE_STATUS)
        {
            sprintf(syscommand , "group %s usm %s \n",group_name,sec_name);
            strcat(File_content , syscommand);
        }
        strcat(File_content,"\n\n");
    }

	//strcat(File_content,"view  all  included  .1  80 \n");
	for (i=0 ; i < pstSummary->view_num; i++)
    {
		memset(syscommand, 0, 500);
		sprintf(syscommand , "view %s %s %s \n",pstSummary->view[i].name,\
												(pstSummary->view[i].view_mode)?"excluded":"included",\
												pstSummary->view[i].oid_tree);
        strcat(File_content , syscommand);    
    }
	strcat(File_content,"\n\n");
	
	//strcat(File_content,"access public_ro_grp   \"\"   any       noauth    exact  all    none   none \n");
	//strcat(File_content,"access private_rw_grp   \"\"   any       noauth    exact  all    all   all \n");
	for(i = 0; i < pstSummary->access_num; i++)
	{
		int mode = 0;
		memset(syscommand, 0 , 500);
		for(j=0; j<pstSummary->community_num; j++)
		{
			if(!strcmp(pstSummary->community[j].community, pstSummary->access[i].accommunity))
			{
				sprintf(access_mode, "%s", (pstSummary->community[j].access_mode)?pstSummary->access[i].acview:"none");
				mode = pstSummary->community[j].access_mode;
			}
		}
		if(mode == 0)
		{
			sprintf(syscommand, "access %s_ro_grp   \"\"   any       noauth    exact  %s    %s   none\n",pstSummary->access[i].accommunity,pstSummary->access[i].acview, access_mode);
			strcat(File_content , syscommand);    
		}
		else if(mode == 1)
		{
			sprintf(syscommand, "access %s_rw_grp   \"\"   any       noauth    exact  %s    %s   none\n",pstSummary->access[i].accommunity,pstSummary->access[i].acview, access_mode);
			strcat(File_content , syscommand);    
		}

	}
    if (pstSummary->snmp_sysinfo.v3_status == ENABLE_STATUS)
    {
        for (i=0 ; i < pstSummary->v3user_num; i++)
        {
            memset(syscommand,0,500);
            memset(no_auth, 0 , 10);
            strcpy(user_name,pstSummary->v3user[i].name);

            if (pstSummary->v3user[i].access_mode == ACCESS_MODE_RO)
            {
                strcpy(access_command,"rouser");
            }
            else if (pstSummary->v3user[i].access_mode == ACCESS_MODE_RW)
            {
                strcpy(access_command,"rwuser");
            }
            else
            {
                strcpy(access_command,"rouser");
            }

            if (pstSummary->v3user[i].authentication.protocal == AUTH_PRO_NONE)
            {
                memset(auth,0,10);
            }
            else if (pstSummary->v3user[i].authentication.protocal == AUTH_PRO_MD5)
            {
                strcpy(auth,"MD5");
            }
            else if (pstSummary->v3user[i].authentication.protocal == AUTH_PRO_SHA)
            {
                strcpy(auth,"SHA");
            }

            if (pstSummary->v3user[i].privacy.protocal== PRIV_PRO_NONE)
            {
                memset(privacy,0,10);
            }
            else if (pstSummary->v3user[i].privacy.protocal == PRIV_PRO_AES)
            {
                strcpy(privacy,"AES");
            }
            else if (pstSummary->v3user[i].privacy.protocal == PRIV_PRO_DES)
            {
                strcpy(privacy,"DES");
            }

            if (!(strcmp(auth,"") || strcmp(privacy,"")))
            {
                strcpy(no_auth, "noauth");
            }

            sprintf(syscommand,"createUser %s %s %s %s %s \n%s %s %s \n",pstSummary->v3user[i].name,\
                    auth,pstSummary->v3user[i].authentication.passwd,\
                    privacy,pstSummary->v3user[i].privacy.passwd,\
                    access_command,pstSummary->v3user[i].name,  \
                    no_auth);
            strcat(File_content , syscommand);
            strcat(File_content,"\n\n");
        }
    }

    /////////////////////get_sys_oid//////////////////////////////////////////////
    app_snmp_system_oid_get(sys_oid);
    memset(syscommand,0,500);
    sprintf(syscommand , "sysobjectid %s\n",sys_oid);
    strcat(File_content,syscommand);
    strcat(File_content,"sysdescr Multi-layer Switch\n");
    strcat(File_content,"\n\n");
    strcat(File_content,"master  agentx \n");
    strcat(File_content,"\n\n");
    memset(syscommand,0,500);
    for (i=0 ; i < pstSummary->receiver_num; i++)
    {
        if (pstSummary->receiver[i].mode == 0)
        {
            sprintf(syscommand , "trapsink %s public %s \n",pstSummary->receiver[i].ip_addr, pstSummary->receiver[i].portno);
            strcat(File_content , syscommand);
        }

        if (pstSummary->receiver[i].mode == 1)
        {
            sprintf(syscommand , "trap2sink %s public %s \n",pstSummary->receiver[i].ip_addr, pstSummary->receiver[i].portno);
            strcat(File_content , syscommand);
        }

        if (pstSummary->receiver[i].mode == 2)
        {
            sprintf(syscommand , "informsink %s public %s \n",pstSummary->receiver[i].ip_addr, pstSummary->receiver[i].portno);
            strcat(File_content , syscommand);
        }

        if (pstSummary->receiver[i].mode == 3)
        {
			if(pstSummary->receiver[i].v3user.authentication.protocal == AUTH_PRO_MD5)
			{
				if(pstSummary->receiver[i].v3user.privacy.protocal == PRIV_PRO_DES)
				{
                    sprintf(syscommand , "trapsess -e %s -v 3 -u %s -a MD5 -A %s -l authPriv -x DES -X %s %s uptime \n",
						pstSummary->receiver[i].engineId, pstSummary->receiver[i].v3user.name, 
						pstSummary->receiver[i].v3user.authentication.passwd, pstSummary->receiver[i].v3user.privacy.passwd, 
						pstSummary->receiver[i].ip_addr);
				}
				else if(pstSummary->receiver[i].v3user.privacy.protocal == PRIV_PRO_AES)
				{
                    sprintf(syscommand , "trapsess -e %s -v 3 -u %s -a MD5 -A %s -l authPriv -x AES -X %s %s uptime \n",
						pstSummary->receiver[i].engineId, pstSummary->receiver[i].v3user.name, 
						pstSummary->receiver[i].v3user.authentication.passwd, pstSummary->receiver[i].v3user.privacy.passwd, 
						pstSummary->receiver[i].ip_addr);
				}
				else
				{
                    sprintf(syscommand , "trapsess -e %s -v 3 -u %s -a MD5 -A %s -l authNoPriv %s uptime \n",
						pstSummary->receiver[i].engineId, pstSummary->receiver[i].v3user.name, 
						pstSummary->receiver[i].v3user.authentication.passwd, 
						pstSummary->receiver[i].ip_addr);
				}
			}
			else if(pstSummary->receiver[i].v3user.authentication.protocal == AUTH_PRO_SHA)
			{
				if(pstSummary->receiver[i].v3user.privacy.protocal == PRIV_PRO_DES)
				{
                    sprintf(syscommand , "trapsess -e %s -v 3 -u %s -a SHA -A %s -l authPriv -x DES -X %s %s uptime \n",
						pstSummary->receiver[i].engineId, pstSummary->receiver[i].v3user.name, 
						pstSummary->receiver[i].v3user.authentication.passwd, pstSummary->receiver[i].v3user.privacy.passwd, 
						pstSummary->receiver[i].ip_addr);
				}
				else if(pstSummary->receiver[i].v3user.privacy.protocal == PRIV_PRO_AES)
				{
                    sprintf(syscommand , "trapsess -e %s -v 3 -u %s -a SHA -A %s -l authPriv -x AES -X %s %s uptime \n",
						pstSummary->receiver[i].engineId, pstSummary->receiver[i].v3user.name, 
						pstSummary->receiver[i].v3user.authentication.passwd, pstSummary->receiver[i].v3user.privacy.passwd, 
						pstSummary->receiver[i].ip_addr);
				}
				else
				{
                    sprintf(syscommand , "trapsess -e %s -v 3 -u %s -a SHA -A %s -l authNoPriv %s uptime \n",
						pstSummary->receiver[i].engineId, pstSummary->receiver[i].v3user.name, 
						pstSummary->receiver[i].v3user.authentication.passwd, 
						pstSummary->receiver[i].ip_addr);
				}
			}
			else
			{
                sprintf(syscommand , "trapsess -e %s -v 3 -u %s -l noAuthNoPriv %s uptime \n",
					pstSummary->receiver[i].engineId, pstSummary->receiver[i].v3user.name, 
					pstSummary->receiver[i].ip_addr);
			}
            strcat(File_content , syscommand);
        }

        strcat(File_content,"\n\n");
    }
    strcat(File_content,"\n\n");
    strcat(File_content,"agentSecName internal \n");
    strcat(File_content,"rouser internal \n");
    strcat(File_content,"linkUpDownNotifications yes \n");
    strcat(File_content,"authtrapenable 1 \n");	
    //strcat(File_content,"dlmod subagent_plugin /opt/lib/subagent_plugin.so \n");
#if 0/*del some trap which commer mib send*/
    strcat(File_content,"linkUpDownNotifications yes \n");
    strcat(File_content,"defaultMonitors yes \n");
#endif
    strcat(File_content,"\n\n");
    fwrite(File_content,strlen(File_content),1,fp);
    fclose(fp);
    return SE_OK;
}

#if 0
/*show_sys_ver in ws_sysinfo.c and dcli_system define conflict*/
int show_sys_ver(struct sys_ver *SysV)
{
    DBusMessage *query, *reply;
    DBusError err;
    int retu=0;
    unsigned int product_id;
    unsigned int sw_version;
    char *product_name = NULL;
    char *base_mac = NULL;
    char *serial_no = NULL;
    char *swname = NULL;
    query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_VER);
    dbus_error_init(&err);
    reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
        }

        return -1;
    }

    if (dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&product_id,
                              DBUS_TYPE_UINT32,&sw_version,
                              DBUS_TYPE_STRING,&product_name,
                              DBUS_TYPE_STRING,&base_mac,
                              DBUS_TYPE_STRING,&serial_no,
                              DBUS_TYPE_STRING,&swname,
                              DBUS_TYPE_INVALID))
    {
        memset(SysV->swname,0,50);
        strcpy(SysV->swname,swname);
        SysV->sw_version=sw_version;
        //add 2009.2.27
        SysV->product_id=product_id;
        //changed 2008.6.26.pm
        //if (SW_COMPATIBLE_VER(sw_version) > 0 ) {}
        memset(SysV->product_name,0,50);
        strcpy(SysV->product_name,product_name);
        memset(SysV->base_mac,0,50);
        strcpy(SysV->base_mac,base_mac);
        memset(SysV->serial_no,0,50);
        strcpy(SysV->serial_no,serial_no);
        //software name
        {
            char software_name_file[256]="";

            if (access("/devinfo/software_name",0)==0)
            {
                sprintf(software_name_file, "if [ -f /devinfo/software_name ];then cat /devinfo/software_name 2>/dev/null;fi");
                GET_CMD_STDOUT(SysV->sw_name,sizeof(SysV->sw_name),software_name_file);
            }
        }
        //product name
        {
            char product_name_file[256]="";

            if (access("/devinfo/product_name",0)==0)
            {
                sprintf(product_name_file, "if [ -f /devinfo/product_name ];then cat /devinfo/product_name 2>/dev/null;fi");
                GET_CMD_STDOUT(SysV->sw_product_name,sizeof(SysV->sw_product_name),product_name_file);
            }
        }
        //version
        {
            char get_sw_ver_cmd[256]="";

            if (access("/etc/version/verstring",0)==0)
            {
                sprintf(get_sw_ver_cmd, "if [ -f /etc/version/verstring ];then cat /etc/version/verstring 2>/dev/null;fi");
                GET_CMD_STDOUT(SysV->sw_version_str,sizeof(SysV->sw_version_str),get_sw_ver_cmd);
            }
        }
        //mac address
        {
            char mac_address_file[256]="";

            if (access("/devinfo/mac",0)==0)
            {
                sprintf(mac_address_file, "if [ -f /devinfo/mac ];then cat /devinfo/mac 2>/dev/null;fi");
                GET_CMD_STDOUT(SysV->sw_mac,sizeof(SysV->sw_mac),mac_address_file);
            }
        }
        //serial number
        {
            char serial_num_file[256]="";

            if (access("/devinfo/sn",0)==0)
            {
                sprintf(serial_num_file, "if [ -f /devinfo/sn ];then cat /devinfo/sn 2>/dev/null;fi");
                GET_CMD_STDOUT(SysV->sw_serial_num,sizeof(SysV->sw_serial_num),serial_num_file);
            }
        }
    }
    else
    {
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
        }

        retu=-1;
    }

    dbus_message_unref(reply);
    return retu;
}
#endif


static int init_sys_info_to_default(STSNMPSysInfo *pstSnmpSysInfo)

{
    int iRet = 0;
    struct product_info info;
    memset(&info, 0, sizeof(struct product_info));
    config_dbus_connection_init();
    show_product_info(&info);
    strncpy(pstSnmpSysInfo->sys_name, info.name, strlen(info.name));
    app_snmp_system_oid_get(pstSnmpSysInfo->sys_oid);
    strcpy(pstSnmpSysInfo->sys_description, "SYSTEM");
    return SE_OK;
}
static int get_sys_non_conf_info(STSNMPSysInfo *pstSnmpSysInfo)
{
#if 0
#define SNMP_ENGINE_VERSION			0x00000001
#define SNMP_ENGINE_VERSION_MASK	0xffffff00
#define SNMP_ENGINE_MAX_INFO_LEN	64
#define SNMP_ENGINE_INFO_STR		"AuteCS snmp agent web config engine data"
#endif
    init_sys_info_to_default(pstSnmpSysInfo);
    FILE * fp = NULL;
    char buf[HOST_NAME_LENTH];
    fp = popen("hostname -v","r");

    if (fp != NULL)
    {
        fgets(buf,HOST_NAME_LENTH,fp);
        pclose(fp);
    }

    memset(pstSnmpSysInfo->sys_description, 0, sizeof(pstSnmpSysInfo->sys_description));
    memcpy(pstSnmpSysInfo->sys_description, buf, strlen(buf)-1);
    return SE_OK;
}

#if 0
char * get_sysOID()
{
    char * sysOid = (char *)malloc(MAX_OID_LENTH);
    memset(sysOid , 0 , MAX_OID_LENTH);
    int i , product_id = 0;
    char  ch_product_id[20] = {0};
    char * endptr = NULL;
    FILE * fp = NULL;

    if(0 == is_file_exist("/proc/product_id"))
    {
        if (NULL != (fp = popen("cat /proc/product_id", "r")))
        {
            fgets(ch_product_id, 20, fp);
            product_id = strtoul(ch_product_id, &endptr, 0);
            pclose(fp);
        }
    }
    for (i = 0; i < 5; i++)
    {
        if (product_id == snmp_product_type[i].value)
        {
            sprintf(sysOid, "%s.%s", snmp_product_type[i].product_type, snmp_product_type[i].product_node);
        }
    }
    return sysOid;
}

static char * get_product_name()
{
    struct sys_ver ptrsysver;
    ptrsysver.product_name=(char *)malloc(50);
    ptrsysver.base_mac=(char *)malloc(50);
    ptrsysver.serial_no=(char *)malloc(50);
    ptrsysver.swname=(char *)malloc(50);
    config_dbus_connection_init();
    show_sys_ver(&ptrsysver);
    char * product_name = (char *)malloc(MAX_SYSTEM_NAME_LEN);
    memset(product_name , 0 , MAX_SYSTEM_NAME_LEN);
    strcpy(product_name , ptrsysver.sw_product_name);
    free(ptrsysver.product_name);
    free(ptrsysver.base_mac);
    free(ptrsysver.serial_no);
    free(ptrsysver.swname);
    return product_name;
}

int set_trap_ip_addr(char * ip_addr)
{
    if (NULL == ip_addr)
        return -1;

    STSNMPTrapReceiver * ptrap_rev = NULL;
    int index = 0;
    ptrap_rev = (STSNMPTrapReceiver *)malloc(sizeof(STSNMPTrapReceiver));
    memset(ptrap_rev , 0 , sizeof(STSNMPTrapReceiver));
    ptrap_rev->status= 0;
    strcpy(ptrap_rev->name, "default");
    strncpy(ptrap_rev->ip_addr , ip_addr , 20);
    snmp_conf_init();
    modify_trap_receiver(ptrap_rev , index);
    snmp_conf_save();
    free(ptrap_rev);
    return 0;
}
#endif
#if 0
char * get_sysOID_OEM_compatible()
{
    FILE * fp = NULL;
	char product_type[8] = {0};
    char * enterprise_part=(char *)malloc(ENTERPRISE_OID_LENGTH);
    char * product_OID=(char *)malloc(PRODUCT_OID_LENGTH);
    char * start_oid = ".1.3.6.1.4.1";
    char * ret_oid = (char *)malloc(TOTAL_OID_LENGTH);
    int len;
	
    memset(ret_oid, 0, TOTAL_OID_LENGTH);
	memset(product_OID, 0, PRODUCT_OID_LENGTH);
    memset(enterprise_part, 0, ENTERPRISE_OID_LENGTH);

    if(0 == is_file_exist(ENTERPRISE_NODE_PATH))
    {
        if (NULL != (fp = fopen(ENTERPRISE_NODE_SH,"r")))
        {
            fgets(enterprise_part, ENTERPRISE_OID_LENGTH, fp);
            pclose(fp);
        }
    }
    len = strlen(enterprise_part);
    if (enterprise_part[len-1] == '\n')
        enterprise_part[len-1] = '\0';
    if ((!strcmp(enterprise_part,"0")) || (!strcmp(enterprise_part,"")) || (CHECK_INPUT_DIGIT(enterprise_part) == -2))
    {
        strcpy(enterprise_part,ENTERPRISE_OID);
    }
	
    if(0 == is_file_exist(PRODUCT_NODE_PATH))
    {
        if (NULL != (fp = fopen(PRODUCT_NODE_SH,"r")))
        {
            fgets(product_OID, ENTERPRISE_OID_LENGTH, fp);
            pclose(fp);
        }
    }
    len = strlen(product_OID);
    if (product_OID[len-1] == '\n')
        product_OID[len-1] = '\0';
    if ((!strcmp(product_OID,"0")) || (!strcmp(product_OID,"")) || (CHECK_INPUT_DIGIT(product_OID) == -2))
    {
        strcpy(product_OID,DEVICEINFO);
    }	

	if(0 == is_file_exist(PRODUCT_TYPE__NODE_PATH))
    {
        if (NULL != (fp = fopen(PRODUCT_TYPE__NODE_PATH,"r")))
        {
            fgets(product_type, ENTERPRISE_OID_LENGTH, fp);
            pclose(fp);
        }
    }
    len = strlen(product_type);
    if (product_type[len-1] == '\n')
        product_type[len-1] = '\0';	

	
	if ((!strcmp(product_type,"")) || (CHECK_INPUT_DIGIT(product_type) == -2))
    {
        strcpy(product_type,"0");
    }

    if ((!strcmp(product_type,"0")) || (!strcmp(product_type,"")))
        sprintf(ret_oid, "%s.%s.%s",start_oid,enterprise_part,product_OID);
    else
        sprintf(ret_oid, "%s.%s.%s.%s",start_oid,enterprise_part,product_OID,product_type);

    free(product_OID);
    free(enterprise_part);
    return ret_oid;
}
#endif
char* get_token_snmp(char *str_tok)
{
    unsigned int temp = 0;

    while (isspace(*(str_tok+temp)) && (*(str_tok+temp) != '\0'))
    {
        temp++;
    }

    return (char *)(str_tok+temp);
}
#if 0
int snmp_xml_modify_for_mib(STSNMPTrapReceiver *pstTrapReceiver,int index)
{
    if (pstTrapReceiver == NULL)
        return -1;

    snmp_conf_init();
    modify_trap_receiver(pstTrapReceiver , index);
    snmp_conf_save();
}

int snmp_xml_read_for_mib(STSNMPTrapReceiver *pstTrapReceiver,int * trap_num)
{
    if (pstTrapReceiver == NULL)
        return -1;

    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *node = NULL;
    char *content;
    int max_trap_num = MAX_TRAPRECEIVER_NUM;
    int i;
    //snmp_conf_init();
    snmp_conf_save();
    doc = xmlReadFile(XML_FILE_PATH, NULL, 0);

    if (doc == NULL)
    {
        return -1;
    }

    root_element = xmlDocGetRootElement(doc);

    //fprintf(stderr,"444444444444444");
    for (node = root_element->children; node; node = node->next)
    {
        content = (char *)xmlNodeGetContent(node);

        if ((content == NULL)||(strlen(content) == 0))
        {
            if (NULL != content) xmlFree(content);

            continue;
        }

        //fprintf(stderr,"name=%s",(char *)node->name);
        if (strcmp((char *)node->name,SE_XML_NODE_RECV_ARRAY) == 0)
        {
            *trap_num = load_trap_receiver_node(node, pstTrapReceiver, sizeof(STSNMPTrapReceiver));
        }

        xmlFree(content);
        //fprintf(stderr,"rpstTrapReceiver0=%s--trap_num=%d",pstTrapReceiver[0].name,*trap_num);
    }

    xmlFreeDoc(doc);
}
#endif

int Generate_default_snmpd_option(STSNMPSummary *pstSummary)
{
    STCommunity * comm = NULL;
    comm = (STCommunity *)malloc(sizeof(STCommunity));
    strcpy(comm->community , "public");
    strcpy(comm->ip_addr , "0.0.0.0");
    strcpy(comm->ip_mask , "0.0.0.0");
    comm->access_mode = 0;
    add_community(comm);
    strcpy(comm->community , "private");
    comm->access_mode = 1;
    add_community(comm);
    snmp_conf_save();
    free(comm);
    return 0;
}

int CHECK_INPUT_DIGIT(char * src)
{
    if (src == NULL)
    {
        return -1;
    }

    char * str = src;
    int i = 0;

    while (str[i] != '\0')
    {
        if (isspace(str[i]))
        {
            i++;
        }

        if (!isdigit(str[i]))
        {
            return -2;
        }

        i++;
    }

    return 0;
}


void free_hw_head(hw_info * hw_head)
{
    hw_info *f1,*f2;
    f1=hw_head->next;
    f2=f1->next;

    while (f2!=NULL)
    {
        free(f1);
        f1=f2;
        f2=f2->next;
    }

    free(f1);
}


int get_mem_usage(P_SYS_INFO pst_sys_info)
{
    FILE *fp = NULL;
    char buffer[BUF_LEN];
    char *iter = NULL;
    unsigned int begin = 0;
    memset(buffer, 0, sizeof(buffer));

    if (NULL != (fp = fopen(cp_mem_info,"r")))
    {
        if (NULL == fgets(buffer, BUF_LEN, fp))
        {
            return SYS_ERR;
        }

        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_total = strtoul(iter, &iter, 0);

        if (NULL == fgets(buffer, BUF_LEN, fp))
        {
            return SYS_ERR;
        }

        begin = 0;
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_free = strtoul(iter, &iter, 0);

        if (NULL == fgets(buffer, BUF_LEN, fp))
        {
            return SYS_ERR;
        }

        begin = 0;
        get_token(buffer, &begin);
        iter = buffer + begin;
        pst_sys_info->pst_mem_status->un_mem_buffers = strtoul(iter, &iter, 0);
        pst_sys_info->pst_mem_status->un_mem_used = pst_sys_info->pst_mem_status->un_mem_total - pst_sys_info->pst_mem_status->un_mem_free;
        fclose(fp);
        return SUCCESS;
    }

    return SYS_ERR;
}



int show_sample_info(wid_sample_info * info)
{
    DBusMessage *query, *reply;
    DBusMessageIter	 iter;
    DBusError err;
    wid_sample_info sampleinfo;
    sampleinfo.monitor_switch = 0;
    sampleinfo.monitor_time = 0;
    sampleinfo.sample_switch = 0;
    sampleinfo.sample_time = 0;
    /*char en[] = "enable";
    char dis[] = "disable";*/
    int ret = WID_RETURN_CODE_SUCCESS;
    query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
                                         WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO);
    dbus_error_init(&err);
    reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
        }

        return -1;
    }

    dbus_message_iter_init(reply,&iter);
    dbus_message_iter_get_basic(&iter,&ret);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&sampleinfo.monitor_switch);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&sampleinfo.monitor_time);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&sampleinfo.sample_switch);
    dbus_message_iter_next(&iter);
    dbus_message_iter_get_basic(&iter,&sampleinfo.sample_time);
    info->monitor_switch = sampleinfo.monitor_switch;
    info->sample_switch = sampleinfo.sample_switch;
    info->monitor_time = sampleinfo.monitor_time;
    info->sample_time =sampleinfo.sample_time;
#if 0
    printf("==============================================================================\n");
    printf("Wireless sample infomation\n");
    printf("monitor_switch:	%s\n",(sampleinfo.monitor_switch == 0)?dis:en);
    printf("sample_switch:	%s\n",(sampleinfo.sample_switch == 0)?dis:en);
    printf("monitor_time:	%d s\n",sampleinfo.monitor_time);
    printf("sample_time:	%d s\n",sampleinfo.sample_time);
    printf("==============================================================================\n");
#endif
    dbus_message_unref(reply);
    return 0;
}






int get_cpu_status(unsigned int *cpu_times, unsigned int *count)
{
    FILE *fp = NULL;
    char buffer[BUF_LEN];
    char *iter = NULL;
    unsigned int begin = 0;
    unsigned int cpu_no = 0;
    unsigned int cpu_status = 0;
    (*count) = 0;
    memset(buffer, 0, sizeof(buffer));
    memset(cpu_times, 0, sizeof(unsigned int)*MAX_CPU*CPU_STATUS);

    if (NULL != (fp = fopen(cp_cpu_info,"r")))
    {
        cpu_no = 0;
        cpu_status = 0;

        while ((NULL != fgets(buffer, BUF_LEN, fp)) && cpu_no < MAX_CPU)
        {
            iter = buffer;
            begin = 0;
            cpu_status = 0;

            if (buffer[begin]=='c' && buffer[begin+1]=='p' && buffer[begin+2]=='u')
            {
                if (buffer[begin+3]!=' ')
                {
                    (*count)++;
                    get_token(buffer, &begin);
                    iter += begin;

                    while (cpu_status < CPU_STATUS)
                    {
                        *(cpu_times+cpu_no*CPU_STATUS+(cpu_status++)) = strtoul(iter, &iter, 0);
                    }

                    cpu_no ++;
                }
            }
            else
            {
                break;
            }
        }

        fclose(fp);
        return SUCCESS;
    }

    return SYS_ERR;
}



int calc_cpu_rate(unsigned int time_old[MAX_CPU][CPU_STATUS],
                  unsigned int time_now[MAX_CPU][CPU_STATUS],
                  int cpu_num, unsigned int *cpu_rate,
                  unsigned int *cpu_rate_all)
{
    unsigned int cpu_no = 0;
    unsigned int cpu_stu = 0;
    unsigned int cpu_total[MAX_CPU];
    unsigned int cpu_total_all = 0;
    unsigned int cpu_use[MAX_CPU];
    unsigned int cpu_use_all = 0;
    unsigned int cpu_idle_all = 0;
#if 0
    memset(cpu_total, 0, sizeof(cpu_total));
    memset(cpu_use, 0, sizeof(cpu_use));

    for (cpu_no = 0; cpu_no<cpu_num; cpu_no++)
    {
        for (cpu_stu=0; cpu_stu<CPU_STATUS; cpu_stu++)
        {
            cpu_total[cpu_no] += (time_now[cpu_no][cpu_stu] - time_old[cpu_no][cpu_stu]);
        }

        cpu_total_all += cpu_total[cpu_no];

        for (cpu_stu=0; cpu_stu<3; cpu_stu++)
        {
            cpu_use[cpu_no] += (time_now[cpu_no][cpu_stu] - time_old[cpu_no][cpu_stu]);
        }

        cpu_use_all += cpu_use[cpu_no];
        cpu_rate[cpu_no] = cpu_use[cpu_no]*100/cpu_total[cpu_no];
    }

    if (0 == cpu_total_all)
    {
        *cpu_rate_all = 1;
    }
    else
    {
        *cpu_rate_all = cpu_use_all*100/cpu_total_all;
    }

#endif

    for (cpu_stu=0; cpu_stu<CPU_STATUS; cpu_stu++)
    {
        cpu_total_all += (time_now[0][cpu_stu] - time_old[0][cpu_stu]);
    }

    cpu_idle_all = (time_now[0][3] - time_old[0][3]);
    cpu_use_all = cpu_total_all - cpu_idle_all;

    if (0 == cpu_total_all)
    {
        *cpu_rate_all = 1;
    }
    else
    {
        *cpu_rate_all = (cpu_use_all*100)/cpu_total_all;
    }

    return 0;
}

ifi_info *get_ifi_info(int family, int doaliases)
{
    ifi_info *ifi, *ifihead, **ifipnext;
    int sockfd, len, lastlen, flags, myflags;
    char *ptr, *buf, lastname[IFNAMSIZ], *cptr;
    struct ifconf ifc;
    struct ifreq *ifr, ifrcopy;
    struct sockaddr_in *sinptr;

    if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        printf("socket error.\n");
        exit(1);
    }

    lastlen = 0;
    len = 10*sizeof(struct ifreq);

    while (1)
    {
        buf = (char*)malloc(len);
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;

        if (ioctl(sockfd, SIOCGIFCONF, &ifc)<0)
        {
            if (errno!=EINVAL||lastlen!=0)
            {
                printf("ioctl error.\n");
            }
        }
        else
        {
            if (ifc.ifc_len == lastlen)
                break;

            lastlen = ifc.ifc_len;
        }

        len += 10*sizeof(struct ifreq);
        free(buf);
    }

    ifihead = NULL;
    ifipnext = &ifihead;
    lastname[0] = 0;

    for (ptr = buf; ptr<buf+ifc.ifc_len;)
    {
        ifr = (struct ifreq*)ptr;
#ifdef HAVE_SOCKADDR_SA_LEN
        len = sizeof(struct sockaddr)>ifr->ifr_addr.sa_len?sizeof(struct sockaddr):ifr->ifr_addr.sa_len;
#else

        switch (ifr->ifr_addr.sa_family)
        {
#ifdef IPV6
            case:
AF_INET6:
                LEN = sizeof(struct sockaddr_in6);
                break;
#endif
            case AF_INET:
            default:
                len = sizeof(struct sockaddr);
                break;
        }

#endif
        ptr += sizeof(ifr->ifr_name) + len;

        if (ifr->ifr_addr.sa_family != family)
            continue;

        myflags = 0;

        if ((cptr=strchr(ifr->ifr_name, ':'))!=NULL)
            *cptr = 0;

        if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ)==0)
        {
            if (doaliases == 0)
                continue;

            myflags = IFI_ALIAS;
        }

        memcpy(lastname, ifr->ifr_name, IFNAMSIZ);
        ifrcopy = *ifr;
        ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
        flags = ifrcopy.ifr_flags;
        /* if ((flags&IFF_UP)==0)
           continue;

         if ((flags&IFF_BROADCAST)==0)
           continue;
         */
        ifi = calloc(1, sizeof(struct ifi_info));
        *ifipnext = ifi;
        ifipnext = &ifi->ifi_next;
        ifi->ifi_flags = flags;
        ifi->ifi_myflags = myflags;
        memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
        ifi->ifi_name[IFI_NAME-1] = '\0';

        switch (ifr->ifr_addr.sa_family)
        {
            case AF_INET:
                sinptr = (struct sockaddr_in *)&ifr->ifr_addr;

                if (ifi->ifi_addr == NULL)
                {
                    ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in));
                    memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));
#ifdef SIOCGIFBRDADDR

                    if (flags & IFF_BROADCAST)
                    {
                        ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
                        sinptr = (struct sockaddr_in *)&ifrcopy.ifr_broadaddr;
                        ifi->ifi_brdaddr = calloc(1, sizeof(struct sockaddr_in));
                        memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));
                    }

#endif
#ifdef SIOCGIFDSTADDR

                    if (flags & IFF_POINTOPOINT)
                    {
                        ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy);
                        sinptr = (struct sockaddr_in*)&ifrcopy.ifr_dstaddr;
                        ifi->ifi_dstaddr = calloc(1, sizeof(struct sockaddr_in));
                        memcpy(ifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in));
                    }

#endif
#ifdef SIOCGIFNETMASK
//if (flags & IFF_BROADCAST)
                    {
                        ioctl(sockfd, SIOCGIFNETMASK, &ifrcopy);
                        sinptr = (struct sockaddr_in *)&ifrcopy.ifr_netmask;
                        ifi->ifi_mask= calloc(1, sizeof(struct sockaddr_in));
                        memcpy(ifi->ifi_mask, sinptr, sizeof(struct sockaddr_in));
                    }
#endif
//////////////
                }

                break;
            default:
                break;
        }
    }

    free(buf);
    return(ifihead);
}

void free_ifi_info(ifi_info *ifihead)
{
    ifi_info *ifi, *ifinext;

    for (ifi=ifihead; ifi!=NULL; ifi=ifinext)
    {
        if (ifi->ifi_addr!=NULL)
            free(ifi->ifi_addr);

        if (ifi->ifi_brdaddr!=NULL)
            free(ifi->ifi_brdaddr);

        if (ifi->ifi_dstaddr!=NULL)
            free(ifi->ifi_dstaddr);

        if (ifi->ifi_mask!=NULL)
            free(ifi->ifi_mask);

        ifinext = ifi->ifi_next;
        free(ifi);
    }
}

char *sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char portstr[7];
    static char str[128];

    switch (sa->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in *sin = (struct sockaddr_in *)sa;

            if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str))==NULL)
                return NULL;

            if (ntohs(sin->sin_port)!=0)
            {
                snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin_port));
                strcat(str, portstr);
            }

            return str;
        }
        break;
        case AF_INET6:
        {
            struct sockaddr_in6 *sin = (struct sockaddr_in6 *)sa;

            if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str))==NULL)
                return NULL;

            if (ntohs(sin->sin6_port)!=0)
            {
                snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin6_port));
                strcat(str, portstr);
            }

            return str;
        }
        break;
        default:
            return NULL;
            break;
    }
}

int interface_list_ioctl(int af,struct inf * interface)
{
    ifi_info *ifi, *ifihead;
    infi *p_tail = NULL;
    struct sockaddr *sa;
    u_char *ptr;
    int i, family, doaliases;

    if (af == 0)
        family = AF_INET;
    else if (af == 1)
        family =AF_INET6;
    else
    {
        printf("invalid <address-family>");
        exit(1);
    }

    interface->next = NULL;
    p_tail = interface;
// infi *p_head = NULL;

    for (ifihead = ifi = get_ifi_info(family, doaliases);
            ifi!=NULL; ifi=ifi->ifi_next)
    {
        infi * p = (infi *)malloc(sizeof(infi));
        memset(p,0,sizeof(infi));
        strcpy(p->if_name,ifi->ifi_name);

        // printf("%s:<", ifi->ifi_name);
        if (ifi->ifi_flags&IFF_UP)
        {
            //	printf("UP");
            strcpy(p->if_stat,"UP");
            p->upflag=1;
        }
        else
        {
            //	printf("DOWN");
            strcpy(p->if_stat,"DOWN");
            p->upflag=0;
        }

        //  if (ifi->ifi_flags&IFF_BROADCAST) printf("BCAST");
        // if (ifi->ifi_flags&IFF_MULTICAST) printf("MCAST");
        // if (ifi->ifi_flags&IFF_LOOPBACK) printf("LOOP");
        // if (ifi->ifi_flags&IFF_POINTOPOINT) printf("P2P");
        // printf(">\n");

        if ((i=ifi->ifi_hlen)>0)
        {
            ptr = ifi->ifi_haddr;

            do
            {
                // printf("%s%x", (i==ifi->ifi_hlen)?" ":":", *ptr++);
            }
            while (--i>0);

            printf("\n");
        }

        if ((sa=ifi->ifi_addr)!=NULL)
        {
            // printf(" IP addr: %s\n",
            //        sock_ntop(sa, sizeof(*sa)));
            strcpy(p->if_addr,sock_ntop(sa, sizeof(*sa)));
        }

        if ((sa=ifi->ifi_brdaddr)!=NULL)
            {}

        // printf(" broadcast addr: %s\n",
        //        sock_ntop(sa, sizeof(*sa)));
        if ((sa=ifi->ifi_dstaddr)!=NULL)
            {}

        // printf(" destnation addr: %s\n",
        //       sock_ntop(sa, sizeof(*sa)));
        //fprintf(stderr,"zhouym : before netmask:\n");

        if ((sa=ifi->ifi_mask)!=NULL)
        {
            //fprintf(stderr,"zhouym : enter netmask:\n");
            strcpy(p->if_mask,sock_ntop(sa, sizeof(*sa)));
        }

        p -> next = NULL;
        p_tail -> next = p;
        p_tail = p;
    }

    free_ifi_info(ifihead);
}

void free_inf(infi * infter)
{
    infi * f1,*f2;
    f1 = infter->next;
    f2 = f1->next;

    while (f2!=NULL)
    {
        free(f1);
        f1 = f2;
        f2 = f2->next;
    }

    free(f1);
}

#if 0
t_conf_item *find_conf_item_by_name(char *name, t_conf_item pt_conf_item[], int max_num)
{
    int i=0;

    for (i=0; i<max_num; i++)
    {
        if (strlen(name)==strlen(pt_conf_item[i].conf_name) && strcmp(name, pt_conf_item[i].conf_name) == 0)
        {
            return &(pt_conf_item[i]);
        }
    }

    return NULL;
}



int load_conf_file_ex(char *file_path, t_conf_item pt_conf_item[], int max_num)
{
    FILE *fp;
    int i,j;
    char line[MAX_CONF_NAME_LEN+MAX_CONF_VALUE_LEN+20]="";
    char *div;
    t_conf_item *p_cur_item;
    char name[MAX_CONF_NAME_LEN+1];
    fp = fopen(file_path, "r");

    if (NULL == fp)
    {
        //fprintf( stderr, "load_conf_file    file %s open err!\n", file_path );
        return RTN_ERR_FILE_OPEN;
    }

    fprintf(stderr, "load_conf_file_ex    file %s open ok!\n", file_path);
    j=0;

    while (!feof(fp))
    {
        fgets(line, sizeof(line), fp)	;
        fprintf(stderr, "load_conf_file    get line : %s\n", line);

        for (i=0; i<strlen(line); (0x0a==line[i]||0x0d==line[i])?line[i]=0:0,i++);

        div=strchr(line, '=');  //

        if (NULL == div)
        {
            continue;
        }

        *div = 0;
        div++;
        memset(name, 0, sizeof(name));
        strncpy(name, line, MIN(div-line,sizeof(name)));
        //fprintf( stderr, "load_conf_file    name = %s\n", name );
        p_cur_item = find_conf_item_by_name(name, pt_conf_item, max_num);

        if (NULL != p_cur_item)
        {
            continue;
        }

        strncpy(pt_conf_item[j].conf_name, name, sizeof(pt_conf_item[j].conf_name));
        strncpy(pt_conf_item[j].conf_value, div, sizeof(pt_conf_item[j].conf_value));
        fprintf(stderr, "load_conf_file_ex %s=%s\n", pt_conf_item[j].conf_name, pt_conf_item[j].conf_value);
        j++;
    }

    fclose(fp);
    return 0;
}





int restart_syslog()
{
	int iRet,status;	
	status = system( S_PATH" "S_RESTART);
	iRet = WEXITSTATUS(status);
	
	return iRet;
}


int man_restart_ntp()
{
	int iRet,status;	
	status = system(N_PATH" "N_RESTART);
	iRet = WEXITSTATUS(status);
	
	return iRet;
}

inline void man_reset_sigmask()
{
    sigset_t psmask;
    memset(&psmask,0,sizeof(psmask));
    sigprocmask(SIG_SETMASK,&psmask,NULL);
}

int write_config(ST_SYS_ALL *sysall, char *file_path)
{
    FILE * fp;
    char File_content[50000],des_name[50],content[1024];
    ST_LOG_KEY keys;
    int i;
    memset(File_content,0,50000);
    memset(des_name,0,50);
    memset(&keys,0,sizeof(keys));

    if (NULL == file_path)
    {
        return -1 ;
    }

    if ((fp = fopen(file_path,"w+"))==NULL)
        return -2;

    for (i=0; i<sysall->opt_num; i++)
    {
        memset(content,0,1024);
        sprintf(content , " options { \n %s };\n\n",((sysall->optinfo[i]).content));
        strcat(File_content,content);
    }

    for (i=0; i<(sysall->opt_num); i++)
    {
        free((sysall->optinfo[i]).content);
    }

    for (i=0; i<sysall->su_num; i++)
    {
        memset(des_name,0,50);
        sprintf(des_name , "source  %s { \n",sysall->suinfo[i].suname);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " %s \n };\n\n",sysall->suinfo[i].content);
        strcat(File_content,content);
    }

    for (i=0; i<sysall->des_num; i++)
    {
        memset(des_name,0,50);
        sprintf(des_name , "destination %s",sysall->desinfo[i].rname);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " {%s};\n\n",sysall->desinfo[i].content);
        strcat(File_content,content);
    }

    for (i=0; i<sysall->f_num; i++)
    {
        memset(des_name,0,50);
        sprintf(des_name , "filter %s",sysall->finfo[i].fname);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " {%s};\n\n",sysall->finfo[i].content);
        strcat(File_content,content);
    }

    for (i=0; i<sysall->log_num; i++)
    {
        memset(des_name,0,50);
        sprintf(des_name , "log { \n source(%s); \n",sysall->loginfo[i].source);
        strcat(File_content,des_name);
        /* multiple keys */
        memset(content,0,1024);
        memset(des_name,0,50);
        cut_up(sysall->loginfo[i].dest, &keys,S_DEST);
        sprintf(content , " %s(%s);\n %s\n };\n\n",S_FILT,sysall->loginfo[i].filter, keys.key);
        strcat(File_content,content);
    }

    ////////////////////////////////////////////////////////
    fwrite(File_content,strlen(File_content),1,fp);
    fclose(fp);
    return 0;
}

int read_filter(char * name,char * node_name, ST_SYS_ALL *sysall)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    memset(sysall, 0, sizeof(ST_SYS_ALL));
    pdoc = xmlReadFile(name,"utf-8",256);

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    xmlChar *value;
    int i=0,j=0,m=0,n=0,p=0;
    pcurnode=pcurnode->xmlChildrenNode;

    while (pcurnode != NULL)
    {
        //options
        if ((!xmlStrcmp(pcurnode->name, BAD_CAST  NODE_OPT)))
        {
            value = xmlNodeGetContent(pcurnode);
            sysall->optinfo[n].content=(char *)malloc(xmlStrlen(value));
            strcpy(sysall->optinfo[n].content,(char *)value);
            xmlFree(value);
            n++;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_SOURCE)))
        {
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->suinfo[p].suname,(char *)value);
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->suinfo[p].content,(char *)value);
                    xmlFree(value);
                }

                testnode = testnode->next;
            }

            p++;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_DES)))
        {
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->desinfo[i].rname,(char *)value);
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_CONTENT)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->desinfo[i].content,(char *)value);
                    xmlFree(value);
                }

                testnode = testnode->next;
            }

            i++;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_FILTER)))
        {
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_VALUE)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->finfo[j].fname,(char *)value);
                    xmlFree(value);
                }

                //filter view enable
                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_VIEWS)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->finfo[j].views,(char *)value);
                    xmlFree(value);
                }

                //filter content
                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->finfo[j].content,(char *)value);
                    xmlFree(value);
                }

                //filter infos
                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_INFOS)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->finfo[j].infos,(char *)value);
                    xmlFree(value);
                }

                testnode = testnode->next;
            }

            j++;
        }

        ///////////////////////////////////////////////////////////////////////////////

        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NODE_LOG)))
        {
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                //log keyz
                if ((!xmlStrcmp(testnode->name, BAD_CAST CH_KEYZ)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->loginfo[m].keyz,(char *)value);
                    xmlFree(value);
                }

                //log source
                if ((!xmlStrcmp(testnode->name, BAD_CAST CH_SOURCE)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->loginfo[m].source,(char *)value);
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST CH_FILTER)))  //node name
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->loginfo[m].filter,(char *)value);
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST CH_DEST)))    //node name
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->loginfo[m].dest,(char *)value);
                    xmlFree(value);
                }

                testnode = testnode->next;
            }

            m++;
        }

        ///////////////////////////////////////////////////////////////////////////////
        pcurnode = pcurnode->next;
    }

    sysall->des_num=i;
    sysall->f_num=j;
    sysall->log_num=m;
    sysall->opt_num=n;
    sysall->su_num=p;
    xmlFreeDoc(pdoc);
    xmlCleanupParser();
    return 0;
}

//free link list
void Free_read_syslogall_st(SYSLOGALL_ST *sysall)
{
    if (sysall->optnum>0)
        Free_read_opt_xml(&(sysall->optst));

    if (sysall->sournum>0)
        Free_read_source_xml(&(sysall->sourst));

    if (sysall->filtnum>0)
        Free_read_filter_xml(&(sysall->filtst));

    if (sysall->destnum>0)
        Free_read_dest_xml(&(sysall->destst));

    if (sysall->lognum>0)
        Free_read_log_xml(&(sysall->logst));
}

int read_syslogall_st(char * xmlpath,SYSLOGALL_ST *sysall)
{
    int i,j,m,n,k;
    read_opt_xml(&(sysall->optst), &i);
    sysall->optnum=i;
    read_source_xml(&(sysall->sourst), &j);
    sysall->sournum=j;
    read_filter_xml(&(sysall->filtst), &m);
    sysall->filtnum=m;
    read_dest_xml(&(sysall->destst), &n);
    sysall->destnum=n;
    read_log_xml(&(sysall->logst), &k);
    sysall->lognum=k;
    return 0;
}

int write_config_syslogallst(SYSLOGALL_ST *sysall, char *file_path)
{
    FILE * fp;
    char *File_content=(char *)malloc(50000);
    char *content=(char *)malloc(1024);
    char *des_name=(char *)malloc(50);
    memset(File_content,0,50000);
    memset(des_name,0,50);
    ST_LOG_KEY keys;
    memset(&keys,0,sizeof(keys));
    struct opt_st    *oq;
    struct source_st *sq;
    struct filter_st *fq;
    struct dest_st   *dq;
    struct log_st    *lq;

    if (NULL == file_path)
    {
        return -1 ;
    }

    if ((fp = fopen(file_path,"w+"))==NULL)
        return -2;

    //option
    oq=sysall->optst.next;

    while (oq != NULL)
    {
        memset(content,0,1024);
        sprintf(content , " options { \n %s };\n\n",oq->contentz);
        strcat(File_content,content);
        oq = oq->next;
    }

    ///////////////////////////////////////////////////////////
    //source
    sq = sysall->sourst.next;

    while (sq!=NULL)
    {
        memset(des_name,0,50);
        sprintf(des_name , "source  %s { \n",sq->valuez);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " %s \n };\n\n",sq->contentz);
        strcat(File_content,content);
        sq = sq->next;
    }

    ///////////////////////////////////////////////////////////
    //des
    dq=sysall->destst.next;

    while (dq!=NULL)
    {
        memset(des_name,0,50);
        sprintf(des_name , "destination %s",dq->valuez);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " {%s};\n\n",dq->contentz);
        strcat(File_content,content);
        dq=dq->next;
    }

    ///////////////////////////////////////////////////////////
    //filter
    fq=sysall->filtst.next;

    while (fq!=NULL)
    {
        memset(des_name,0,50);
        sprintf(des_name , "filter %s",fq->valuez);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " {%s};\n\n",fq->contentz);
        strcat(File_content,content);
        fq=fq->next;
    }

    //////////////////////////////////////////////////////////////////
    //log
    lq=sysall->logst.next;

    while (lq!=NULL)
    {
        if (strcmp(lq->enablez,"0")!=0)
        {
            memset(des_name,0,50);
            sprintf(des_name , "log { \n source(%s); \n",lq->sourcez);
            strcat(File_content,des_name);
            memset(content,0,1024);
            memset(des_name,0,50);
            cut_up(lq->des, &keys,S_DEST);
            sprintf(content , " %s(%s);\n %s\n };\n\n",S_FILT,lq->filterz, keys.key);
            strcat(File_content,content);
        }

        lq=lq->next;
    }

    ////////////////////////////////////////////////////////
    fwrite(File_content,strlen(File_content),1,fp);
    fclose(fp);
    free(File_content);
    free(des_name);
    free(content);
    return 0;
}


int mod_log_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;
    xmlNodePtr propNodePtr = pcurnode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            if (xmlHasProp(pcurnode,BAD_CAST attribute))
            {
                propNodePtr = pcurnode;
                xmlAttrPtr attrPtr = propNodePtr->properties;

                while (attrPtr != NULL)
                {
                    if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
                    {
                        xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);

                        if (!xmlStrcmp(szAttr,BAD_CAST ruler))
                        {
                            xmlNodePtr childPtr = propNodePtr;
                            childPtr=childPtr->children;
                            xmlChar *value;

                            while (childPtr !=NULL)
                            {
                                if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
                                {
                                    value = xmlNodeGetContent(childPtr);
                                    xmlNodeSetContent(childPtr, BAD_CAST  newc);
                                    xmlFree(value);
                                }

                                childPtr = childPtr->next;
                            }
                        }

                        xmlFree(szAttr);
                    }

                    attrPtr = attrPtr->next;
                }
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}

//free link list
void Free_read_dest_xml(struct dest_st *head)
{
    struct dest_st *f1,*f2;
    f1=head->next;
    f2=f1->next;

    while (f2!=NULL)
    {
        free(f1->valuez);
        free(f1->contentz);
        free(f1->sysipz);
        free(f1->sysport);
        free(f1->timeflag);
        free(f1->indexz);
        free(f1);
        f1=f2;
        f2=f2->next;
    }

    free(f1);
}

int read_dest_xml(struct dest_st *chead,int *confnum)
{
    xmlDocPtr doc;
    xmlNodePtr cur,tmp;
    int conflag=0;
    doc = xmlReadFile(XML_FPATH, "utf-8", 256);

    if (doc == NULL)
    {
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "root"))
    {
        xmlFreeDoc(doc);
        return -1;
    }

    struct dest_st *ctail=NULL;

    chead->next=NULL;

    ctail=chead;

    cur = cur->xmlChildrenNode;

    while (cur !=NULL)
    {
        tmp = cur->xmlChildrenNode;

        if (!xmlStrcmp(cur->name, BAD_CAST NODE_DES))
        {
            /////////////conf informations
            struct dest_st  *cq=NULL;
            cq=(struct dest_st *)malloc(sizeof(struct dest_st)+1);
            memset(cq,0,sizeof(struct dest_st)+1);

            if (NULL == cq)
            {
                return  -1;
            }

            cq->valuez=(char *)malloc(50);
            memset(cq->valuez,0,50);
            cq->contentz=(char *)malloc(256);
            memset(cq->contentz,0,256);
            cq->timeflag=(char *)malloc(20);
            memset(cq->timeflag,0,20);
            cq->sysipz=(char *)malloc(50);
            memset(cq->sysipz,0,50);
            cq->sysport=(char *)malloc(10);
            memset(cq->sysport,0,10);
            cq->indexz=(char *)malloc(20);
            memset(cq->indexz,0,20);
            conflag++;
            xmlChar *value=NULL;

            while (tmp !=NULL)
            {
                //dest value
                if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VALUE)))
                {
                    memset(cq->valuez,0,50);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->valuez,(char *)value);
                    xmlFree(value);
                }
                //dest content
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONTENT)))
                {
                    memset(cq->contentz,0,256);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->contentz,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_MARKZ)))
                {
                    memset(cq->timeflag,0,20);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->timeflag,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSIP)))
                {
                    memset(cq->sysipz,0,50);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->sysipz,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSPORT)))
                {
                    memset(cq->sysport,0,10);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->sysport,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_INDEXZ)))
                {
                    memset(cq->indexz,0,20);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->indexz,(char *)value);
                    xmlFree(value);
                }

                tmp = tmp->next;
            }

            cq->next = NULL;
            ctail->next = cq;
            ctail = cq;
        }

        cur = cur->next;
    }

    *confnum=conflag;
    xmlFreeDoc(doc);
    return 0;
}

//free link list
void Free_read_log_xml(struct log_st *head)
{
    struct log_st *f1,*f2;
    f1=head->next;
    f2=f1->next;

    while (f2!=NULL)
    {
        free(f1->keyz);
        free(f1->sourcez);
        free(f1->filterz);
        free(f1->des);
        free(f1->enablez);
        free(f1->timeflag);
        free(f1->sysipz);
        free(f1->sysport);
        free(f1->flag);
        free(f1->indexz);
        free(f1);
        f1=f2;
        f2=f2->next;
    }

    free(f1);
}
int read_log_xml(struct log_st *chead,int *confnum)
{
    xmlDocPtr doc;
    xmlNodePtr cur,tmp;
    int conflag=0;
    doc = xmlReadFile(XML_FPATH, "utf-8", 256);

    if (doc == NULL)
    {
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "root"))
    {
        xmlFreeDoc(doc);
        return -1;
    }

    struct log_st *ctail=NULL;

    chead->next=NULL;

    ctail=chead;

    cur = cur->xmlChildrenNode;

    while (cur !=NULL)
    {
        tmp = cur->xmlChildrenNode;

        if (!xmlStrcmp(cur->name, BAD_CAST NODE_LOG))
        {
            /////////////conf informations
            struct log_st  *cq=NULL;
            cq=(struct log_st *)malloc(sizeof(struct log_st)+1);
            memset(cq,0,sizeof(struct log_st)+1);

            if (NULL == cq)
            {
                return  -1;
            }

            cq->keyz=(char *)malloc(50);
            memset(cq->keyz,0,50);
            cq->sourcez=(char *)malloc(50);
            memset(cq->sourcez,0,50);
            cq->filterz=(char *)malloc(128);
            memset(cq->filterz,0,128);
            cq->des=(char *)malloc(128);
            memset(cq->des,0,128);
            cq->enablez=(char *)malloc(10);
            memset(cq->enablez,0,10);
            cq->timeflag=(char *)malloc(20);
            memset(cq->timeflag,0,20);
            cq->sysipz=(char *)malloc(50);
            memset(cq->sysipz,0,50);
            cq->sysport=(char *)malloc(10);
            memset(cq->sysport,0,10);
            cq->indexz=(char *)malloc(20);
            memset(cq->indexz,0,20);
            conflag++;
            xmlChar *value=NULL;

            while (tmp !=NULL)
            {
                //log keyz
                if ((!xmlStrcmp(tmp->name, BAD_CAST CH_KEYZ)))
                {
                    memset(cq->keyz,0,50);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->keyz,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_ENABLES)))
                {
                    memset(cq->enablez,0,10);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->enablez,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST CH_SOURCE)))
                {
                    memset(cq->sourcez,0,50);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->sourcez,(char *)value);
                    xmlFree(value);
                }
                //log filter
                else if ((!xmlStrcmp(tmp->name, BAD_CAST CH_FILTER)))
                {
                    memset(cq->filterz,0,128);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->filterz,(char *)value);
                    xmlFree(value);
                }
                //log des
                else if ((!xmlStrcmp(tmp->name, BAD_CAST CH_DEST)))
                {
                    memset(cq->des,0,128);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->des,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_MARKZ)))
                {
                    memset(cq->timeflag,0,20);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->timeflag,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSIP)))
                {
                    memset(cq->sysipz,0,50);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->sysipz,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_SYSPORT)))
                {
                    memset(cq->sysport,0,10);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->sysport,(char *)value);
                    xmlFree(value);
                }
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_INDEXZ)))
                {
                    memset(cq->indexz,0,20);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->indexz,(char *)value);
                    xmlFree(value);
                }

                tmp = tmp->next;
            }

            cq->next = NULL;
            ctail->next = cq;
            ctail = cq;
        }

        cur = cur->next;
    }

    *confnum=conflag;
    xmlFreeDoc(doc);
    return 0;
}

//free link list
void Free_read_filter_xml(struct filter_st *head)
{
    struct filter_st *f1,*f2;
    f1=head->next;
    f2=f1->next;

    while (f2!=NULL)
    {
        free(f1->valuez);
        free(f1->viewz);
        free(f1->contentz);
        free(f1->infos);
        free(f1);
        f1=f2;
        f2=f2->next;
    }

    free(f1);
}

int read_filter_xml(struct filter_st *chead,int *confnum)
{
    xmlDocPtr doc;
    xmlNodePtr cur,tmp;
    int conflag=0;
    doc = xmlReadFile(XML_FPATH, "utf-8", 256);

    if (doc == NULL)
    {
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "root"))
    {
        xmlFreeDoc(doc);
        return -1;
    }

    struct filter_st *ctail=NULL;

    chead->next=NULL;

    ctail=chead;

    cur = cur->xmlChildrenNode;

    while (cur !=NULL)
    {
        tmp = cur->xmlChildrenNode;

        if (!xmlStrcmp(cur->name, BAD_CAST NODE_FILTER))
        {
            /////////////conf informations
            struct filter_st  *cq=NULL;
            cq=(struct filter_st *)malloc(sizeof(struct filter_st)+1);
            memset(cq,0,sizeof(struct filter_st)+1);

            if (NULL == cq)
            {
                return  -1;
            }

            cq->valuez=(char *)malloc(50);
            memset(cq->valuez,0,50);
            cq->viewz=(char *)malloc(10);
            memset(cq->viewz,0,10);
            cq->contentz=(char *)malloc(128);
            memset(cq->contentz,0,128);
            cq->infos=(char *)malloc(128);
            memset(cq->infos,0,128);
            /////////////
            conflag++;
            xmlChar *value=NULL;

            while (tmp !=NULL)
            {
                //filter value key
                if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VALUE)))
                {
                    memset(cq->valuez,0,50);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->valuez,(char *)value);
                    xmlFree(value);
                }
                //filter views en or dis
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VIEWS)))
                {
                    memset(cq->viewz,0,10);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->viewz,(char *)value);
                    xmlFree(value);
                }
                //filter content
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONTENT)))
                {
                    memset(cq->contentz,0,128);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->contentz,(char *)value);
                    xmlFree(value);
                }
                //filter views
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_INFOS)))
                {
                    memset(cq->infos,0,128);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->infos,(char *)value);
                    xmlFree(value);
                }

                tmp = tmp->next;
            }

            cq->next = NULL;
            ctail->next = cq;
            ctail = cq;
        }

        cur = cur->next;
    }

    *confnum=conflag;
    xmlFreeDoc(doc);
    return 0;
}

//free link list
void Free_read_opt_xml(struct opt_st *head)
{
    struct opt_st *f1,*f2;
    f1=head->next;
    f2=f1->next;

    while (f2!=NULL)
    {
        free(f1->contentz);
        free(f1);
        f1=f2;
        f2=f2->next;
    }

    free(f1);
}

int read_opt_xml(struct opt_st *chead,int *confnum)
{
    xmlDocPtr doc;
    xmlNodePtr cur,tmp;
    int conflag=0;
    doc = xmlReadFile(XML_FPATH, "utf-8", 256);

    if (doc == NULL)
    {
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "root"))
    {
        xmlFreeDoc(doc);
        return -1;
    }

    struct opt_st *ctail=NULL;

    chead->next=NULL;

    ctail=chead;

    cur = cur->xmlChildrenNode;

    while (cur !=NULL)
    {
        tmp = cur->xmlChildrenNode;

        if (!xmlStrcmp(cur->name, BAD_CAST NODE_OPT))
        {
            /////////////conf informations
            struct opt_st  *cq=NULL;
            cq=(struct opt_st *)malloc(sizeof(struct opt_st)+1);
            memset(cq,0,sizeof(struct opt_st)+1);

            if (NULL == cq)
            {
                return  -1;
            }

            cq->contentz=(char *)malloc(256);
            memset(cq->contentz,0,256);
            /////////////
            conflag++;
            xmlChar *value=NULL;
            value=xmlNodeGetContent(cur);
            strcpy(cq->contentz,(char *)value);
            xmlFree(value);
            cq->next = NULL;
            ctail->next = cq;
            ctail = cq;
        }

        cur = cur->next;
    }

    *confnum=conflag;
    xmlFreeDoc(doc);
    return 0;
}

//free link list
void Free_read_source_xml(struct source_st *head)
{
    struct source_st *f1,*f2;
    f1=head->next;
    f2=f1->next;

    while (f2!=NULL)
    {
        free(f1->contentz);
        free(f1->valuez);
        free(f1);
        f1=f2;
        f2=f2->next;
    }

    free(f1);
}

int read_source_xml(struct source_st *chead,int *confnum)
{
    xmlDocPtr doc;
    xmlNodePtr cur,tmp;
    int conflag=0;
    doc = xmlReadFile(XML_FPATH, "utf-8", 256);

    if (doc == NULL)
    {
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }

    if (xmlStrcmp(cur->name, (const xmlChar *) "root"))
    {
        xmlFreeDoc(doc);
        return -1;
    }

    struct source_st *ctail=NULL;

    chead->next=NULL;

    ctail=chead;

    cur = cur->xmlChildrenNode;

    while (cur !=NULL)
    {
        tmp = cur->xmlChildrenNode;

        if (!xmlStrcmp(cur->name, BAD_CAST NODE_SOURCE))
        {
            /////////////conf informations
            struct source_st  *cq=NULL;
            cq=(struct source_st *)malloc(sizeof(struct source_st)+1);
            memset(cq,0,sizeof(struct source_st)+1);

            if (NULL == cq)
            {
                return  -1;
            }

            cq->valuez=(char *)malloc(50);
            memset(cq->valuez,0,50);
            cq->contentz=(char *)malloc(256);
            memset(cq->contentz,0,256);
            /////////////
            conflag++;
            xmlChar *value=NULL;

            while (tmp !=NULL)
            {
                //source value
                if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_VALUE)))
                {
                    memset(cq->valuez,0,50);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->valuez,(char *)value);
                    xmlFree(value);
                }
                //source content
                else if ((!xmlStrcmp(tmp->name, BAD_CAST NODE_CONTENT)))
                {
                    memset(cq->contentz,0,256);
                    value=xmlNodeGetContent(tmp);
                    strcpy(cq->contentz,(char *)value);
                    xmlFree(value);
                }

                tmp = tmp->next;
            }

            cq->next = NULL;
            ctail->next = cq;
            ctail = cq;
        }

        cur = cur->next;
    }

    *confnum=conflag;
    xmlFreeDoc(doc);
    return 0;
}

int get_second_xmlnode(char * fpath,char * node_name,char * content,char *gets,int flagz)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);
    int tagz = 0;

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            tagz ++ ;
            xmlNodePtr childPtr = pcurnode;
            childPtr=childPtr->children;
            xmlChar *value;

            while (childPtr !=NULL)
            {
                if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
                {
                    value = xmlNodeGetContent(childPtr);

                    if (tagz == flagz)
                    {
                        strcpy(gets,(char *)value);
                    }

                    xmlFree(value);
                }

                childPtr = childPtr->next;
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}


int mod_second_xmlnode(char * fpath,char * node_name,char * content,char *newc,int flagz)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);
    int tagz = 0;

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            tagz ++ ;
            xmlNodePtr childPtr = pcurnode;
            childPtr=childPtr->children;
            xmlChar *value;

            while (childPtr !=NULL)
            {
                if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
                {
                    value = xmlNodeGetContent(childPtr);

                    if (tagz == flagz)
                    {
                        xmlNodeSetContent(childPtr, BAD_CAST  newc);
                    }

                    xmlFree(value);
                }

                childPtr = childPtr->next;
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}


int find_second_xmlnode(char * fpath,char * node_name,char * content,char *keyz,int *flagz)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    *flagz = 0;
    int tagz = 0,freetag = -1;
    pdoc = xmlReadFile(psfilename,"utf-8",256);

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;  //root---->des

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))       //root--->des
        {
            tagz ++;
            xmlNodePtr childPtr = pcurnode;
            childPtr=childPtr->children;
            xmlChar *value;

            while (childPtr !=NULL)
            {
                if (!xmlStrcmp(childPtr->name, BAD_CAST content))
                {
                    value = xmlNodeGetContent(childPtr);

                    if (strcmp(keyz,(char *)value)==0)
                    {
                        freetag = 0;
                        break;
                    }

                    xmlFree(value);
                }

                childPtr = childPtr->next;
            }

            if (freetag==0)
            {
                xmlFree(value);
            }
        }

        if (freetag==0)
            break;

        pcurnode = pcurnode->next;
    }

    if (freetag==0)//find
        *flagz = tagz;
    else
        *flagz = 0;

    xmlSaveFile(fpath,pdoc);
    return 0;
}

int add_second_xmlnode(char * fpath,char * node_name,ST_SYS_ALL *sysall)
{
    int numz=0,i=0;
    numz=sizeof(sysall->suinfo)/sizeof(sysall->suinfo[0]);
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",256);

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);
    xmlAddChild(pcurnode,node);

    for (i=0; i<numz; i++)
    {
        if (strcmp(sysall->suinfo[i].suname,"")!=0)
        {
            xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST sysall->suinfo[i].suname);
            xmlNodePtr content1 = xmlNewText(BAD_CAST sysall->suinfo[i].content);
            xmlAddChild(node,node1);
            xmlAddChild(node1,content1);
        }
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}


int del_second_xmlnode(char * fpath,char * node_name,int flagz)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);
    int tagz = 0;

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            tagz ++;

            if (tagz == flagz)
            {
                xmlNodePtr tempNode;
                tempNode = pcurnode->next;
                xmlUnlinkNode(pcurnode);
                xmlFreeNode(pcurnode);
                pcurnode= tempNode;
                continue;
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}



int find_log_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,ST_LOG_KEY *logkey)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;
    xmlNodePtr propNodePtr = pcurnode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            if (xmlHasProp(pcurnode,BAD_CAST attribute))
            {
                propNodePtr = pcurnode;
                xmlAttrPtr attrPtr = propNodePtr->properties;

                while (attrPtr != NULL)
                {
                    if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
                    {
                        xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);

                        if (!xmlStrcmp(szAttr,BAD_CAST ruler))
                        {
                            xmlNodePtr childPtr = propNodePtr;
                            childPtr=childPtr->children;
                            xmlChar *value;

                            while (childPtr !=NULL)
                            {
                                if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
                                {
                                    value = xmlNodeGetContent(childPtr);
                                    strcpy(logkey->key,(char *)value);
                                    xmlFree(value);
                                }

                                childPtr = childPtr->next;
                            }
                        }

                        xmlFree(szAttr);
                    }

                    attrPtr = attrPtr->next;
                }
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}


int  cut_up(char *dstring,ST_LOG_KEY *keys,char *ruler)
{
    char input[256];
    memset(input,0,256);
    strcpy(input,dstring);
    char *p;
    char des[128];
    char output[512];
    memset(output,0,512);
    p=strtok(input,";");

    while (p != NULL)
    {
        memset(des,0,128);
        sprintf(des,"%s (%s);",ruler,p);
        strcat(output,des);
        p = strtok(NULL, ";");
    }

    strcpy(keys->key,output);
    return 0;
}

char *first_ip(char *dstring)
{
    char input[256];
    memset(input,0,256);
    strcpy(input,dstring);
    char *p;
    p=strtok(input,";");

    if (p)
        return p;
}



char *first_port(char *dstring)
{
    char input[256];
    memset(input,0,256);
    char *output;
    strcpy(input,dstring);
    output=first_ip(dstring);
    char *p;
    p=strtok(output,"(");

    if (p)
        p=strtok(NULL,";");

    return p;
}

int  cut_up_ip(char *dstring,char *ip)
{
    char *input;
    char *p;
    char temp[60];
    memset(temp,0,60);
    input=first_ip(dstring);
    p=strtok(input,"\"");

    if (p)
        p=strtok(NULL,"\"");

    strcpy(ip,p);
    return 0;
}

int  cut_up_port(char *dstring,char *port)
{
    char *input;
    char *p;
    //char q;
    input=first_port(dstring);
    p=strtok(input,"(");

    if (p)
        p=strtok(NULL,")");

    strcpy(port,p);
    return 0;
}

char *replace_ip(char *strbuf, char *sstr, char *dstr)
{
    char *p,*p1;
    int len;

    if ((strbuf == NULL)||(sstr == NULL)||(dstr == NULL))
        return NULL;

    p = strstr(strbuf, sstr);

    if (p == NULL)  /*not found*/
        return NULL;

    len = strlen(strbuf) + strlen(dstr) - strlen(sstr);
    p1 = malloc(len);
    bzero(p1, len);
    strncpy(p1, strbuf, p-strbuf);
    strcat(p1, dstr);
    p += strlen(sstr);
    strcat(p1, p);
    return p1;
}


int ip_save(char *fpath,char *value)
{
    FILE *fp;
    char buff[128];
    memset(buff,0,128);
    strcpy(buff,value);
    int ret,op_ret;
    ret=is_file_exist(fpath);

    if (ret==0)
    {
        op_ret=line_num(fpath,"wc -l");

        if (op_ret>4)
            return 2;
        else
        {
            fp=fopen(fpath,"at");

            if (fp==NULL)
                return 1;
            else
            {
                fwrite(buff,strlen(buff),1,fp);
                fclose(fp);
            }
        }
    }
    else
    {
        fp=fopen(fpath,"wt");

        if (fp==NULL)
            return 1;
        else
        {
            fwrite(buff,strlen(buff),1,fp);
            fclose(fp);
        }
    }

    return 0;
}


int  line_num(char *fpath,char *cmd)
{
    FILE *fp;
    char buff[128];
    char temp[30];
    memset(temp,0,30);
    sprintf(temp,"%s %s",cmd,fpath);
    fp=popen(temp,"r");
    fgets(buff, sizeof(buff), fp);
    char input[128];
    memset(input,0,128);
    strcpy(input,buff);
    char *p;
    int i;
    p=strtok(input," ");

    if (p)
        i=atoi(p);

    pclose(fp);
    return i;
}


int get_ip(char *fpath,ST_IP_CONTENT * ip)
{
    FILE *fp;
    char buff[128];
    fp=fopen(fpath,"r");

    if (fp==NULL)
        return 1;
    else
    {
        fgets(buff, sizeof(buff), fp);

        do
        {
            strcat(ip->content,buff);
            fgets(buff, sizeof(buff), fp);
        }
        while (!feof(fp));

        fclose(fp);
    }

    return 0;
}



int check_abc_d(char *ff)
{
    int pf,i,j;
    pf = strtoul(ff,0,10);
    char *df;
    df=(char *)malloc(sizeof(ff));
    sprintf(df,"%d",pf);
    i=strlen(ff);
    j=strlen(df);
    free(df);

    if (i!=j)
        return -1;
    else
        return 0;
}


int if_subs(char *zstring,char *subs)
{
    char input[256];
    memset(input,0,256);
    strcpy(input,zstring);
    char *p;
    p=strtok(input,";");

    while (p != NULL)
    {
        if (strcmp(subs,p)==0)
        {
            return 2;
            break;
        }

        p = strtok(NULL, ";");
    }

    return 0;
}

int read_ntp(char * name, ST_SYS_ALL *sysall)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = name;
    //sysall = (ST_SYS_ALL *)malloc(sizeof(ST_SYS_ALL));
    memset(sysall, 0, sizeof(ST_SYS_ALL));
    pdoc = xmlReadFile(psfilename,"utf-8",256);
    //pdoc = xmlReadFile(psfilename, NULL, 0);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    //xmlChar *key;
    xmlChar *value;
    xmlChar *content;
    int i=0,j=0,n=0,p=0;
    pcurnode=pcurnode->xmlChildrenNode;

    while (pcurnode != NULL)
    {
        //drift
        if ((!xmlStrcmp(pcurnode->name, BAD_CAST  NTP_DRIFT)))
        {
            value = xmlNodeGetContent(pcurnode);
            sysall->optinfo[n].content=(char *)malloc(xmlStrlen(value));
            strcpy(sysall->optinfo[n].content,(char *)value);
            xmlFree(value);
            n++;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NTP_SERV)))
        {
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->suinfo[p].suname,(char *)value);
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->suinfo[p].content,(char *)value);
                    xmlFree(value);
                }

                testnode = testnode->next;
            }

            p++;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NTP_REST)))
        {
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->desinfo[i].rname,(char *)value);
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_CONTENT)))
                {
                    content = xmlNodeGetContent(testnode);
                    strcpy(sysall->desinfo[i].content,(char *)value);
                    xmlFree(content);
                }

                testnode = testnode->next;
            }

            i++;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NTP_BROD)))
        {
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_VALUE)))
                {
                    value = xmlNodeGetContent(testnode);
                    strcpy(sysall->finfo[j].fname,(char *)value);
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))
                {
                    content = xmlNodeGetContent(testnode);
                    strcpy(sysall->finfo[j].content,(char *)value);
                    xmlFree(content);
                }

                testnode = testnode->next;
            }

            j++;
        }

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        pcurnode = pcurnode->next;
    }

    sysall->des_num=i;
    sysall->f_num=j;
    sysall->su_num=p;
    sysall->opt_num=n;
    xmlFreeDoc(pdoc);
    xmlCleanupParser();
    return 0;
}
void Free_read_ntp_server(struct ntp_server_st *head)
{
    struct ntp_server_st *f1,*f2;
    f1=head->next;

    if (NULL == f1)
    {
        return ;
    }

    f2=f1->next;

    while (f2!=NULL)
    {
        free(f1);
        f1=f2;
        f2=f2->next;
    }

    free(f1);
}


int read_ntp_server(char * name, struct ntp_server_st *head)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    struct ntp_server_st *tail = NULL;
    head->next = NULL;
    tail = head;
    pdoc = xmlReadFile(name,"utf-8",256);

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    xmlChar *value;
    xmlChar *content;
    pcurnode=pcurnode->xmlChildrenNode;

    while (pcurnode != NULL)
    {
        //server
        if ((!xmlStrcmp(pcurnode->name, BAD_CAST NTP_SERV)))
        {
            struct ntp_server_st *cq=NULL;
            cq=(struct ntp_server_st *)malloc(sizeof(struct ntp_server_st)+1);

            if (NULL == cq)
            {
                return  -1;
            }

            memset(cq,0,sizeof(struct ntp_server_st)+1);
            xmlChar* szAttr = xmlGetProp(pcurnode,BAD_CAST NODE_ATT);
            memset(cq->stime,0,sizeof(cq->stime));
            strncpy(cq->stime,szAttr,sizeof(cq->stime));
            xmlFree(szAttr);
            xmlNodePtr testnode;
            testnode=pcurnode;
            testnode=testnode->children;

            while (testnode !=NULL)
            {
                //server
                if ((!xmlStrcmp(testnode->name, BAD_CAST  NODE_VALUE)))
                {
                    memset(cq->sip,0,sizeof(cq->sip));
                    value = xmlNodeGetContent(testnode);
                    strncpy(cq->sip,(char *)value,sizeof(cq->sip));
                    xmlFree(value);
                }

                if ((!xmlStrcmp(testnode->name, BAD_CAST NODE_CONTENT)))
                {
                    memset(cq->sper,0,sizeof(cq->sper));
                    value = xmlNodeGetContent(testnode);
                    strncpy(cq->sper,(char *)value,sizeof(cq->sper));
                    xmlFree(value);
                }

                testnode = testnode->next;
            }

            cq->next = NULL;
            tail->next = cq;
            tail = cq;
        }

        pcurnode = pcurnode->next;
    }

    xmlFreeDoc(pdoc);
    xmlCleanupParser();
    return 0;
}

int write_ntp(ST_SYS_ALL *sysall, char *file_path)
{
    FILE * fp;
    char File_content[10000],des_name[50],content[1024];
    memset(File_content,0,10000);
    memset(des_name,0,50);
    ST_LOG_KEY keys;
    memset(&keys,0,sizeof(keys));
    int i;

    if (NULL == file_path)
    {
        return -1 ;
    }

    if ((fp = fopen(file_path,"w+"))==NULL)
        return -2;

    ///////////////////////////////////////////////////////////
    //restrict 0.0.0.0 mask  0.0.0.0  parameter

    for (i=0; i<sysall->des_num; i++)
    {
        memset(des_name,0,50);
        sprintf(des_name , "restrict %s  ",sysall->desinfo[i].rname);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " %s \n",sysall->desinfo[i].content);
        strcat(File_content,content);
    }

    ///////////////////////////////////////////////////////////
    //server 0.0.0.0 perferd

    for (i=0; i<sysall->su_num; i++)
    {
        memset(des_name,0,50);
        sprintf(des_name , "server  %s ",sysall->suinfo[i].suname);
        strcat(File_content,des_name);
        memset(content,0,1024);
        sprintf(content , " %s \n",sysall->suinfo[i].content);
        strcat(File_content,content);
    }

    ///////////////////////////////////////////////////////////
    //restrict 0.0.0.0

    for (i=0; i<sysall->f_num; i++)
    {
        memset(des_name,0,50);
        sprintf(des_name , "fudge %s stratum 0\n",sysall->finfo[i].fname);
        strcat(File_content,des_name);
        memset(des_name,0,50);
        sprintf(des_name , "restrict %s\n",sysall->finfo[i].fname);
        strcat(File_content,des_name);
    }

#if 0

    //////////////////////////////////////////////////////////////////
    for (i=0; i<sysall->opt_num; i++)
    {
        memset(content,0,1024);
        sprintf(content , "driftfile %s \n",((sysall->optinfo[i]).content));
        strcat(File_content,content);
    }

    for (i=0; i<(sysall->opt_num); i++)
    {
        free((sysall->optinfo[i]).content);
    }

#endif
    fwrite(File_content,strlen(File_content),1,fp);
    fclose(fp);
    return 0;
}


int add_ntp_node(char *fpath,char * node_name,char * value,char *content)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",256);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);
    xmlAddChild(pcurnode,node);
    xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST NODE_VALUE);
    xmlNodePtr content1 = xmlNewText(BAD_CAST value);
    xmlNodePtr node2 = xmlNewNode(NULL,BAD_CAST NODE_CONTENT);
    xmlNodePtr content2 = xmlNewText(BAD_CAST  content);
    xmlAddChild(node,node1);
    xmlAddChild(node,node2);
    xmlAddChild(node1,content1);
    xmlAddChild(node2,content2);
    xmlSaveFile(fpath,pdoc);
    return 0;
}



int add_ntp_node_new(char *fpath,char * node_name,char * value,char *content,char *attribute)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",256);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);
    xmlAddChild(pcurnode,node);
    xmlNodePtr node1 = xmlNewNode(NULL,BAD_CAST NODE_VALUE);
    xmlNodePtr content1 = xmlNewText(BAD_CAST value);
    xmlNodePtr node2 = xmlNewNode(NULL,BAD_CAST NODE_CONTENT);
    xmlNodePtr content2 = xmlNewText(BAD_CAST  content);
    xmlAddChild(node,node1);
    xmlAddChild(node,node2);
    xmlAddChild(node1,content1);
    xmlAddChild(node2,content2);
    xmlSetProp(node,NODE_ATT, attribute);
    xmlSaveFile(fpath,pdoc);
    return 0;
}


int ntp_del(char *fpath,char *node_name,char *attribute,char *key)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",256);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            if (xmlHasProp(pcurnode,BAD_CAST attribute))
            {
                if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
                {
                    xmlNodePtr tempNode;
                    tempNode = pcurnode->next;
                    xmlUnlinkNode(pcurnode);
                    xmlFreeNode(pcurnode);
                    pcurnode= tempNode;
                    continue;
                }
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}

int find_ntp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *logkey)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    int flag=-1;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",256);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;
    xmlNodePtr propNodePtr = pcurnode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            if (xmlHasProp(pcurnode,BAD_CAST attribute))
            {
                propNodePtr = pcurnode;
                xmlAttrPtr attrPtr = propNodePtr->properties;

                while (attrPtr != NULL)
                {
                    if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
                    {
                        xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);

                        if (!xmlStrcmp(szAttr,BAD_CAST ruler))
                        {
                            xmlNodePtr childPtr = propNodePtr;
                            childPtr=childPtr->children;
                            xmlChar *value;

                            while (childPtr !=NULL)
                            {
                                if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
                                {
                                    value = xmlNodeGetContent(childPtr);
                                    strcpy(logkey,(char *)value);
                                    flag=0;
                                    xmlFree(value);
                                }

                                childPtr = childPtr->next;
                            }
                        }

                        xmlFree(szAttr);
                    }

                    attrPtr = attrPtr->next;
                }
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    //return 0;
    return flag;
}



int del_ntp_node(char * fpath,char * node_name,char *attribute,char *ruler)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    pdoc = xmlReadFile(fpath,"utf-8",XML_PARSE_RECOVER);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;
    xmlNodePtr propNodePtr = pcurnode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            xmlAttrPtr attrPtr = xmlGetProp(pcurnode,BAD_CAST NODE_ATT);

            if (!xmlStrcmp(attrPtr,BAD_CAST ruler))
            {
                xmlNodePtr tempNode;
                tempNode = pcurnode->next;
                xmlUnlinkNode(pcurnode);
                xmlFreeNode(pcurnode);
                pcurnode= tempNode;
                continue;
            }

            xmlFree(attrPtr);
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    xmlFreeDoc(pdoc);
    return 0;
}


int mod_ntp_node(char * fpath,char * node_name,char *attribute,char *ruler,char * content,char *newc)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    char *psfilename;
    psfilename = fpath;
    pdoc = xmlReadFile(psfilename,"utf-8",XML_PARSE_RECOVER);

    if (NULL == pdoc)
    {
        //fprintf(cgiOut,"error: open file %s" , psfilename);
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;
    xmlNodePtr propNodePtr = pcurnode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            if (xmlHasProp(pcurnode,BAD_CAST attribute))
            {
                propNodePtr = pcurnode;
                xmlAttrPtr attrPtr = propNodePtr->properties;

                while (attrPtr != NULL)
                {
                    if (!xmlStrcmp(attrPtr->name, BAD_CAST attribute))
                    {
                        xmlChar* szAttr = xmlGetProp(propNodePtr,BAD_CAST attribute);

                        if (!xmlStrcmp(szAttr,BAD_CAST ruler))
                        {
                            xmlNodePtr childPtr = propNodePtr;
                            childPtr=childPtr->children;
                            xmlChar *value;

                            while (childPtr !=NULL)
                            {
                                if ((!xmlStrcmp(childPtr->name, BAD_CAST content)))
                                {
                                    value = xmlNodeGetContent(childPtr);
                                    xmlNodeSetContent(childPtr, BAD_CAST  newc);
                                    xmlFree(value);
                                }

                                childPtr = childPtr->next;
                            }
                        }

                        xmlFree(szAttr);
                    }

                    attrPtr = attrPtr->next;
                }
            }
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(fpath,pdoc);
    return 0;
}

//get cli-log on|off

int get_cli_syslog_state()
{
    FILE *fp=NULL;
    char *ptr=NULL;
    int ret=0;
    fp = fopen(VTYSH_CLI_LOG_CONFIG_FILE,"r");

    if (!fp)
        return 0;

    ptr=malloc(8);
    memset(ptr,0,8);

    if (!ptr)
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

#if 0
int get_curr_syslog_num(char *xml_fpath)
{
    SYSLOGALL_ST sysall;
    struct log_st *head=NULL;
    int num=0;
    read_syslogall_st(xml_fpath, &sysall);

    for (head = &sysall.logst; head != NULL; head = head->next)
    {
        if (NULL != head.flag)
        {
            num++;
        }
    }

    return num;
}
#endif

int add_syslog_server(char *xml_fpath, int enable, char *ip, int port, int filter ,int indexz)
{
    ST_SYS_ALL sysall,logall;
    time_t now;
    unsigned int timez = (unsigned int)time(&now);
    memset(&sysall, 0, sizeof(ST_SYS_ALL));
    sprintf(sysall.suinfo[0].suname, "%s",NODE_ENABLES);
    sprintf(sysall.suinfo[0].content, "%d",enable);
    sprintf(sysall.suinfo[1].suname,  "%s",CH_SOURCE);
    sprintf(sysall.suinfo[1].content,  "%s","s_all");
    sprintf(sysall.suinfo[2].suname,  "%s",CH_FILTER);
    sprintf(sysall.suinfo[2].content,  "%s",filterlist[filter]);
    sprintf(sysall.suinfo[3].suname, "%s", CH_DEST);
    sprintf(sysall.suinfo[3].content, "df_server_%d", timez);
    sprintf(sysall.suinfo[4].suname,  "%s",NODE_MARKZ);
    sprintf(sysall.suinfo[4].content, "%d", timez);
    sprintf(sysall.suinfo[5].suname,  "%s",NODE_SYSIP);
    sprintf(sysall.suinfo[5].content, "%s", ip);
    sprintf(sysall.suinfo[6].suname,  "%s",NODE_SYSPORT);
    sprintf(sysall.suinfo[6].content, "%d", port);
    sprintf(sysall.suinfo[7].suname,  "%s",NODE_INDEXZ);
    sprintf(sysall.suinfo[7].content, "%d", indexz);
    add_second_xmlnode(xml_fpath, NODE_LOG, &sysall);
    memset(&logall, 0, sizeof(ST_SYS_ALL));
    sprintf(logall.suinfo[0].suname,  "%s",NODE_VALUE);
    sprintf(logall.suinfo[0].content, "df_server_%d", timez);
    sprintf(logall.suinfo[1].suname,  "%s",NODE_CONTENT);
    sprintf(logall.suinfo[1].content, "udp(\"%s\" port(%d))", ip, port);
    sprintf(logall.suinfo[2].suname,  "%s",NODE_MARKZ);
    sprintf(logall.suinfo[2].content, "%d", timez);
    sprintf(logall.suinfo[3].suname,  "%s",NODE_SYSIP);
    sprintf(logall.suinfo[3].content, "%s", ip);
    sprintf(logall.suinfo[4].suname,  "%s",NODE_SYSPORT);
    sprintf(logall.suinfo[4].content, "%d", port);
    sprintf(logall.suinfo[5].suname,  "%s",NODE_INDEXZ);
    sprintf(logall.suinfo[5].content, "%d", indexz);
    add_second_xmlnode(xml_fpath, NODE_DES, &logall);
    return 0;
}
int mod_syslog_server(char *xml_fpath, char *timeflag, int enable, char *ipaddr, int port, int filter)
{
    int flagz=0,idindex=0;
    char *tempz=(char *)malloc(128);
    char *strport=(char *)malloc(20);
    //log idindex
    find_second_xmlnode(xml_fpath, NODE_LOG, NODE_MARKZ, timeflag, &idindex);
    //des flagz
    find_second_xmlnode(xml_fpath, NODE_DES, NODE_MARKZ, timeflag,&flagz);

    if (flagz != 0)
    {
        memset(tempz,0,128);
        sprintf(tempz,"udp(\"%s\" port(%d))",ipaddr,port);
        mod_second_xmlnode(xml_fpath, NODE_DES, NODE_CONTENT, tempz, flagz);
        mod_second_xmlnode(xml_fpath, NODE_DES, NODE_SYSIP, ipaddr, flagz);
        memset(strport,0,20);
        sprintf(strport,"%d",port);
        mod_second_xmlnode(xml_fpath, NODE_DES, NODE_SYSPORT, strport, flagz);

        if (idindex != 0)
        {
            memset(tempz,0,128);
            sprintf(tempz,"%d",enable);
            mod_second_xmlnode(xml_fpath, NODE_LOG, NODE_ENABLES, tempz, idindex);
            mod_second_xmlnode(xml_fpath, NODE_LOG, NODE_SYSIP, ipaddr, idindex);
            mod_second_xmlnode(xml_fpath, NODE_LOG, NODE_SYSPORT, strport, idindex);
            mod_second_xmlnode(xml_fpath, NODE_LOG, CH_FILTER, filterlist[filter], idindex);
        }
    }

    free(tempz);
    free(strport);
    return 0;
}

int del_syslog_server(char *xml_fpath, int idindex)
{
    int flagz=0;
    char gets[50];
    memset(gets,0,50);
    get_second_xmlnode(xml_fpath, NODE_LOG, NODE_MARKZ,&gets,idindex);
    del_second_xmlnode(xml_fpath, NODE_LOG, idindex);

    if (strcmp(gets,"")!=0)
    {
        find_second_xmlnode(xml_fpath, NODE_DES, NODE_MARKZ, gets,&flagz);
        del_second_xmlnode(xml_fpath, NODE_DES, flagz);
    }

    return 0;
}
void save_syslog_file()
{
    SYSLOGALL_ST sysall;
    memset(&sysall,0,sizeof(sysall));
    read_syslogall_st(XML_FPATH, &sysall);
    write_config_syslogallst(&sysall, CONF_FPATH);
    Free_read_syslogall_st(&sysall);
}
void if_syslog_exist()
{
    char *cmd=(char *)malloc(128);
    memset(cmd,0,128);

    if (access(XML_FPATH,0)!=0)
    {
        memset(cmd,0,128);
        sprintf(cmd,"sudo cp %s  %s",XML_SYS_D,XML_FPATH);
        system(cmd);
        memset(cmd,0,128);
        sprintf(cmd,"sudo chmod 666 %s",XML_FPATH);
        system(cmd);
    }

    if (access(CONF_FPATH,0)!=0)
    {
        memset(cmd,0,128);
        sprintf(cmd,"sudo cp %s %s",CONF_SYS_D,CONF_FPATH);
        system(cmd);
        memset(cmd,0,128);
        sprintf(cmd,"sudo chmod 666 %s",CONF_FPATH);
        system(cmd);
    }

    free(cmd);
}
int mod_first_xmlnode(char *xmlpath,char *node_name,char *newc)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    xmlNodePtr design_node = NULL;
    pdoc = xmlReadFile(xmlpath,"utf-8",256);

    if (NULL == pdoc)
    {
        return 1;
    }

    int if_second_flag=0;
    pcurnode = xmlDocGetRootElement(pdoc);
    design_node=pcurnode;
    pcurnode = pcurnode->xmlChildrenNode;

    while (NULL != pcurnode)
    {
        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            xmlNodeSetContent(pcurnode, BAD_CAST  newc);
            if_second_flag=0;
            break;
        }
        else
        {
            if_second_flag=1;
        }

        pcurnode = pcurnode->next;
    }

    if (if_second_flag==1)
    {
        xmlNodePtr node = xmlNewNode(NULL,BAD_CAST node_name);
        xmlAddChild(design_node,node);
        xmlNodePtr content1 = xmlNewText(BAD_CAST newc);
        xmlAddChild(node,content1);
        design_node=NULL;
    }

    xmlSaveFile(xmlpath,pdoc);
    return if_second_flag;
}
int get_first_xmlnode(char *xmlpath,char *node_name,char *newc)
{
    xmlDocPtr pdoc = NULL;
    xmlNodePtr pcurnode = NULL;
    pdoc = xmlReadFile(xmlpath,"utf-8",256);

    if (NULL == pdoc)
    {
        return 1;
    }

    pcurnode = xmlDocGetRootElement(pdoc);
    pcurnode = pcurnode->xmlChildrenNode;

    while (NULL != pcurnode)
    {
        xmlChar *value;

        if (!xmlStrcmp(pcurnode->name, BAD_CAST node_name))
        {
            value = xmlNodeGetContent(pcurnode);
            strcpy(newc,(char *)value);
            xmlFree(value);
        }

        pcurnode = pcurnode->next;
    }

    xmlSaveFile(xmlpath,pdoc);
    return 0;
}

int is_file_exist(char *filtpath)
{
    return access(filtpath, 0);
}

#endif
int snmpd_dbus_connection_init(void)
{
    DBusError dbus_error;
    dbus_error_init(&dbus_error);

    if (NULL == ccgi_dbus_connection)
    {
        ccgi_dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);

        if (ccgi_dbus_connection == NULL)
        {
            printf("dbus_bus_get(): %s", dbus_error.message);
            return CCGI_FAIL;
        }

        dbus_bus_request_name(ccgi_dbus_connection,"aw.snmpd",0,&dbus_error);

        if (dbus_error_is_set(&dbus_error))
        {
            printf("request name failed: %s", dbus_error.message);
            return CCGI_FAIL;
        }
    }

    return CCGI_SUCCESS;
}

int snmpd_dbus_init(void)
{
    //DcliWInit();
    snmpd_dbus_connection_init();
}


/*∂¡»°snmp≈‰÷√ µƒ∫Ø ˝*/
int load_snmp_conf( char *conf_file ,STSNMPSummary *pstSummary)
{
	FILE *fp;		
	char line[256] = {0};
	char deny_ipadd[32]="";
	int i = 0;
	int flag = 1;
	char comm_name[16] = {0};
	char comm_mode[16] = {0};
	char comm_ipdaddr[32] = {0};
	char comm_mask[32] = {0};
	char view_name[32] = {0};
	char view_mode[16] = {0};
	char view_id[32] = {0};
	char access_comm[32] = {0};
	char access_view[32] = {0};
	char trap_ip[32] = {0};
	char trap_portno[8] = {0};
	char username[32] = {0};
	char auth_type[32] = {0};
	char auth_passwd[32] = {0};
	char priv_type[32] = {0};
	char priv_passwd[32] = {0};

    char group_type[16] = {0};
    char group_status[16] = {0};
    int group_status_num = 0;
    

	char *access_view_tmp=NULL;
	
	if(NULL == (fp = fopen( conf_file, "r") ))
	{
		if(NULL == (fp = fopen( "/mnt/snmpd_conf.conf", "r") ))
		{
			if(NULL == (fp = fopen( "/opt/services/conf/snmpd_conf.conf", "r") ))
			{
				return -1;
			}
			system("cp /opt/services/conf/snmpd_conf.conf "CONF_FILE_PATH);
		}
		else
		{
			system("cp /mnt/snmpd_conf.conf "CONF_FILE_PATH);
		}
	}
	while(fgets(line,sizeof(line),fp))
	{
		flag =1;
		if(strncmp(line,"com2sec no_option",strlen("com2sec no_option"))==0)
		{
			
			sscanf(line, "com2sec no_option %32[0-9.]",deny_ipadd);
			for(i=0;i<pstSummary->deny_ip_num;i++)
			{
				if(!strcmp(deny_ipadd,pstSummary->deny_ip[i].ip_addr))
				{
					flag = 0;
				}
			}
			if(flag)
			{
				strcpy(pstSummary->deny_ip[pstSummary->deny_ip_num].ip_addr, deny_ipadd);
				pstSummary->deny_ip_num++;
			}		
			memset(line, 0, 256);
		}
		else if((strncmp(line,"com2sec",strlen("com2sec"))==0)&&(strncmp(line,"com2sec no_option",strlen("com2sec no_option"))!=0))
		{	
			sscanf(line, "com2sec %[0-9a-zA-Z]_%[a-zA-Z] %32[0-9.]/%32[0-9.]",comm_name,comm_mode,comm_ipdaddr,comm_mask);
			for(i=0;i<pstSummary->community_num;i++)
			{
				if(!strcmp(comm_name,pstSummary->community[i].community))
				{
					flag = 0;
				}
			}
			if(flag)
			{
				strcpy(pstSummary->community[pstSummary->community_num].community, comm_name);
				if(!strcmp(comm_mode,"ReadOnly"))
				{
					pstSummary->community[pstSummary->community_num].access_mode = 0;
				}
				else if(!strcmp(comm_mode,"ReadWrite"))
				{
					pstSummary->community[pstSummary->community_num].access_mode = 1;
				}
				strcpy(pstSummary->community[pstSummary->community_num].ip_addr, comm_ipdaddr);
				strcpy(pstSummary->community[pstSummary->community_num].ip_mask, comm_mask);
				pstSummary->community_num++;
			}
			memset(line, 0, 256);
		}
		else if(strncmp(line,"view",strlen("view"))==0)
		{ 
			sscanf(line, "view %s %s %s",view_name, view_mode, view_id);
			for(i=0;i<pstSummary->view_num;i++)
			{
				if(!strcmp(view_name,pstSummary->view[i].name))
				{
					flag = 0;
				}
			}
			if(flag)
			{
				strcpy(pstSummary->view[pstSummary->view_num].name, view_name);
				if(!strcmp(view_mode,"included"))
				{
					pstSummary->view[pstSummary->view_num].view_mode= 0;
				}
				else 
				{
					pstSummary->view[pstSummary->view_num].view_mode = 1;
				}
				strcpy(pstSummary->view[pstSummary->view_num].oid_tree, view_id);
				pstSummary->view_num++;
			}
			memset(line, 0, 256);
		}
		else if((strncmp(line,"access",strlen("access"))==0)&&(strncmp(line,"access NoOptionGroup",strlen("access NoOptionGroup"))!=0))
		{
			sscanf(line, "access %[0-9a-zA-Z]_",access_comm);
			access_view_tmp = strstr(line, "exact");
			sscanf(access_view_tmp+strlen("exact  "),"%[^ ]",access_view);
			strcpy(pstSummary->access[pstSummary->access_num].accommunity, access_comm);
			strcpy(pstSummary->access[pstSummary->access_num].acview, access_view);
			pstSummary->access_num++;
			memset(line, 0, 256);
		}
		else if( strncmp(line,"trapsink",strlen("trapsink"))==0 )
		{
			sscanf(line, "trapsink %32[0-9.] public %s",trap_ip, trap_portno);
			pstSummary->receiver[pstSummary->receiver_num].mode = 0;
			pstSummary->receiver[pstSummary->receiver_num].index = pstSummary->receiver_num+1;
			strcpy(pstSummary->receiver[pstSummary->receiver_num].ip_addr, trap_ip);
			strcpy(pstSummary->receiver[pstSummary->receiver_num].portno, trap_portno);
			pstSummary->receiver_num++;
			memset(line, 0, 256);
		}
		else if( strncmp(line,"trap2sink",strlen("trap2sink"))==0 )
		{
			sscanf(line, "trap2sink %32[0-9.] public %s",trap_ip, trap_portno);
			pstSummary->receiver[pstSummary->receiver_num].mode = 1;
			pstSummary->receiver[pstSummary->receiver_num].index = pstSummary->receiver_num+1;
			strcpy(pstSummary->receiver[pstSummary->receiver_num].ip_addr, trap_ip);
			strcpy(pstSummary->receiver[pstSummary->receiver_num].portno, trap_portno);
			pstSummary->receiver_num++;
			memset(line, 0, 256);
		}
		else if( strncmp(line,"informsink",strlen("informsink"))==0 )
		{
			sscanf(line, "informsink %32[0-9.] public %s",trap_ip, trap_portno);
			pstSummary->receiver[pstSummary->receiver_num].mode = 2;
			pstSummary->receiver[pstSummary->receiver_num].index = pstSummary->receiver_num+1;
			strcpy(pstSummary->receiver[pstSummary->receiver_num].ip_addr, trap_ip);
			strcpy(pstSummary->receiver[pstSummary->receiver_num].portno, trap_portno);
			pstSummary->receiver_num++;
			memset(line, 0, 256);
		}
		else if( strncmp(line,"trapsess",strlen("trapsess"))==0 )
		{
			memset(auth_type, 0 , 32);
			memset(priv_type, 0 , 32);
			if(strstr(line, "-l authPriv"))
			{
			    sscanf(line, "trapsess -e %30[a-zA-Z0-9] -v 3 -u %30[a-zA-Z0-9.] -a %8[a-zA-Z0-9.] -A %21[a-zA-Z0-9.] -l authPriv -x %8[a-zA-Z0-9.] -X %21[a-zA-Z0-9.] %32[0-9.] uptime \n",
						pstSummary->receiver[pstSummary->receiver_num].engineId, 
						pstSummary->receiver[pstSummary->receiver_num].v3user.name, 
						auth_type,
						pstSummary->receiver[pstSummary->receiver_num].v3user.authentication.passwd,
						priv_type,
						pstSummary->receiver[pstSummary->receiver_num].v3user.privacy.passwd, 
						pstSummary->receiver[pstSummary->receiver_num].ip_addr);
				if(strncmp(auth_type, "MD5", 3) == 0)
				{
				    pstSummary->receiver[pstSummary->receiver_num].v3user.authentication.protocal = AUTH_PRO_MD5;
				}
				else if(strncmp(auth_type, "SHA", 3) == 0)
				{
				    pstSummary->receiver[pstSummary->receiver_num].v3user.authentication.protocal = AUTH_PRO_SHA;
				}
				
				if(strncmp(priv_type, "DES", 3) == 0)
				{
				    pstSummary->receiver[pstSummary->receiver_num].v3user.privacy.protocal = PRIV_PRO_DES;
				}
				else if(strncmp(priv_type, "AES", 3) == 0)
				{
				    pstSummary->receiver[pstSummary->receiver_num].v3user.privacy.protocal = PRIV_PRO_AES;
				}
			}
			else if(strstr(line, "-l authNoPriv"))
			{
			    sscanf(line, "trapsess -e %30[a-zA-Z0-9] -v 3 -u %32[a-zA-Z0-9.] -a %8[a-zA-Z0-9.] -A %21[a-zA-Z0-9.] -l authNoPriv %32[0-9.] uptime \n",
						pstSummary->receiver[pstSummary->receiver_num].engineId, 
						pstSummary->receiver[pstSummary->receiver_num].v3user.name, 
						auth_type,
						pstSummary->receiver[pstSummary->receiver_num].v3user.authentication.passwd,
						pstSummary->receiver[pstSummary->receiver_num].ip_addr);
				if(strncmp(auth_type, "MD5", 3) == 0)
				{
				    pstSummary->receiver[pstSummary->receiver_num].v3user.authentication.protocal = AUTH_PRO_MD5;
				}
				else if(strncmp(auth_type, "SHA", 3) == 0)
				{
				    pstSummary->receiver[pstSummary->receiver_num].v3user.authentication.protocal = AUTH_PRO_SHA;
				}
				pstSummary->receiver[pstSummary->receiver_num].v3user.privacy.protocal = PRIV_PRO_NONE;

			}
			else if(strstr(line, "-l noAuthNoPriv"))
			{
			    sscanf(line, "trapsess -e %30[a-zA-Z0-9] -v 3 -u %32[a-zA-Z0-9.] -l noAuthNoPriv %32[0-9.] uptime \n",
						pstSummary->receiver[pstSummary->receiver_num].engineId, 
						pstSummary->receiver[pstSummary->receiver_num].v3user.name,
						pstSummary->receiver[pstSummary->receiver_num].ip_addr);
				pstSummary->receiver[pstSummary->receiver_num].v3user.authentication.protocal = AUTH_PRO_NONE;
				pstSummary->receiver[pstSummary->receiver_num].v3user.privacy.protocal = PRIV_PRO_NONE;
			}
			
			pstSummary->receiver[pstSummary->receiver_num].mode = 3;
			pstSummary->receiver[pstSummary->receiver_num].index = pstSummary->receiver_num+1;
			strcpy(pstSummary->receiver[pstSummary->receiver_num].portno, "162");
			pstSummary->receiver_num++;
			memset(line, 0, 256);
		}
		else if( strncmp(line,"createUser",strlen("createUser"))==0)
		{
			memset(auth_type, 0 , 32);
			memset(priv_type, 0 , 32);
			memset(username, 0, 32);
			sscanf(line, "createUser %32[a-zA-Z0-9.] %8[a-zA-Z0-9.] %21[a-zA-Z0-9.] %8[a-zA-Z0-9.] %21[a-zA-Z0-9.]",username, auth_type, auth_passwd, priv_type, priv_passwd);
			for(i=0;i<pstSummary->v3user_num;i++)
			{
				if(!strcmp(pstSummary->v3user[i].name,username))
				{
					flag = 0;
				}
			}
			if(flag)
			{
				strcpy(pstSummary->v3user[pstSummary->v3user_num].name,username);
				if(!strcmp(auth_type, "MD5"))
				{
					pstSummary->v3user[pstSummary->v3user_num].authentication.protocal = AUTH_PRO_MD5;
				}
				else if(!strcmp(auth_type, "SHA"))
				{
					pstSummary->v3user[pstSummary->v3user_num].authentication.protocal = AUTH_PRO_SHA;
				}
				else
				{
					pstSummary->v3user[pstSummary->v3user_num].authentication.protocal = AUTH_PRO_NONE;
				}
                
				strcpy(pstSummary->v3user[pstSummary->v3user_num].authentication.passwd,auth_passwd);
				if(!strcmp(priv_type, "DES"))
				{
					pstSummary->v3user[pstSummary->v3user_num].privacy.protocal = PRIV_PRO_DES;
				}	
				else
				{
					pstSummary->v3user[pstSummary->v3user_num].privacy.protocal = PRIV_PRO_NONE;
				}
				strcpy(pstSummary->v3user[pstSummary->v3user_num].privacy.passwd,priv_passwd);
				pstSummary->v3user_num++;		
			}
			memset(line, 0, 256);
		}
		else if(strncmp(line,"rouser",strlen("rouser"))==0)
		{
			memset(username, 0, 32);
			sscanf(line, "rouser %s",username);
			for(i=0;i<pstSummary->v3user_num;i++)
			{
				if(!strcmp(pstSummary->v3user[i].name,username))
				{
					pstSummary->v3user[i].access_mode = 0;
				}
			}
			memset(line, 0, 256);
		}
		else if(strncmp(line,"rwuser",strlen("rwuser"))==0)
		{
			memset(username, 0, 32);
			sscanf(line, "rwuser %s",username);
			for(i=0;i<pstSummary->v3user_num;i++)
			{
				if(!strcmp(pstSummary->v3user[i].name,username))
				{
					pstSummary->v3user[i].access_mode = 1;
				}
			}
			memset(line, 0, 256);
		}
        else if(0 == strncmp(line,"group_status",strlen("group_status")))
		{ 
			sscanf(line, "group_status %s %s", group_type, group_status);

            if(0 == strncmp(group_status,"disable",strlen("disable")))
                group_status_num = DISABLE_STATUS;
            else
                group_status_num = ENABLE_STATUS;

            if(!strncmp(group_type,"v1",strlen("v1")))
            {
                pstSummary->snmp_sysinfo.v1_status = group_status_num;
            }
            else if(!strncmp(group_type,"v2c",strlen("v2c")))
            {
                pstSummary->snmp_sysinfo.v2c_status = group_status_num;
            }
            else if(!strncmp(group_type,"usm",strlen("usm")))
            {
                pstSummary->snmp_sysinfo.v3_status = group_status_num;
            }

			memset(line, 0, 256);
		}
	}
	fclose(fp);
	return 0;
}
#endif

#ifdef HAVE_BRIDGE_STP
int man_get_brg_g_state(int *stpmode)
{
    DBusMessage *query, *reply;
    DBusError err;
    int op_ret = 0;
    int mode = 0;
    query = dbus_message_new_method_call(
                RSTP_DBUS_NAME,    \
                RSTP_DBUS_OBJPATH,    \
                RSTP_DBUS_INTERFACE,    \
                RSTP_DBUS_METHOD_GET_PROTOCOL_STATE);
    dbus_error_init(&err);
    reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
    dbus_message_unref(query);

    if (NULL == reply)
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    }

    if (dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&op_ret,
                              DBUS_TYPE_UINT32,&mode,
                              DBUS_TYPE_INVALID))
    {
        *stpmode = mode;
        /*printf("stp dcli stpmode value is %d",*stpmode);*/
    }
    else
    {
        printf("Failed get args.\n");

        if (dbus_error_is_set(&err))
        {
            printf("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    }

    /*printf(" stp return op_ret value %d and stpmode value %d\n",op_ret,*stpmode);*/
    dbus_message_unref(reply);
    return op_ret;
}

#endif

