#ifndef __MAN_ACL_H__
#define __MAN_ACL_H__

#define	MAX_ACL_RULE_NUMBER						1000
#define MAX_GROUP_NUM							1024
#define SHOW_SERVICE_SIZE                       (128 * 1024)

#define STANDARD_ACL_RULE 0
#define EXTENDED_ACL_RULE 1
#define MAX_IP_STRLEN 16

#define ACCESS_PORT_TYPE 0
#define ACCESS_VID_TYPE  1

#define ALIAS_NAME_SIZE 		0x15
#define ACL_MATCH_ERROR_MSG_LEN	0x64
#define ALIAS_NAME_LEN_ERROR	0x1
#define ALIAS_NAME_HEAD_ERROR	(ALIAS_NAME_LEN_ERROR+1)
#define ALIAS_NAME_BODY_ERROR	(ALIAS_NAME_LEN_ERROR+2)
#define TIME_SPLIT_DASH 	'/'
#define TIME_SPLIT_SLASH	':'

#define NPD_ACL_RULE_SHOWRUN_CFG_SIZE (3000*1024)

#define ACL_TIME_NAME_EXIST    1
#define ACL_TIME_NAME_NOTEXIST 2
#define ACL_TIME_PERIOD_NOT_EXISTED 3
#define ACL_TIME_PERIOD_EXISTED 4
#define MAX_EXT_RULE_NUM	500
#define ACL_ANY_PORT NPD_ACL_RULE_SHOWRUN_CFG_SIZE

struct xgress_group_g
{
	unsigned int	g_index;
	unsigned int	group_index;
	unsigned int	acl_count;
	unsigned int	port_count;
	unsigned int	rule_index[MAX_ACL_RULE_NUMBER];
	unsigned int*	group_bind_port;
};

struct voice_vlan_s{
	int		global_enable;
	char	is_enable;
	char	is_trust;
	char	cmd_arg_pri;
	char	cmd_arg_cosq;
	char	cmd_arg_dscp;
	char	cmd_name_flag;
	int		cmd_arg_value;
};

#endif
