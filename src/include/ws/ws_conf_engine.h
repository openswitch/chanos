/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/AuteCS/cgic205/portal/ws_conf_engine.h,v $
*$Author: tangsiqi $
*$Date: 2009/06/11 06:27:38 $
*$Revision: 1.3 $
*$State: Exp $
*
*
*/

#ifndef _WS_CONF_ENGINE_H
#define _WS_CONF_ENGIEN_H

#include <stdio.h>
#include <sys/wait.h>

#define SCRIPT_FILE_PATH	"sudo /opt/services/init/eag_init"
#define EAG_STATUS_FILE		"/opt/services/status/eag_status.status"

#define MAX_CONF_NAME_LEN	32
#define MAX_CONF_VALUE_LEN	128
#define MAX_SHOW_KEY_LEN	32


#define RTN_ERR				-1
#define RTN_ERR_FILE_OPEN	(RTN_ERR-1)	

#define NASID_CONF_PATH "/opt/services/conf/nasidvlan_conf.conf"
#define NASID_MAX_NUM 	512
#define MAX_INDEX_LEN 	32

typedef struct {
	char index[MAX_INDEX_LEN];
	char start_vlan[MAX_INDEX_LEN];
	char end_vlan[MAX_INDEX_LEN];
	char nas[MAX_INDEX_LEN];
	int  nas_port_id;
}NAS_VLAN_ID;


typedef struct st_conf_item{
	char conf_name[MAX_CONF_NAME_LEN];
	char conf_value[MAX_CONF_VALUE_LEN];
	char show_key[MAX_SHOW_KEY_LEN];
	int  show_flag;//当这个值为1时才会显示出来。
	char *error;
	//这些回调函数都是由上层代码设置的。
	int  (*output_html_obj)( struct st_conf_item *this, void *param );//输出input或者其它输入对象的html代码
	int  (*get_value_user_input)( struct st_conf_item *this, void *param );//得到相应的用户输入，并进行错误检查,返回错误id
}t_conf_item;

//装载conf文件
int load_conf_file( char *file_path, t_conf_item pt_conf_item[], int max_num );
int load_conf_file_ex( char *file_path, t_conf_item pt_conf_item[], int max_num );


//保存conf文件
int save_conf_file( char *file_path, t_conf_item pt_conf_item[], int max_num );

//获得服务状态
int eag_services_status();
//开启服务
int eag_services_start();
//关闭服务
int eag_services_stop( );
//重启服务
int eag_services_restart();
//idle time 
int idle_tick_status();

#endif
