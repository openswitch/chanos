#ifndef __COMMON_QOSACL_H__
#define __COMMON_QOSACL_H__

/*NPD Table Structure*/

enum
{
	QOS_FALSE = 0,
	QOS_TRUE = 1
};

/*
*	QOS-profile Data Structure.
*/

#define NPD_QOS_INC(x)  (x)++
#define NPD_QOS_DEC(x)  ( (x)>0?(x)--:0 ) 

#define	QOS_PROFILE_DONOT_CARE_E    0xffff

typedef enum
{
	QOS_DP_DONOT_CARE_E,
	QOS_DP_GREEN_E = 1 ,
	QOS_DP_YELLOW_E ,
	QOS_DP_RED_E
}QOS_DP_LEVEL_E;

typedef struct
{
	int 					  index;
    QOS_DP_LEVEL_E            dropPrecedence;
    unsigned int              userPriority;
    unsigned int              trafficClass;
    unsigned int              dscp;
    unsigned int              exp;  
	unsigned int              swPortNum;
}QOS_PROFILE_STC;


enum
{
	VOICE_VLAN_DEFUALUT = 0,
	VOICE_VLAN_ID,
	VOICE_VLAN_DOT1P,
	VOICE_VLAN_UNTAGGED,
	VOICE_VLAN_NONE
};

typedef struct
{
    unsigned int			profileIndex;
}QOS_DSCP_PROFILE_MAP_STC;


typedef struct
{
    unsigned int			profileIndex;
}QOS_UP_PROFILE_MAP_STC;

/* QOS-policy map Data Structure. */
typedef enum 
{
	KEEP_E = 0,
	DISABLE_E ,	
	ENABLE_E, 
	INVALID_E
}QOS_NORMAL_ENABLE_E;

enum
{
	PORT_MODE_DEFAULT,
	PORT_MODE_SP,
	PORT_MODE_WRR,
	PORT_MODE_HYBRID
};

enum
{
	DROP_MODE_TAIL,
	DROP_MODE_RED,
	DROP_MODE_WRED
};

typedef struct
{
	int 							index;
    int                             domain;
	QOS_NORMAL_ENABLE_E      		modifyUp;
	QOS_NORMAL_ENABLE_E      		modifyDscp;
	unsigned int	         		remapDscp;
	unsigned int                    dscpMapdscp[MAX_DSCP_DSCP_NUM];
	QOS_DSCP_PROFILE_MAP_STC        dscpMapQosProfile[MAX_DSCP_PROFILE_NUM];
	QOS_UP_PROFILE_MAP_STC          upMapQosProfile[MAX_UP_PROFILE_NUM];
	unsigned int					eth_count;
	npd_pbmp_t                      eth_mbr;
}QOS_PORT_POLICY_MAP_ATTRIBUTE_STC;

/*
out profile action
*/
typedef enum
{
	OUT_PROFILE_KEEP_E = 0 ,
	OUT_PROFILE_DROP ,
	OUT_PROFILE_REMAP_ENTRY,
	OUT_PROFILE_REMAP_TABLE
}QOS_PROFILE_OUT_PROFILE_ACTION_E;

typedef enum
{
    POLICER_COLOR_BLIND_E,
    POLICER_COLOR_AWARE_E
} QOS_POLICER_COLOR_MODE_ENT;

typedef enum
{
	POLICER_TB_STRICT_E,
	POLICER_TB_LOOSE_E
}QOS_METER_MODE_E;

typedef enum
{
	POLICER_PACKET_SIZE_TUNNEL_PASSENGER_E, 
    POLICER_PACKET_SIZE_L3_ONLY_E, 
    POLICER_PACKET_SIZE_L2_INCLUDE_E,
    POLICER_PACKET_SIZE_L1_INCLUDE_E
}QOS_POLICING_MODE_E;

typedef enum 
{
	POLICER_MRU_1536_E,
	POLICER_MRU_2K_E,
	POLICER_MRU_10K_E
}QOS_POLICER_MRU_E;


typedef struct _policer_color_cmd
{
    QOS_PROFILE_OUT_PROFILE_ACTION_E    cmd;
    unsigned int                        qosProfileID;    
    unsigned int                        transmite_flag;
}policer_color_cmd_t;


typedef struct{
	int                                 index;	
	unsigned int                        policerEnable;    
	QOS_POLICER_COLOR_MODE_ENT          meterColorMode;
	unsigned int					    cir;
	unsigned int					    cbs;
	unsigned int                        pir;
	unsigned int                        pbs;
    unsigned int                        counterEnable;
	unsigned int                        counterSetIndex;
	QOS_PROFILE_OUT_PROFILE_ACTION_E    cmd;
    QOS_NORMAL_ENABLE_E     			modifyDscp;
	QOS_NORMAL_ENABLE_E     			modifyUp;
    policer_color_cmd_t                    gPktParam;  
    policer_color_cmd_t                    yPktParam; 
    policer_color_cmd_t                    rPktParam; 
	unsigned int                           swPortNum;
    int direction;
}QOS_POLICER_STC;


typedef struct{	
  int              index;
  unsigned long    outOfProfileBytesCnt;
  unsigned long    inProfileBytesCnt;
  unsigned int     swPortNum;
}QOS_COUNTER_STC;


/**************
   queue scheduler
**************************/
typedef enum{
	QOS_PORT_TX_WRR_ARB_GROUP0_E = 0,
	QOS_PORT_TX_WRR_ARB_GROUP1_E,	
	QOS_PORT_TX_SP_ARB_GROUP_E,
	QOS_PORT_TX_WRR_ARB_E,
	QOS_PORT_TX_DEFAULT_WRR_ARB_E,
	QOS_PORT_TX_SP_WRR_ARB_E
}QOS_QUEUE_ALGORITHEM_FLAG_E;

typedef struct{
	unsigned int	groupFlag;	
	unsigned int	weight;
}QOS_WRR_TX_WEIGHT_E;
/***********
	traffic shape 
***************/

typedef struct{
	unsigned int     queueEnable;
	unsigned long 	 Maxrate;
	unsigned int 	 burstSize;
	unsigned int	 kmstate;
}QOS_SHAPER_QUEUE;

typedef struct {
	unsigned int poMapId;	
}QOS_PORT_POMAP_CFG_STC;

typedef struct {
	unsigned int	ingress_enable;
	unsigned int	egress_enable;
}QOS_PORT_ACL_SWITCH;


typedef struct {
	unsigned char ingressEnable;
	unsigned char egressEnable;
	unsigned int ingressRuleGrpId;
	unsigned int egressRuleGrpId;
}QOS_PORT_RULE_GRP_CFG_STC;

typedef struct {
	unsigned int		portEnable;
	unsigned long 		Maxrate;
	unsigned int  		burstSize;
	unsigned int		kmstate;
	QOS_SHAPER_QUEUE    queue[MAX_COS_QUEUE_NUM];	
}QOS_PORT_TC_CFG_STC;

typedef struct {
	int queue_type;
	QOS_WRR_TX_WEIGHT_E queue[MAX_COS_QUEUE_NUM];
    int queue_drop_mode;
    int queue_wred_weight[MAX_COS_QUEUE_NUM];
}QOS_PORT_COS_CFG_STC;

typedef struct {
	int inServicePolicyID;
}QOS_PORT_CLASS_POLICY_SERVICE;


enum{
    QOS_TRUST_PORT,
    QOS_TRUST_L2,
    QOS_TRUST_L3,
    QOS_TRUST_L2L3
};

typedef struct{
	unsigned int              ifIndex;
	QOS_PORT_POMAP_CFG_STC    poMapCfg;
	QOS_PORT_TC_CFG_STC       tcCfg;
	QOS_PORT_COS_CFG_STC      cosCfg;
    unsigned int              qosProfileIndex;
    int                       egressRemark;
    int                       ingressRemark;
    int                       trust;
}QOS_PORT_CFG_STC;

typedef struct
{
	int                     qos_mode;
	QOS_METER_MODE_E		meter;
	QOS_POLICING_MODE_E 	policing;
	QOS_POLICER_MRU_E		mru;
}QOS_GLOBAL_PARM_STC;


typedef struct cpu_flow_control_s
{
	char protocol[64];
	unsigned int bandwith;
	int priority;
}cpu_flow_control_t;


#define SYSTEM_QOS_POLICY          1

/*
 *
 * ACL Rule Main Data Structure.
 *
 */
#define NPD_TIME_RANGE_SIZE	32

typedef enum {	
	ACL_DIRECTION_INGRESS_E = 0, 
	ACL_DIRECTION_EGRESS_E, 
	ACL_DIRECTION_TWOWAY_E,
	ACL_DIRECTION_NONE_DISABLE_E
}ACL_DIRECTION_E;


enum L4PORT_RANGE_OP
{
    PORT_RANGE = 0,
    LITTLE_THAN,
    GREAT_THAN,
    NOT_EQUAL
};

enum L4_PORT_RANGE_OFFSET
{
    OPERATOR_GT_OFFSET = 0,
    OPERATOR_LT_OFFSET = 4,
    OPERATOR_NEQ_OFFSET = 8,
    PROTOCOL_OFFSET = 12
};

enum L4_PORT_RANGE_IP_PROTOCOL
{
    TCP_PROTOCOL = 6,
    UDP_PROTOCOL = 17
};

enum service_show_flag
{
    ACL_SERVICE,
    OTHER_SERVICE
};


struct vlan_based_acl_info_s
{
    int vlanId;
    int entryId;
    int phase;
    int doublephase;
    unsigned char unit;
    struct list_head list;
};

struct l4port_range_s
{
    int index;
    unsigned int l4port;
    unsigned int operation;
    unsigned int protocol;
    int bind_count;
};

#define ACL_GROUP_MAX_RULES 128
#define ACL_GROUP_DESP_LENTH 129
struct acl_group_stc
{
    char name[32];
    unsigned int group_index;
    unsigned int acl_index[ACL_GROUP_MAX_RULES];
    unsigned int is_deployed;
    npd_pbmp_t   portbmp;
    npd_vbmp_t   vlanbmp;
    int          dir_type;
    char desp[ACL_GROUP_DESP_LENTH];
};


#define NPD_ACL_RULE_SHOWRUN_CFG_SIZE (3000*1024)

#define MAX_GROUP_NUM 1024

#define MAX_ACL_RULE_NUMBER	1000

/* SG definition*/
#define SG_DEFAULT_DENY  "SG_DEFAULT_DENY"
#define SG_DEFAULT_PERMIT "SG_DEFAULT"

#define SG_MAX_STATIC_INDEX_NUM      256
#define SG_STATIC_INDEX_START_ID     1
#define SG_MAX_DYNAMIC_INDEX_NUM     256
#define SG_DYNAMIC_INDEX_START_ID    (SG_MAX_STATIC_INDEX_NUM+1)
#define SG_DEFAULT_PERMIT_ID  (2*SG_MAX_STATIC_INDEX_NUM+2*SG_MAX_DYNAMIC_INDEX_NUM+1)
#define SG_DEFAULT_DENY_ID    (SG_DEFAULT_PERMIT_ID+1)
#define SG_VLAN_DROP_ENTRY_OFFSET    (SG_MAX_STATIC_INDEX_NUM + SG_MAX_DYNAMIC_INDEX_NUM)
#define NPD_SG_MAX_TABLE_SIZE   (SG_MAX_STATIC_INDEX_NUM + SG_MAX_DYNAMIC_INDEX_NUM)

#define IN_TIME_RANGE             1
#define OUT_TIME_RANGE            0

#define TIME_RANGE_ABS_TIME        (1<<0)
#define TIME_RANGE_WEEKEND         (1<<1)
#define TIME_RANGE_WORKDAY         (1<<2)
#define TIME_RANGE_EVERYDAY        (1<<3)

typedef struct class_map_index_s
{
    char map_name[32];
    int index;
    int is_acl;
    int is_binded;
    int is_deployed;
}class_map_index_t;

typedef struct policy_map_index_s
{
    char map_name[32];
    int index;
    int class_map_index; /*bind with a class map*/
    int is_deployed;
    char acl_group[32];
}policy_map_index_t;

struct service_policy_s
{
    char policy_map_name[32];
    int policy_index;
    int service_policy_index;
    int dir_type;  
    npd_pbmp_t group;
    npd_vbmp_t vlanbmp;
    int reserved;
    int time_range_set; /*if in time range is TRUE, else FALSE*/
};
typedef struct service_policy_route_s
{
	char policy_map_name[32];
	unsigned int nexthopv4;
	unsigned int nexthopv6[4];
	unsigned int change_coount;
}service_policy_route_t;

struct class_map_vty
{
    char map_name[32];
    int pref;
};

typedef struct class_map_rule_s
{
	char lk_phase;
    char map_name[32];
    int index; /*same as class_map_index->index*/
    char cmd_name[32];
    char cmd_arg[16];
    char cmd_mask[16];
}class_map_rule_t;

typedef struct preset_value_s
{
    char *preset_value; 
    char *preset_value_name;
}preset_value_t;

typedef struct preset_arg_s
{
    preset_value_t** value;
    int preset_value_num;
}preset_arg_t;

typedef struct rule_cmd_s
{
    char *cmd_name;
    preset_arg_t *arg;
    int  hw_auto_recog;
    int  layer2h_offset;
    int  layer3h_offset;
    int  layer4h_offset;
    int  length;
    void (*func_apply)(int, void*);
    void (*func_free)(int, void*);
	unsigned char (*func_phase)(int, void*);
    void (*func_compile)(char *, char[16], char *);
}rule_cmd_t;

typedef struct policy_map_rule_s
{
	char lk_phase;
    char map_name[32];
    int index;
    char cmd_name[32];
    char cmd_arg[128];
    char cmd_mask[128];
}policy_map_rule_t;

typedef struct policy_map_cmd_s
{
    char *cmd_name;
    void (*func_apply)(void *);
    void (*func_free)(void *);
    void (*func_compile)(void *, int argc, char *argv[]);
}policy_map_cmd_t;

typedef struct policy_map_rule_conflict_s
{
    struct list_head list;
    struct policy_map_cmd_s *cmd;
}policy_map_rule_conflict_t;

#define INGRESS_SERVICE_POLICY
#define EGRESS_SERVICE_POLICY

typedef struct intf_group_s
{
    npd_pbmp_t ports;
    npd_vbmp_t vlans;
    unsigned char trunks[CHASSIS_TRUNK_RANGE_MAX+1/8];
} intf_group_t;



#endif

