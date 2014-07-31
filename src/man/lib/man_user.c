
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
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
#include "man_user.h"
#include "npd/nbm/npd_cplddef.h"

/* Execute command in child process. */
 int execute_dcli_shell (const char *command)
{		
	return system(command);

}

int is_admin_user(char* name)
{
	struct passwd *pwd = NULL;

	pwd = getpwuid(getuid());
	if(!pwd)
		return -1;
	if(strcmp(pwd->pw_name,"admin")) 
		return -1;
	return 1;
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

int is_user_self(char* name)
{
/*
    int name_len = 0;
    char *current_user = getenv("LOGNAME");
    name_len = strlen(name);
    if(name_len != strlen(current_user)){
        return 0;
    }
    if(!strncmp(name,current_user,strlen(current_user)))
        return 1;
    else
        return 0;
*/
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

int get_user_role(char* name)
{
	int uid,i;
	struct passwd *passwd;
	struct group *group=NULL;

	group = getgrnam(ADMINGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 1;

	}
	
	group = getgrnam(VIEWGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 0;

	}
	return -1;

}

int get_self_role()
{
	int uid,i;
	struct passwd *passwd = NULL;
	struct group *group=NULL;
	char *name;

	passwd = getpwuid(getuid());
	if(passwd)
		name = passwd->pw_name;
	else
		return -1;
	group = getgrnam(ADMINGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 1;

	}
	
	group = getgrnam(VIEWGROUP);
	if(group && group->gr_mem )
	{
		for(i=0;group->gr_mem[i];i++)
			if(!strcmp(group->gr_mem[i],name))
				return 0;

	}
	return -1;

}

/****************************************
*added user to system
*input 
*       name: user name
*	  passwd: user password
*output 
* 	  none
*return
*       0     OK
*       -1   the user is exist
*       -2   system error
*
*****************************************/

int dcli_user_add_sh(const char* name,const char* password,char* enable,char* sec)
{
	char command[128];
	if(!name || !password || !enable || !sec)
		return -3;
	sprintf(command,"useradd.sh %s \'%s\' %s %s",name,password,enable,sec);
	return execute_dcli_shell(command);

}

int dcli_user_name_check(char* name)
{
	int i = 0;
	int len ;
	char tmp;
	if(!name)
		return 1;
	len = strlen(name);
	if(len < 4 || len > 32)
		return 2;
	tmp = *name;
	if(!(tmp>='a' && tmp<='z')&&!(tmp>='A' &&tmp<='Z'))
		return 3;
	for(i = 0;i<len ;i++)
	{
		tmp = *(name+i);
		if(tmp == '_' || (tmp>='0' && tmp<='9')||(tmp>='a' &&tmp<='z')||(tmp>='A' &&tmp<='Z'))
		{
			continue;
		}
		else
		{
			return 1;
		}
	}
	return 0;
	
}
int dcli_user_passwd_check(char* passwd)
{
	int i = 0;
	int len ;
	char tmp;
	if(!passwd)
		return 1;
	len = strlen(passwd);
	if(len < 6 || len > 16)
		return 2;
	for(i = 0;i<len ;i++)
	{
		tmp = *(passwd+i);
		if((tmp>='0' &&tmp<='9')||(tmp>='a' &&tmp<='z')||(tmp>='A' &&tmp<='Z')|| tmp == '_')
		{
			continue;
		}
		else
		{
			return 1;
		}
	}
	return 0;

}

/* 0 is legal ,-1 is illegal */
int is_user_legal(char *name,char *passwd)
{
    int ret=-1;
    ret = dcli_user_name_check(name);	
	if(ret == 1)
	{
		printf("the user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'\n");
		return -1;
	}
	else if(ret == 2)
	{
		printf("the user name length should be >=4 & <=32\n");
		return -1;
	}
	else if(ret == 3)
	{
		printf("the user name first char  should be 'A'-'Z' or 'a'-'z'\n");
		return -1;
	}
    ret = dcli_user_passwd_check(passwd);
	if(ret == 1)
	{
		printf("the user password should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'\n");
		return -1;
	}	
	else if(ret == 2)
	{
		printf("the user password length should be >=6 & <=16\n");
		return -1;
	}
    if(!is_user_exsit(name))		
	{
		printf("the user %s doesn't exist\n",name);
		return -1;
	}
    return 0;
}

int sync_add_user(char *name,char *pwd,char *role,int encrypt,char *err_msg)
{
    int ret =-1;
	char passwd[32];
    char para[128];
	ret = dcli_user_name_check(name);	
	if(ret == 1)
	{
		memcpy(err_msg,"the user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'",strlen("the user name should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'"));
		return -1;
	}
	else if(ret == 2)
	{
		memcpy(err_msg,"the user name length should be >=4 & <=32",strlen("the user name length should be >=4 & <=32"));
		return -1;
	}
	else if(ret == 3)
	{
		memcpy(err_msg,"the user name first char  should be 'A'-'Z' or 'a'-'z'",strlen("the user name first char  should be 'A'-'Z' or 'a'-'z'"));
		return -1;
	}
	if(!encrypt)
	{
		sprintf(passwd,"normal");
		ret = dcli_user_passwd_check(pwd);
		if(ret == 1)
		{
			memcpy(err_msg,"the user password should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'",strlen("the user password should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'"));
			return -1;
		}	
		else if(ret == 2)
		{
			memcpy(err_msg,"the user password length should be >=6 & <=16",strlen("the user password length should be >=6 & <=16"));
			return -1;
		}
	}
	else
	{
		sprintf(passwd,"security");
	}

	if(is_user_exsit(name))		
	{
		sprintf(err_msg,"the user %s is exist",name);
		return -1;
	}
    sprintf(para,"%s %s %s",name,pwd,role);
	ret = db_update_via_para(ADD_USER, 3, para, passwd);
	return ret;
    
}

int sync_delete_user(char *name,char *err_msg)
{
    int ret = -1;	
	if(!is_user_exsit(name))		
	{
		sprintf(err_msg,"the user %s is not exist",name);
		return -1;
	}
	ret = is_user_self(name);
	if(ret == 1)
	{
		sprintf(err_msg,"Can't del user %s self",name);
		return -1;
	}
	else if(ret == -1)
	{
		sprintf(err_msg,"the user %s is not exist",name);
		return -1;
	}
	
	if(get_user_role(name) == 1)
	{
		sprintf(err_msg,"Can't del user %s which belong admin group",name);
		return -1;
	}
	ret = db_entry_delete_via_para(DEL_USER, 1, name, NULL);
	return ret;
    
}

int sync_change_user_role(char *name,char *role,char *err_msg)
{
    int ret = -1;
    char para[64];
	if(!strcmp(name,"admin"))
	{
		memcpy(err_msg,"can't change user 'ADMIN' role",strlen("can't change user 'ADMIN' role "));
		return ret;
	}
    sprintf(para,"%s %s",name,role);
	ret = db_update_via_para(CHANGE_ROLE, 2, para, NULL);
	return ret;
    
}

int sync_change_user_pwd(char *name,char *new_pwd,int encrypt,char *err_msg)
{
    int ret = -1;
	char passwd[10];
    char para[64];
	if(!is_user_exsit(name))		
	{
		sprintf(err_msg,"the user %s is not exist\n",name);
		return -1;
	}
	if(!encrypt)
	{
		sprintf(passwd,"normal");
		ret = dcli_user_passwd_check(new_pwd);
		if(ret == 1)
		{
			memcpy(err_msg,"the user password should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'",strlen("the user password should be 'A'-'Z'  'a'-'z' '1'-'9'or '_'"));
			return -1;
		}	
		else if(ret == 2)
		{
			memcpy(err_msg,"the user password length should be >=6 & <=16",strlen("the user password length should be >=6 & <=16"));
			return -1;
		}
	}
	else
	{
		sprintf(passwd,"security");
	}
    sprintf(para,"%s %s",name,new_pwd);
	ret = db_update_via_para(CHANGE_PWD, 2, para, passwd);
	return ret;
    
}

int get_next_admin_user(int number,char *name)
{
    int i=0;
    int ret=-1;
	struct group *grentry = NULL;
	char *ptr;
	grentry = getgrnam(ADMINGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
		{
            if(number==i+1)
            {
                memcpy(name,ptr,strlen(ptr));
                name[strlen(ptr)] = '\0';
                ret=0;
                break;
            }
		}
	}
    return ret;
    
}
int get_next_view_user(int number,char *name)
{
    int i=0;
    int ret=-1;
	struct group *grentry = NULL;
	char *ptr;
	grentry = getgrnam(VIEWGROUP);
	if (grentry)
	{
		for(i=0;ptr=grentry->gr_mem[i];i++)
		{
            if(number==i+1)
            {
                memcpy(name,ptr,strlen(ptr));
                name[strlen(ptr)] = '\0';
                ret=0;
                break;
            }
		}
	}
    return ret;
    
}
