/*
* Copyright (c) 2008,Autelan - AuteCS
* All rights reserved.
*
*$Source: /rdoc/AuteCS/cgic205/snmp_agent/ws_snmpd_engine.h,v $
*$Author: qiaojie $
*$Date: 2009/06/16 08:57:24 $
*$Revision: 1.14 $
*$State: Exp $
*
*$Log: ws_snmpd_engine.h,v $
*Revision 1.14  2009/06/16 08:57:24  qiaojie
*消除编译警告
*
*Revision 1.13  2009/06/01 06:43:19  tangsiqi
*no message
*
*Revision 1.12  2009/05/19 10:01:04  tangsiqi
*修改OID生成规则，初始化SNMP出厂配置
*
*Revision 1.11  2009/05/11 10:43:51  tangsiqi
*no message
*
*Revision 1.10  2009/05/07 11:14:44  tangsiqi
*for trap mib
*
*Revision 1.9  2009/05/07 09:57:52  tangsiqi
*no message
*
*Revision 1.8  2009/04/29 09:12:35  tangsiqi
*no message
*
*Revision 1.7  2009/04/09 07:32:44  tangsiqi
*完成对OEM的OID兼容
*
*Revision 1.6  2009/03/23 06:36:06  shaojunwu
*添加release函数，与init函数对应
*
*Revision 1.5  2009/01/05 10:42:52  tangsiqi
*for sysname in miblib
*
*Revision 1.4  2009/01/05 10:02:41  tangsiqi
*no message
*
*Revision 1.3  2008/12/30 09:59:07  tangsiqi
*no message
*
*Revision 1.2  2008/12/29 06:02:27  tangsiqi
*snmp基本功能版
*
*Revision 1.1  2008/12/16 02:32:41  tangsiqi
*snmp module
*
*
*/

#ifndef _WS_SNMPD_ENGINE_H
#define _WS_SNMPD_ENGINE_H

#include <stdio.h>
#include <stdlib.h>

#define	XML_FILE_PATH	"/var/run/snmpd_option"
#define CONF_FILE_PATH		"/var/run/snmpd_conf.conf"
#define STATUS_FILE_PATH		"/var/run/snmpd_status.status"
#define SUB_STATUS_FILE_PATH		"/var/run/subagent_status.status"
#define TRAP_STATUS_FILE_PATH		"/var/run/trap-helper_status.status"




#define SNMP_ENGINE_VERSION			0x00000001
#define SNMP_ENGINE_VERSION_MASK	0xffffff00
#define SNMP_ENGINE_MAX_INFO_LEN	64
#define SNMP_ENGINE_INFO_STR		"AuteCS snmp agent web config engine data"


#define SNMP_AGENG_PORT			161
#define SNMP_TRAP_DEFAULT_PORT	162


#define SCRIPT_PATH				"sudo /opt/services/init/snmpd_init"
#define SCRIPT_PARAM_START		"start"
#define SCRIPT_PARAM_RESTART	"restart"
#define SCRIPT_PARAM_STOP		"stop"
 

//定义结构体及相关参数
#define MAX_SNMP_NAME_LEN 	30
#define MAX_IP_ADDR_LEN		32
#define MAX_TREE_NODE_LEN	64

#define MIN_SNMP_PASSWORD_LEN	12
#define MAX_SNMP_PASSWORD_LEN	24


#define MAX_COMMUNITY_NUM		10
#define MAX_SNMPV3_USER_NUM		10
#define MAX_TRAPRECEIVER_NUM	10
#define MAX_SNMP_VIEW_NUM		20
#define MAX_RISTRICT_IP_NUM		10
#define MAX_SNMP_ACCESS_NUM		20

#define MAX_OID_LEN				256
#define MAX_SYSTEM_NAME_LEN		128
#define MAX_SYSTEM_DESCRIPTION	128
#define MAX_FILE_CONTENT		50000  //文件内字符串数目大小
#define MAX_MIB_SYSNAME			128
#define HOST_NAME_LENTH			128


#if 0
//because the sizeof enum is different in different compiler, so use #define
typedef enum{
	ACCESS_MODE_RO,
	ACCESS_MODE_RW
}ACCESS_MODE;
#else

#define ACCESS_MODE		int
#define VIEW_MODE int
#define	ACCESS_MODE_RO	0
#define ACCESS_MODE_RW	1

#endif

#if 0

typedef enum{
	RULE_ENABLE,
	RULE_DISABLE
}CONF_STATUS;

#else

#define CONF_STATUS		int
#define RULE_ENABLE		0
#define RULE_DISABLE	1

#endif
//community
typedef struct {
	char	community[MAX_SNMP_NAME_LEN+1];
	char	ip_addr[MAX_IP_ADDR_LEN];
	char	ip_mask[MAX_IP_ADDR_LEN];
	
	ACCESS_MODE	access_mode;
	CONF_STATUS	status;
}STCommunity;


//v3user
#if 0
typedef enum{
	AUTH_PRO_NONE,
	AUTH_PRO_MD5,
	AUTH_PRO_SHA
}AUTH_PROTOCAL;
#else

#define	AUTH_PROTOCAL	int
#define AUTH_PRO_NONE	0
#define AUTH_PRO_MD5	1
#define AUTH_PRO_SHA	2
#endif

#if 0
typedef enum{
	PRIV_PRO_NONE,
	PRIV_PRO_AES,
	PRIV_PRO_DES
}PRIV_PROTOCAL;
#else
#define	PRIV_PROTOCAL	int
#define PRIV_PRO_NONE	0
#define PRIV_PRO_AES	1
#define PRIV_PRO_DES	2
#endif

typedef struct {
	char 		name[MAX_SNMP_NAME_LEN+1];
	ACCESS_MODE access_mode;
	
	//authentication  info
	struct {
		AUTH_PROTOCAL protocal;
		char	passwd[MAX_SNMP_PASSWORD_LEN+1];
	}	authentication;
	//privacy  info
	struct {
		PRIV_PROTOCAL protocal;
		char	passwd[MAX_SNMP_PASSWORD_LEN+1];
	}	privacy;
	
	CONF_STATUS status;	
}STSNMPV3User;

//trap receiver
typedef struct {
	unsigned int index;
	char 	name[MAX_SNMP_NAME_LEN+1];
	char	ip_addr[MAX_IP_ADDR_LEN];
	char 	portno[32];
	int		mode;
	CONF_STATUS status;
	char engineId[32];
	STSNMPV3User v3user;
}STSNMPTrapReceiver;

//system info
typedef struct {
	char sys_name[MAX_SYSTEM_NAME_LEN];
	char sys_description[MAX_SYSTEM_DESCRIPTION];
	char sys_oid[MAX_OID_LEN];
	unsigned int agent_port;
	unsigned int trap_port;
	
	CONF_STATUS	v1_status;
	CONF_STATUS	v2c_status;
	CONF_STATUS	v3_status;	

	int 		mtThreadSwitch;
	int			logSwitch;
	int 		cache_time;
}STSNMPSysInfo;

typedef struct {
	char	name[MAX_SNMP_NAME_LEN+1];
	char	oid_tree[MAX_TREE_NODE_LEN];
	
	VIEW_MODE	view_mode;
}STSNMPView;

typedef struct {
	char	ip_addr[MAX_IP_ADDR_LEN];
	
}STSNMPIP;

typedef struct {
	char	accommunity[MAX_SNMP_NAME_LEN+1];
	char	acview[MAX_SNMP_NAME_LEN+1];
	
}STSNMPAccess;

//all conf
typedef struct {
	int 				deny_ip_num;
	STSNMPIP			deny_ip[MAX_RISTRICT_IP_NUM];
	
	char				info[SNMP_ENGINE_MAX_INFO_LEN];
	int 				version;
	
	STSNMPSysInfo		snmp_sysinfo;
	 
	int 				community_num;	
	STCommunity			community[MAX_COMMUNITY_NUM];
	
	int					v3user_num;
	STSNMPV3User		v3user[MAX_SNMPV3_USER_NUM];
	
	int					receiver_num;
	STSNMPTrapReceiver	receiver[MAX_SYSTEM_DESCRIPTION];

	int					view_num;
	STSNMPView			view[MAX_SNMP_VIEW_NUM];

	int					access_num;
	STSNMPAccess		access[MAX_SNMP_ACCESS_NUM];

}STSNMPSummary;



//product_oid
typedef struct {
	unsigned int 	value;
	char * 			product_oid;
}PRODUCT_OID_STATE;

typedef struct {
	unsigned int 	value;
	char * 			product_type;
	char * 			product_node;
}PRODUCT_OID_TYPE;


#ifndef OEM_COM
#define OEM_COM
#define ENTERPRISE_OID_LENGTH 20
#define PRODUCT_OID_LENGTH 20
#define TOTAL_OID_LENGTH 128
#define ENTERPRISE_NODE_PATH	"/devinfo/enterprise_snmp_oid"
#define ENTERPRISE_NODE_SH	" cat /devinfo/enterprise_snmp_oid"

#define PRODUCT_NODE_PATH		"/devinfo/snmp_sys_oid"
#define PRODUCT_NODE_SH		"cat /devinfo/snmp_sys_oid"

#define PRODUCT_TYPE__NODE_PATH	"/devinfo/product_type"

#define ENTERPRISE_OID 	"31656"
#define DEVICEINFO		"2.4"
#define MAX_OID_LENTH 	128


#endif


#if 0

#define PRODUCT_ID_GENERATE(stack, slot, asic, device)	\
	(((stack & 0xF) << 20) | ((slot & 0xF) << 16) | ((asic & 0xF) << 12) | ((device & 0xF) << 8))

/*
  * Full product type id definition, communicated between control processes and management processes
  * currently supported product id as follows:
  *		PRODUCT_ID_AX7K 	- 0xC5A000	(chassis,5 slots, marvell, cheetah2 275)
  *		PRODUCT_ID_AX5K 	- 0xB1A300	(box, 1 slots, marvell, cheetah2 265_lite<only use 12GE+2XG>)
  *		PRODUCT_ID_AU4K 	- 0xB1A100	(box, 1 slots, marvell, cheetah2 265)
  *		PRODUCT_ID_AU3K 	- 0xB1A200	(box, 1 slots, marvell, cheetah2 255)
  *		PRODUCT_ID_AU3K_BCM - 0xB1B000 	(box, 1 slots, broadcom, raven 56024B0)
  *
*/
enum product_id_e {
	PRODUCT_ID_NONE,
	PRODUCT_ID_AX7K = PRODUCT_ID_GENERATE(PRODUCT_STK_CHASSIS_E, 5, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_275_E),
	PRODUCT_ID_AX5K = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_265L_E), 
	PRODUCT_ID_AU4K = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_265_E),
	PRODUCT_ID_AU3K = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_MV_E, ASIC_MV_CHEETAH2_255_E),
	PRODUCT_ID_AU3K_BCM = PRODUCT_ID_GENERATE(PRODUCT_STK_BOX_E, 1, PRODUCT_ASIC_BCM_E, ASIC_BCM_RAVEN_56024B0_E),
	PRODUCT_ID_MAX
};
#endif
/*
enum module_id_e {
	MODULE_ID_NONE ,				
	MODULE_ID_AX7_CRSMU = PRODUCT_ID_AX7K + 1,       
	MODULE_ID_AX7_6GTX ,        
	MODULE_ID_AX7_6GE_SFP,
	MODULE_ID_AX7_XFP ,
	MODULE_ID_AX7_6GTX_POE ,   //not implemented currently 
	MODULE_ID_AX5_5612 = PRODUCT_ID_AX5K + 1,
	MODULE_ID_AU4_4626 = PRODUCT_ID_AU4K + 1,
	MODULE_ID_AU4_4524 , //not implemented currently 
	MODULE_ID_AU4_4524_POE , //not implemented currently 
	MODULE_ID_AU3_3524 = PRODUCT_ID_AU3K + 1,
	MODULE_ID_AU3_3028 = PRODUCT_ID_AU3K_BCM + 1, //not implemented currently 
	MODULE_ID_AU3_3052,
	MODULE_ID_MAX     
};*/



//定义错误消息,大于0为warning，小于0为error
//SE  = SNMPD ENGINE
#define SE_OK	0

#define SE_WARNING_INITIALIZED		(SE_OK+1)

#define SE_ERROR					(SE_OK-1)
#define SE_ERROR_ERR_INPUT			(SE_OK-2)
#define SE_ERROR_MALLOC_FAILED		(SE_OK-3)
#define SE_ERROR_NOT_INITIAL		(SE_OK-4)
#define SE_ERROR_COMMU_MAX_LIMIT	(SE_OK-5)
#define SE_ERROR_V3USER_MAX_LIMIT	(SE_OK-6)
#define SE_ERROR_TRAP_MAX_LIMIT		(SE_OK-7)
#define SE_ERROR_FILE_NOT_EXIST		(SE_OK-8)
#define SE_ERROR_XML_FILE_ERR		(SE_OK-9)
#define SE_ERROR_NEW_FILE_ERR		(SE_OK-10)
#define SE_ERROR_CGI_CAPTURE_STRING		(SE_OK-11)



//xml 相关定义
#define SE_XML_NODE_ROOT		"snmp_conf"
#define SE_XML_FILE_INFO		"file_info"
#define SE_XML_SYS_VERSION		"version"
#define SE_XML_SYS_THREADSWITCH		"thread_switch"
#define SE_XML_SYS_LOGSWITCH	"log_switch"
#define SE_XML_SYS_CACHETIME    "cache_time"

#define SE_XML_NODE_SYSINFO		"snmp_sys_info"
#define SE_XML_SYSINFO_NAME		"sys_name"
#define SE_XML_SYSINFO_DSCR		"sys_description"
#define SE_XML_SYSINFO_OID		"sys_oid"
#define SE_XML_SYSINFO_APORT	"sys_agent_port"
#define SE_XML_SYSINFO_TPORT	"sys_trap_port"
#define SE_XML_V1_STATUS		"snmp_v1_status"
#define SE_XML_V2C_STATUS		"snmp_v2c_status"
#define SE_XML_V3_STATUS		"snmp_v3_status"

#define SE_XML_NODE_COMM_ARRAY		"snmp_community_array"
#define SE_XML_NODE_COMMUNITY	"snmp_community"
#define SE_XML_COMM_KEY			"comm_key"
#define SE_XML_COMM_IP			"comm_ip"
#define SE_XML_COMM_MASK		"comm_mask"
#define SE_XML_COMM_ACCESS		"comm_access_mode"
#define SE_XML_COMM_STATUS		"comm_status"

#define SE_XML_NODE_V3USER_ARRAY	"snmp_v3user_array"
#define SE_XML_NODE_V3USER		"snmp_v3user"
#define SE_XML_V3USER_NAME		"v3user_name"
#define SE_XML_V3USER_ACCESS	"v3user_access_mode"
#define SE_XML_V3USER_AUTHPRO	"v3user_auth_protocal"
#define SE_XML_V3USER_AUTHPW	"v3user_auth_passwd"
#define SE_XML_V3USER_PRIVPRO	"v3user_priv_protocal"
#define SE_XML_V3USER_PRIVPW	"v3user_priv_passwd"
#define SE_XML_V3USER_STATUS	"v3user_status"

#define SE_XML_NODE_RECV_ARRAY	"snmp_trap_receiver_array"
#define SE_XML_NODE_RECV		"snmp_trap_receiver"
#define SE_XML_TRAP_INDEX		"trap_index"
#define SE_XML_TRAP_NAME		"trap_name"
#define SE_XML_TRAP_IP			"trap_ip"
#define SE_XML_TRAP_PORTNO		"trap_portno"
#define SE_XML_TRAP_STATUS		"trap_status"


//将192.169.1.1格式的地址转化成int型的地址
#define INET_ATON(ipaddr,addr_str)	\
		{\
			unsigned int a1,a2,a3,a4;\
			int ret;\
			ret = sscanf(addr_str,"%u.%u.%u.%u",&a1,&a2,&a3,&a4);\
			if( ret == 4 ){\
				ipaddr = a1*256*256*256+a2*256*256+a3*256+a4;\
			}else{\
				ipaddr=0;\
			}\
		}
//将int 32的值转化成ip地址字符串
#define INET_NTOA(ip_int,addr_str)\
		{\
			unsigned int a1,a2,a3,a4;\
			unsigned int ip_uint = (unsigned int)ip_int;\
			a1 = (ip_uint&0xff000000)>>24;\
			a2 = (ip_uint&0x00ff0000)>>16;\
			a3 = (ip_uint&0x0000ff00)>>8;\
			a4 = (ip_uint&0x000000ff);\
			sprintf( addr_str, "%d.%d.%d.%d", a1,a2,a3,a4 );\
		}




//提供给上层调用的api

int snmp_conf_init();
int snmp_conf_save();
int start_snmp_service();
int restart_snmp_service();
int stop_snmp_service();
int snmp_conf_release();



int get_sysinfo( STSNMPSysInfo *pstSnmpSysInfo );
int set_sysinfo( STSNMPSysInfo *pstSnmpSysInfo );

int get_community_num();
int add_community( STCommunity *pstCommunity );
int get_community( STCommunity *pstCommunity, int index );
int modify_community( STCommunity *pstCommunity, int index );
int del_community( int index );


int get_v3user_num();
int add_v3user( STSNMPV3User *pstV3User );
int modify_v3user( STSNMPV3User *pstV3User, int index );
int get_v3user( STSNMPV3User *pstV3User, int index );
int del_v3user( int index );


int get_trap_receiver_num();
int add_trap_receiver( STSNMPTrapReceiver *pstTrapReceiver );
int modify_trap_receiver( STSNMPTrapReceiver *pstTrapReceiver, int index );
int get_trap_receiver( STSNMPTrapReceiver *pstTrapReceiver, int index );
int del_trap_receiver( int index );
//add 2009-3-27
int set_trap_ip_addr(char * ip_addr);

//add datee 2008-12-25
char * get_sysOID();
char * get_sysOID_OEM_compatible();
char* get_token_snmp(char *str_tok);

extern int snmp_xml_modify_for_mib(STSNMPTrapReceiver *pstTrapReceiver,int index);
extern int snmp_xml_read_for_mib(STSNMPTrapReceiver *pstTrapReceiver,int * trap_num);
int CHECK_INPUT_DIGIT(char * src);

#endif

