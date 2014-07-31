#ifndef __MAN_USER_H__
#define __MAN_USER_H__

#define VIEWGROUP "vtyview"
#define ADMINGROUP "vtyadmin"

struct dcli_user
{
  char *name;
  char role;
  char *passwd;
};
#define CONSOLEPWDFILE "/etc/ttyS0pwd"

int execute_dcli_shell (const char *command);
int is_admin_user(char* name);
int is_user_exsit(char* name);
int is_user_self(char* name);
int get_user_role(char* name);
int get_self_role();
int dcli_user_add_sh(const char* name,const char* password,char* enable,char* sec);
int dcli_user_name_check(char* name);
int dcli_user_passwd_check(char* passwd);
int get_next_admin_user(int number,char *name);
int get_next_view_user(int number,char *name);
int is_user_legal(char *name,char *passwd);
int sync_add_user(char *name,char *pwd,char *role,int encrypt,char *err_msg);
 int sync_delete_user(char *name,char *err_msg);
 int sync_change_user_role(char *name,char *role,char *err_msg);
 int sync_change_user_pwd(char *name,char *new_pwd,int encrypt,char *err_msg);


#endif

