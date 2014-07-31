#ifndef _WS_USERINFO_H
#define _WS_USERINFO_H

#include<stdio.h>                              /*输入输出头文件*/
#include<string.h>                             /*字符分类头文件*/
#include<stdlib.h>   						   /*动态存储分配头文件*/
#include <ctype.h>

/*get_user()*/
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#define N 50                                   /*用户名和密码的长度*/   
#define BUF_LEN 128                           /*加密字符串的长度*/

#define VIEWGROUP "vtyview"                    /*vtyview用户组*/
#define ADMINGROUP "vtyadmin"                  /*vtyadmin管理员组*/

#define PATH_LENG 512 /*路径字符串长度 */

#define MAX_USER_NUM 15

#define USER_LOG_IN "/var/run/cpp/web_session.xml"
#define DIFF_TIME 300         /*访问系统的时间间隔门限值，如果用户最近两次访问系统的时间间隔大于该值，则退出系统重新登陆*/

///////////add by tangsiqi 2009-5-25
#define WEB_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255"))
#define WEB_IPMASK_STRING_MINLEN	(strlen("0.0.0.0"))
#define WEB_MAC_STRING_MINLEN	(strlen("00:00:00:00:00:00"))


#define INPUT_OK 				0
#define INPUT_ERROR				-1

#define INPUT_NULL				INPUT_ERROR-1
#define INPUT_LENGTH_ERROR		INPUT_ERROR-2
#define INPUT_FORMAT_ERROR		INPUT_ERROR-3
#define INPUT_CHAR_ERROR		INPUT_ERROR-4
#define INPUT_IP_MASK_ERROR		INPUT_ERROR-5
#define INPUT_IP_ERROR			INPUT_ERROR-6







typedef struct  {
       int session_id;
       char user_name[N];
	   int access_time;           /*用户登录时间*/
	   int last_access_time;      /*用户最后访问时间*/
}SESSION_PROFILE;


//char* dcryption(char *str);
char* encryption(char *user, char *pass);
char* dcryption(char *session_id);



//int get_user(char* groupname);             /*获得用户信息*/
int set_deluser(char* groupnme,char* current_user);          /*填充要删除用户的信息*/
char *setclour(int i);                     /*设置列表显示颜色*/
int checkuser_exist(char* username);     /*检查用户是否存在*/
int checkuser_group(char* username);    /*判断用户权限*/
int get_integrate_config_file(char *integrate_default, char *config_file); /*获得配置文件路径*/

int checkpassword(char *password);//检查用户密码是否有非法字符
int checkPoint(char *ptr);	//检查是否数值
int str2ulong(char *str,unsigned int *Value,int min_num,int max_num); //字符串转化ulong函数

int new_user_profile();
void add_user_infor(SESSION_PROFILE session);
void del_user_infor(char* session_id);
void modify_Last_AccTime(char* session_id,int new_access_time);
int Search_user_infor_byID(char* session_id,int ser_type,char* ser_result);
int Search_user_infor_byName(char* user_name,int ser_type,char* ser_result);
int create_user_infor(char *user_name);


//////add by tangsiqi  2009-5-25/////////////
int Input_IP_address_Check(char * ipAddress);
int Input_MAC_address_Check(char * macAddress);
int Input_User_Name_Check(char * iName);
int Input_User_Index_Check(char * iIndex);
int WEB_Pagination_get_range(int pageNumForREQ, int perPage_show_data_num, int * showHead, int *showTail);

int if_user_outtime(char *user_name);

int update_usrtime(char *session_id);

#endif

