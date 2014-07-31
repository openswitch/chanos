
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
   Class map function.
   Copyright (C) 2011, Autelan Co., Ltd.

*/
#ifdef HAVE_QOS
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_classmap.h"


int first_free_entryId[2] = {RULE_ENTRY_FIRST_ID, RULE_ENTRY_FIRST_ID};
struct service_index_s service_index_alloc = { 0 };


#define SHOW_SERVICE_SIZE       (128 * 1024)

#define L4_PORT_RANGE_OP_TOTOAL 3
#define TCP_CMP_EXHAUST         0
#define TCP_CMP_NOT_EXHAUST     1
#define UDP_CMP_EXHAUST         0
#define UDP_CMP_NOT_EXHAUST     1

unsigned int npd_tcp_cmp_used = 0;
unsigned int npd_udp_cmp_used = 0;

int npd_vlan_acl_vid_conflict_check(char* policy_name, unsigned int netif_index);
int npd_is_time_in_time_range(char *map_name, unsigned int *op_ret);
int npd_l4port_range_del_by_cmd_arg(const char *cmd_arg);
int npd_is_acl_associate_time_range_info(char* map_name, char* time_range_name);

#define	diffserv_check_golbal_start() \
	{\
		if(1)\
		{

#define	diffserv_check_golbal_end() \
		}\
	}


extern int dbtable_hash_head_key
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
);

extern int dbtable_hash_next_key
(
    hash_table_index_t *hash,
    void * data,
    void * outdata,
    unsigned int (*filter)(void *,void *)
);

rule_cmd_t dst_mac_rule =
{
    .cmd_name = "destination-address mac"
};

rule_cmd_t src_mac_rule =
{
    .cmd_name = "source-address mac"
};

preset_value_t dot1q_tpid =
{
    .preset_value = "33024",	/* 0x8100 */
    .preset_value_name = "DOT1Q"
};

preset_value_t vman_tpid =
{
    .preset_value = "34984",	/* 0x88a8 */
    .preset_value_name = "VMAN"
};

preset_value_t dot1qinq_tpid =
{
    .preset_value = "37120",	/* 0x9100 */
    .preset_value_name = "DOT1QINQ"
};


preset_value_t* tpid_preset[3]=
{
    &dot1q_tpid,
    &vman_tpid,
    &dot1qinq_tpid
};


preset_arg_t tpid_preset_arg =
{
    .value = tpid_preset,
    .preset_value_num = 3
};

rule_cmd_t inner_tpid_rule =
{
    .cmd_name = "inner-tpid",
    .arg = &tpid_preset_arg
};

rule_cmd_t outer_tpid_rule =
{
    .cmd_name = "outer-tpid",
    .arg = &tpid_preset_arg
};

rule_cmd_t vlan_format_rule = 
{
    .cmd_name = "vlan-tag-num",
};

rule_cmd_t cos_rule =
{
    .cmd_name = "cos"
};

rule_cmd_t inner_vlan_rule =
{
    .cmd_name = "inner-vlan"
};

rule_cmd_t outer_vlan_rule =
{
    .cmd_name = "outer-vlan"
};

rule_cmd_t double_q_rule =
{
    .cmd_name = "double tag"
};

rule_cmd_t single_q_rule =
{
    .cmd_name = "single tag"
};


preset_value_t pppoe_ethtype =
{
    .preset_value = "34916",	/* 0x8864 */
    .preset_value_name = "pppoe"
};

preset_value_t pppoe_c_ethtype =
{
    .preset_value = "34915",	/* 0x8863 */
    .preset_value_name = "pppoe-control"
};

preset_value_t eap_ethtype =
{
    .preset_value = "34958",	/* 0x888e */
    .preset_value_name = "eap"
};

preset_value_t appletalk_ethtype =
{
    .preset_value = "32923",	/* 0x809b */
    .preset_value_name = "appletalk"
};

preset_value_t arp_ethtype =
{
    .preset_value = "2054",	/* 0x0806 */
    .preset_value_name = "arp"
};

preset_value_t ip_ethtype =
{
    .preset_value = "2048",	/* 0x0800 */
    .preset_value_name = "ipv4"
};

preset_value_t ipv6_ethtype =
{
    .preset_value = "34525",	/* 0x86dd */
    .preset_value_name = "ipv6"
};

preset_value_t ipx_ethtype =
{
    .preset_value = "33079",	/* 0x8137 */
    .preset_value_name = "ipx"
};

preset_value_t novell_ethtype =
{
    .preset_value = "33080",	/* 0x8138 */
    .preset_value_name = "novell"
};
	
preset_value_t rarp_ethtype =
{
    .preset_value = "32821",	/* 0x8035 */
    .preset_value_name = "rarp"
};

preset_value_t* ethertype_preset[9] =
{
    &arp_ethtype,
    &ip_ethtype,
    &ipv6_ethtype,
    &ipx_ethtype,
    &novell_ethtype,
    &pppoe_ethtype,    
    &eap_ethtype,
    &appletalk_ethtype,
    &rarp_ethtype
};

preset_arg_t ethertype_preset_arg =
{
    .value = ethertype_preset,
    .preset_value_num = 9
};

rule_cmd_t ether_type_rule =
{
    .cmd_name = "ethertype",
    .arg = &ethertype_preset_arg
};

rule_cmd_t ip_tos_rule =
{
    .cmd_name = "ip tos"
};

rule_cmd_t ip_prece_rule =
{
    .cmd_name = "ip precedence"
};

rule_cmd_t ip_routed_rule =
{
    .cmd_name = "ip routed"
};



/*
The RECOMMENDED values of the AF codepoints are as follows: AF11 = '
   001010', AF12 = '001100', AF13 = '001110', AF21 = '010010', AF22 = '
   010100', AF23 = '010110', AF31 = '011010', AF32 = '011100', AF33 = '
   011110', AF41 = '100010', AF42 = '100100', and AF43 = '100110'.  The
   table below summarizes the recommended AF codepoint values.
   101110 = DSCP 46 EF
   */
preset_value_t dscp_af11 =
{
    .preset_value = "10",
    .preset_value_name = "af11"
};

preset_value_t dscp_af12 =
{
    .preset_value = "12",
    .preset_value_name = "af12"
};

preset_value_t dscp_af13 =
{
    .preset_value = "14",
    .preset_value_name = "af13"
};

preset_value_t dscp_af21 =
{
    .preset_value = "18",
    .preset_value_name = "af21"
};

preset_value_t dscp_af22 =
{
    .preset_value = "20",
    .preset_value_name = "af22"
};

preset_value_t dscp_af23 =
{
    .preset_value = "22",
    .preset_value_name = "af23"
};

preset_value_t dscp_af31 =
{
    .preset_value = "26",
    .preset_value_name = "af31"
};

preset_value_t dscp_af32 =
{
    .preset_value = "28",
    .preset_value_name = "af32"
};

preset_value_t dscp_af33 =
{
    .preset_value = "30",
    .preset_value_name = "af33"
};

preset_value_t dscp_af41 =
{
    .preset_value = "34",
    .preset_value_name = "af41"
};

preset_value_t dscp_af42 =
{
    .preset_value = "36",
    .preset_value_name = "af42"
};

preset_value_t dscp_af43 =
{
    .preset_value = "38",
    .preset_value_name = "af43"
};

preset_value_t dscp_ef =
{
    .preset_value = "46",
    .preset_value_name = "ef"
};

preset_value_t dscp_be =
{
    .preset_value = "0",
    .preset_value_name = "be"
};

preset_value_t dscp_cs0 =
{
    .preset_value = "0",
    .preset_value_name = "cs0"
};

preset_value_t dscp_cs1 =
{
    .preset_value = "8",
    .preset_value_name = "cs1"
};

preset_value_t dscp_cs2 =
{
    .preset_value = "16",
    .preset_value_name = "cs2"
};

preset_value_t dscp_cs3 =
{
    .preset_value = "24",
    .preset_value_name = "cs3"
};

preset_value_t dscp_cs4 =
{
    .preset_value = "32",
    .preset_value_name = "cs4"
};

preset_value_t dscp_cs5 =
{
    .preset_value = "40",
    .preset_value_name = "cs5"
};

preset_value_t dscp_cs6 =
{
    .preset_value = "48",
    .preset_value_name = "cs6"
};

preset_value_t dscp_cs7 =
{
    .preset_value = "56",
    .preset_value_name = "cs7"
};

preset_value_t* dscp_preset[22] =
{
	&dscp_af11,
    &dscp_af12,
    &dscp_af13,
    &dscp_af21,
    &dscp_af22,
    &dscp_af23,
    &dscp_af31,
    &dscp_af32,
    &dscp_af33,
    &dscp_af41,
    &dscp_af42,
    &dscp_af43,
    &dscp_ef,
    &dscp_be,
    &dscp_cs0,
    &dscp_cs1,
    &dscp_cs2,
    &dscp_cs3,
    &dscp_cs4,
    &dscp_cs5,
    &dscp_cs6,
    &dscp_cs7
};

preset_arg_t dscp_preset_arg =
{
    .value = dscp_preset,
    .preset_value_num = 22
};

rule_cmd_t ip_dscp_rule =
{
    .cmd_name = "ip dscp",
    .arg = &dscp_preset_arg
};

preset_value_t ip_proto_icmp =
{
    .preset_value = "1",
    .preset_value_name = "icmp"
};

preset_value_t ip_proto_igmp =
{
    .preset_value = "2",
    .preset_value_name = "igmp"
};

preset_value_t ip_proto_tcp =
{
    .preset_value = "6",
    .preset_value_name = "tcp"
};

preset_value_t ip_proto_udp =
{
    .preset_value = "17",
    .preset_value_name = "udp"
};

preset_value_t* ip_proto_preset[4] =
{
    &ip_proto_icmp, 
	&ip_proto_igmp,
	&ip_proto_tcp,
	&ip_proto_udp
};

preset_arg_t ip_proto_preset_arg =
{
    .value = ip_proto_preset,
    .preset_value_num = 4 
};

rule_cmd_t ip_protocol_rule =
{
    .cmd_name = "protocol",
    .arg = &ip_proto_preset_arg
};

rule_cmd_t ip_dstip_rule =
{
    .cmd_name = "dstip"
};

rule_cmd_t ip_dstip6_rule =
{
    .cmd_name = "dstip6"
};

rule_cmd_t ip_srcip_rule =
{
    .cmd_name = "srcip"
};

rule_cmd_t ip_srcip6_rule =
{
    .cmd_name = "srcip6"
};

preset_value_t ip_port_echo =
{
    .preset_value = "7",
    .preset_value_name = "echo"
};

preset_value_t ip_port_ftpdata =
{
    .preset_value = "20",
    .preset_value_name = "ftpdata"
};

preset_value_t ip_port_ftp =
{
    .preset_value = "21",
    .preset_value_name = "ftp"
};

preset_value_t ip_port_telnet =
{
    .preset_value = "23",
    .preset_value_name = "telnet"
};

preset_value_t ip_port_smtp =
{
    .preset_value = "25",
    .preset_value_name = "smtp"
};

preset_value_t ip_port_domain =
{
    .preset_value = "53",
    .preset_value_name = "domain"
};

preset_value_t ip_port_tftp =
{
    .preset_value = "69",
    .preset_value_name = "tftp"
};

preset_value_t ip_port_http =
{
    .preset_value = "80",
    .preset_value_name = "http"
};

preset_value_t ip_port_www =
{
    .preset_value = "80",
    .preset_value_name = "www"
};

preset_value_t ip_port_snmp =
{
    .preset_value = "161",
    .preset_value_name = "snmp"
};

preset_value_t* ip_port_preset[10] =
{
    &ip_port_echo,
	&ip_port_ftp,
	&ip_port_ftpdata,
	&ip_port_telnet,
	&ip_port_smtp,
	&ip_port_domain,
	&ip_port_tftp,
	&ip_port_http,
	&ip_port_www,
    &ip_port_snmp
};

preset_arg_t ip_port_preset_arg =
{
    .value = ip_port_preset,
    .preset_value_num = 10 
};

rule_cmd_t ip_dstl4port_rule =
{
    .cmd_name = "dstl4port",
    .arg = &ip_port_preset_arg
};

rule_cmd_t ip_srcl4port_rule =
{
    .cmd_name = "srcl4port",
    .arg = &ip_port_preset_arg
};

preset_value_t tcp_fin =
{
	.preset_value = "0x1",
	.preset_value_name = "fin"
};

preset_value_t tcp_syn =
{
	.preset_value = "0x2",
	.preset_value_name = "syn"
};

preset_value_t tcp_rst =
{
	.preset_value = "0x4",
	.preset_value_name = "rst"
};

preset_value_t tcp_psh =
{
	.preset_value = "0x8",
	.preset_value_name = "psh"
};

preset_value_t tcp_ack =
{
	.preset_value = "0x10",
	.preset_value_name = "ack"
};

preset_value_t tcp_urg =
{
	.preset_value = "0x20",
	.preset_value_name = "urg"
};

preset_value_t * tcp_flag_pre_parameter[6] =
{
	&tcp_fin,
	&tcp_syn,
	&tcp_rst,
	&tcp_psh,
	&tcp_ack,
	&tcp_urg
};

preset_arg_t tcp_flag_pre_arg =
{
    .value = tcp_flag_pre_parameter,
    .preset_value_num = 10 
};

rule_cmd_t ip_tcp_flag_rule =
{
	.cmd_name = "tcp-flag",
	.arg = &tcp_flag_pre_arg,
};

rule_cmd_t icmp_type_rule =
{
	.cmd_name = "icmp-type",
};

rule_cmd_t icmp_code_rule =
{
	.cmd_name = "icmp-code",
};

/*policy map set usually rule*/
rule_cmd_t drop_rule =
{
    .cmd_name = "drop"
};

rule_cmd_t qos_profile_rule = 
{
    .cmd_name = "qos-profile"
};

rule_cmd_t policer_rule =
{
    .cmd_name = "policer"
};

rule_cmd_t trap_rule = 
{
	.cmd_name = "trap"
};

rule_cmd_t copytocpu_rule = 
{
	.cmd_name = "copy-to-cpu"
};

rule_cmd_t color_rule =
{
    .cmd_name = "conform-color"
};

rule_cmd_t assign_queue_rule =
{
    .cmd_name = "assign-queue"
};

rule_cmd_t mark_cos_rule =
{
    .cmd_name = "mark-cos"
};

rule_cmd_t mark_ip_precedence_rule =
{
    .cmd_name = "mark ip-precedence"
};

rule_cmd_t mark_ip_dscp_rule =
{
    .cmd_name = "mark ip-dscp",
	.arg = &dscp_preset_arg
};

rule_cmd_t color_aware_rule =
{
    .cmd_name = "color aware"
};

rule_cmd_t mirror_port_rule =
{
    .cmd_name = "mirror"
};

rule_cmd_t mirror_profile_rule =
{
    .cmd_name = "mirror-profile"
};

rule_cmd_t redirect_port_rule =
{
    .cmd_name = "redirect"
};

rule_cmd_t redirect_slot_rule = 
{
	.cmd_name = "redirect-slot"
};

rule_cmd_t policy_route_rule =
{
    .cmd_name = "policy-route"
};

rule_cmd_t outervlan_add =
{
    .cmd_name = "policy-vlanadd"
};

rule_cmd_t policer_simple_rule =
{
    .cmd_name = "police-simple"
};

rule_cmd_t policer_advance_rule =
{
    .cmd_name = "policer"
};

rule_cmd_t yellow_action_rule =
{
    .cmd_name = "conform-action"
};

rule_cmd_t red_action_rule =
{
    .cmd_name = "violate-action"
};

rule_cmd_t red_action2_rule =
{
    .cmd_name = "exceed-action"
};

rule_cmd_t remark_rule = 
{
    .cmd_name = "remark"
};

rule_cmd_t counter_rule =
{
    .cmd_name = "account"
};

/*time range rule cmd define*/
rule_cmd_t time_range_rule =
{
    .cmd_name = "time-range"
};
/*end*/
rule_cmd_t l4port_range_src_rule =
{
    .cmd_name = "l4port_range_src"
};

rule_cmd_t l4port_range_dst_rule = 
{
    .cmd_name = "l4port_range_dst"
};

rule_cmd_t* classmap_match_restrict[] = 
{
	&dst_mac_rule,
	&src_mac_rule,
	&inner_tpid_rule,
	&outer_tpid_rule,
	&vlan_format_rule,
	&cos_rule,
	&inner_vlan_rule,
	&outer_vlan_rule,
	&double_q_rule,
	&single_q_rule,
	&ether_type_rule,
	&ip_tos_rule,
	&ip_prece_rule,
	&ip_dscp_rule,
	&ip_protocol_rule,
	&ip_dstip_rule,
	&ip_dstip6_rule,
	&ip_srcip_rule,
	&ip_srcip6_rule,
	&ip_dstl4port_rule,
	&ip_srcl4port_rule,
	&ip_tcp_flag_rule,
	&icmp_type_rule,
	&icmp_code_rule,
	/*time range rule*/
    &time_range_rule,
    &l4port_range_src_rule,
    &l4port_range_dst_rule,
	&ip_routed_rule	
};

int classmap_match_restrict_num = 
    (sizeof(classmap_match_restrict)/sizeof(classmap_match_restrict[0]));

rule_cmd_t* policymap_match_restrict[] = 
{
	&drop_rule,
    &qos_profile_rule,
    &policer_rule,
    &trap_rule,
	&color_rule,
	&assign_queue_rule,
	&mark_cos_rule,
	&mark_ip_precedence_rule,
	&mark_ip_dscp_rule,
	&color_aware_rule,
	&mirror_port_rule,
	&mirror_profile_rule,
	&redirect_port_rule,
	&policy_route_rule,
	&outervlan_add,
	&policer_simple_rule,
	&policer_advance_rule,
	&yellow_action_rule,
	&red_action_rule,
	&red_action2_rule,
	&remark_rule,
	&counter_rule,
	&copytocpu_rule,
	&redirect_slot_rule,
};

int policymap_match_restrict_num 
     = (sizeof(policymap_match_restrict)/sizeof(policymap_match_restrict[0]));

rule_cmd_t**	class_map_match_vec;
int class_map_match_vec_rule_num;

rule_cmd_t**	policy_map_set_vec;
int policy_map_set_vec_rule_num;

/*hw operation*/
int (*qos_hw_rule_create)(int service_policy_index, int dir, npd_pbmp_t *ports, char lkphase);
int (*qos_hw_rule_add_intf)(int service_policy_index, int dir, unsigned int netif_index, char lkphase);
int (*qos_hw_rule_destroy)(int service_policy_index, int dir, npd_pbmp_t *ports);
int (*qos_hw_rule_del_intf)(int service_policy_index, int dir, unsigned int netif_index, char lkphase);
int (*qos_hw_rule_install_entry)(int service_policy_index, char lkphase);
int (*qos_hw_rule_vlan_create)(int service_policy_index, int dir, npd_vbmp_t *vlans, char lkphase);
int (*qos_hw_rule_add_vlan)(int service_policy_index, int dir, unsigned short vid, char lkphase);
int (*qos_hw_rule_del_vlan)(int service_policy_index, int dir, unsigned short vid, char lkphase);
	
void install_hw_rule_func(
    int (*hw_rule_create)(int, int, npd_pbmp_t *, char),
    int (*hw_rule_vlan_create)(int, int, npd_vbmp_t *, char),
    int (*hw_rule_add_intf)(int, int, unsigned int, char),
    int (*hw_rule_add_vlan)(int, int, unsigned short, char),
    int (*hw_rule_destroy)(int, int, npd_pbmp_t *),
    int (*hw_rule_del_intf)(int, int, unsigned int, char),
    int (*hw_rule_del_vlan)(int, int, unsigned short, char),
    int (*hw_rule_install_entry)(int, char)
    )
{
    qos_hw_rule_create = hw_rule_create;
    qos_hw_rule_vlan_create = hw_rule_vlan_create;
    qos_hw_rule_add_intf = hw_rule_add_intf;
    qos_hw_rule_add_vlan = hw_rule_add_vlan;
    qos_hw_rule_destroy = hw_rule_destroy;
    qos_hw_rule_del_intf = hw_rule_del_intf;
    qos_hw_rule_del_vlan = hw_rule_del_vlan;
	qos_hw_rule_install_entry = hw_rule_install_entry;
}


void install_rule_cmd_func(
    rule_cmd_t *rule,
    void (*func_apply)(int, void*),
    unsigned char (*func_phase)(int, void*),
    void (*func_compile)(char*, char[16], char *)
    )
{
    rule->func_apply = func_apply;
    rule->func_phase = func_phase;
    rule->func_compile = func_compile;
}

int (*qos_hw_l4port_range_alloc)(int port_range_index, unsigned int protocol);
int (*qos_hw_l4port_range_free)(int port_range_index, unsigned int protocol);

void install_hw_l4port_range_func(
    int (*hw_l4port_range_alloc)(int , unsigned int),
    int (*hw_l4port_range_free)(int , unsigned int)
    )
{
    qos_hw_l4port_range_alloc = hw_l4port_range_alloc;
    qos_hw_l4port_range_free = hw_l4port_range_free;
}

void install_class_map_match_vec(rule_cmd_t** match_vec, int num)
{
    class_map_match_vec = match_vec;
    class_map_match_vec_rule_num = num;  
}

void install_policy_map_set_vec(rule_cmd_t** match_vec, int num)
{
    policy_map_set_vec = match_vec;
    policy_map_set_vec_rule_num = num;
}


/* Lookup rule command from match list. */
rule_cmd_t* class_map_lookup_match(const char* name)
{
	unsigned int 	i = 0;
	rule_cmd_t*		rule = NULL;

	for (i = 0; i < class_map_match_vec_rule_num; i++)
	{
		if (NULL != class_map_match_vec[i])
		{
			rule = class_map_match_vec[i];
			if (strcmp(rule->cmd_name, name) == 0)
			{
				return rule;
			}
		}
	}

	return NULL;
}

/* Lookup rule command from set list. */
rule_cmd_t* policy_map_lookup_set(const char* name)
{
	unsigned int 	i = 0;
	rule_cmd_t*		rule = NULL;

	for (i = 0; i < policy_map_set_vec_rule_num; i++)
	{
		if (NULL != policy_map_set_vec[i])
		{
			rule = policy_map_set_vec[i];
			if (strcmp (rule->cmd_name, name) == 0)
			{
				return rule;
			}
		}
	}
	return NULL;
}

/* class map array index and hash index */
array_table_index_t* class_map_master;
hash_table_index_t* class_map_master_name;
/* class map rule hash index */
hash_table_index_t* class_map_match_rule;
/* policy map array index and hash index */
array_table_index_t* policy_map_index;
hash_table_index_t* policy_map_name;
/* policy map rule hash index */
hash_table_index_t* policy_map_set_rule;
/* service policy array index and hash index */
array_table_index_t* service_policy_index;
hash_table_index_t* service_policy_name;
hash_table_index_t *ser_policy_pb_index;/*HASH*/
hash_table_index_t *ser_policy_pb_hash;
array_table_index_t *port_group_array_index;
hash_table_index_t  *port_group_hash_index;

array_table_index_t* time_range_info_master;
hash_table_index_t* time_range_info_name;

array_table_index_t* l4port_range_master;
hash_table_index_t* l4port_range_name;

array_table_index_t* acl_group_index;
hash_table_index_t* acl_group_name;
unsigned int l4port_range_namehash_key(void *new)
{
    struct l4port_range_s *entry = (struct l4port_range_s*)new;

    return (entry->l4port % 256);
}

unsigned int l4port_range_namehash_cmp(void *new, void *old)
{
    struct l4port_range_s *entry1 = (struct l4port_range_s*)new;
    struct l4port_range_s *entry2 = (struct l4port_range_s*)old;
	
    return ((entry1->operation == entry2->operation)
        && (entry1->protocol == entry2->protocol));
}

unsigned int time_range_info_namehash_key(void *new)
{
    struct time_range_info_s *entry = (struct time_range_info_s*)new;

    return name_hash(entry->name, 256);
}

unsigned int time_range_info_namehash_cmp(void *new, void *old)
{
    struct time_range_info_s *entry1 = (struct time_range_info_s*)new;
    struct time_range_info_s *entry2 = (struct time_range_info_s*)old;
	
    return (0 == strcmp(entry1->name, entry2->name));
}

unsigned int acl_group_namehash_key(void *new)
{
    struct acl_group_stc *entry = (struct acl_group_stc*)new;

    return name_hash(entry->name, 256);
}

unsigned int acl_group_namehash_cmp(void *new, void *old)
{
    struct acl_group_stc *entry1 = (struct acl_group_stc*)new;
    struct acl_group_stc *entry2 = (struct acl_group_stc*)old;
	
    return (0 == strcmp(entry1->name, entry2->name));
}

unsigned int class_map_index_namehash_key(void *new)
{
    struct class_map_index_s *entry = (struct class_map_index_s*)new;

    return name_hash(entry->map_name, CMAP_MAX_RULE_NUM);
}

unsigned int class_map_index_namehash_cmp(void *new, void *old)
{
    struct class_map_index_s *entry1 = (struct class_map_index_s*)new;
    struct class_map_index_s *entry2 = (struct class_map_index_s*)old;
	
    return (0 == strcmp(entry1->map_name, entry2->map_name));
}

unsigned int class_map_rule_key(void *new)
{
    struct class_map_rule_s *entry = (struct class_map_rule_s*)new;

    return (entry->index % CMAP_MAX_RULE_NUM);
}

unsigned int class_map_rule_cmp(void *new, void *old)
{
	struct class_map_rule_s *entry1 = (struct class_map_rule_s*)new;
	struct class_map_rule_s *entry2 = (struct class_map_rule_s*)old;

	return (
		(0 == strcmp(entry1->map_name, entry2->map_name))
		&& (entry1->index == entry2->index)
		&& (0 == strcmp(entry1->cmd_name, entry2->cmd_name))
		&& (0 == memcmp(entry1->cmd_arg, entry2->cmd_arg, sizeof(entry1->cmd_arg)))
		&& (0 == memcmp(entry1->cmd_mask, entry2->cmd_mask, sizeof(entry1->cmd_mask)))
		);
}

unsigned int class_map_rule_simple_cmp(void *new, void *old)
{
	struct class_map_rule_s *entry1 = (struct class_map_rule_s*)new;
	struct class_map_rule_s *entry2 = (struct class_map_rule_s*)old;

	return (
		(0 == strcmp(entry1->map_name, entry2->map_name))
		&& (entry1->index == entry2->index)
		&& (0 == strcmp(entry1->cmd_name, entry2->cmd_name))
		);
}

unsigned int class_map_rule_filter(void *input, void *exist)
{
	struct class_map_rule_s *entry1 = (struct class_map_rule_s *)input;
	struct class_map_rule_s *entry2 = (struct class_map_rule_s *)exist;

	return(entry1->index == entry2->index);
}

unsigned int policy_map_index_namehash_key(void *new)
{
	struct policy_map_index_s *entry = (struct policy_map_index_s*)new;

	return name_hash(entry->map_name, CMAP_MAX_RULE_NUM);
}

unsigned int policy_map_index_namehash_cmp(void *new, void *old)
{
    struct policy_map_index_s *entry1 = (struct policy_map_index_s*)new;
    struct policy_map_index_s *entry2 = (struct policy_map_index_s*)old;
	
    return (0 == strcmp(entry1->map_name, entry2->map_name));
}

unsigned int policy_map_rule_key(void *new)
{
    struct policy_map_rule_s *entry = (struct policy_map_rule_s*)new;

    return (entry->index % CMAP_MAX_RULE_NUM);
}

unsigned int policy_map_rule_cmp(void *new, void *old)
{
	struct policy_map_rule_s *entry1 = (struct policy_map_rule_s*)new;
	struct policy_map_rule_s *entry2 = (struct policy_map_rule_s*)old;

	return (
        (0 == strcmp(entry1->map_name, entry2->map_name))
        && (entry1->index == entry2->index)
        && (0 == strcmp(entry1->cmd_name, entry2->cmd_name))
		&& (0 == memcmp(entry1->cmd_arg, entry2->cmd_arg, sizeof(entry1->cmd_arg)))
        && (0 == memcmp(entry1->cmd_mask, entry2->cmd_mask, sizeof(entry1->cmd_mask)))
        );
}

unsigned int policy_map_rule_simple_cmp(void *new, void *old)
{
	struct policy_map_rule_s *entry1 = (struct policy_map_rule_s*)new;
	struct policy_map_rule_s *entry2 = (struct policy_map_rule_s*)old;

	return (
        (0 == strcmp(entry1->map_name, entry2->map_name))
        && (entry1->index == entry2->index)
        && (0 == strcmp(entry1->cmd_name, entry2->cmd_name))
        );
}

unsigned int policy_map_rule_filter(void *input, void *exist)
{
    struct policy_map_rule_s *entry1 = (struct policy_map_rule_s *)input;
	struct policy_map_rule_s *entry2 = (struct policy_map_rule_s *)exist;

	return (entry1->index == entry2->index && (0 == strcmp(entry1->map_name, entry2->map_name)));
}

unsigned int service_policy_index_namehash_key(void *new)
{
	struct service_policy_s *entry = (struct service_policy_s*)new;

	return name_hash(entry->policy_map_name, CMAP_MAX_RULE_NUM);
}

unsigned int service_policy_index_namehash_cmp(void *new, void *old)
{
    struct service_policy_s *entry1 = (struct service_policy_s*)new;
    struct service_policy_s *entry2 = (struct service_policy_s*)old;
    return ((0 == strcmp(entry1->policy_map_name, entry2->policy_map_name))
             && (entry1->dir_type == entry2->dir_type));
}
#ifdef HAVE_ROUTE
unsigned int ser_policy_pb_name_hash(void *new)
{
	struct service_policy_route_s *entry = (struct service_policy_route_s*)new;

	return name_hash(entry->policy_map_name, CMAP_MAX_RULE_NUM);
}

unsigned int ser_policy_pb_ip_hash(void *new)
{
	unsigned int key;
	struct service_policy_route_s *entry = (struct service_policy_route_s*)new;

	key = jhash_1word(entry->nexthopv4, 0x35798642);
    key %= CMAP_MAX_RULE_NUM;
	return key;
}

unsigned int ser_policy_pb_cmp(void *new, void *old)
{
    struct service_policy_route_s *entry1 = (struct service_policy_route_s*)new;
    struct service_policy_route_s *entry2 = (struct service_policy_route_s*)old;
    return (0 == strcmp(entry1->policy_map_name, entry2->policy_map_name));
}
#endif
unsigned int port_grp_index_key(void *new)
{
	struct port_group_s *entry = (struct port_group_s*)new;

	return (entry->service_policy_index % 256);
}

unsigned int port_grp_index_cmp(void *new, void *old)
{
    struct port_group_s *entry1 = (struct port_group_s*)new;
    struct port_group_s *entry2 = (struct port_group_s*)old;
    return (entry1->service_policy_index == entry2->service_policy_index);
}
#ifndef HAVE_MEMORY_SHORT
int service_index_init
(
    int max_rule_num, 
    int rule_entry_max, 
    int (*nam_qos_set_entry)(int , void *, int, char, unsigned char)
)   
{
    service_index_alloc.nam_set_entry = nam_qos_set_entry;
    service_index_alloc.rule_entry_max = rule_entry_max;
    service_index_alloc.max_rule_num = max_rule_num;
    service_index_alloc.service_rule = (service_index_rule_t *)malloc(sizeof(service_index_rule_t) * max_rule_num);
    if(service_index_alloc.service_rule == NULL)
        return -1;
    else
    {
        memset(service_index_alloc.service_rule, 0, (sizeof(service_index_rule_t) * max_rule_num));
#if 0        
        for(m = 0; m < 2; m++)
        {
             for(j = 0; j < rule_entry_max; j++)
             {
                  for(k = 0; k < CLASSMAP_PHASE; k++)
                  {
                       for(i = 0; i < max_rule_num; i++)
                       {
                           service_index_alloc.service_rule[i].entry[j][k][m].entryId = -1;
                       }
                  }
             }
        }
#endif        
    }
    return 0;

}

int free_last_to_index(int index ,unsigned char unit, char del_flag)
{
    int i = 0;
    entry_rule *entry = NULL;
	entry_rule *preentry = NULL;
    
    for(i = CMAP_MAX_RULE_NUM-1; i > index; i--)
    {
        if(service_index_alloc.service_rule[i].service_index_flag == 1)
        {
			entry = service_index_alloc.service_rule[i].head;
			while(entry)
			{
                if(entry->flag != RULE_NO_USED)
                {
                    first_free_entryId[entry->unit]--;
                    if(entry->rule != NULL)
                    {
                        (* service_index_alloc.nam_set_entry)(entry->entryId, 
                            entry->rule, del_flag, entry->phase, entry->unit);
                    }
                }
				if(entry->flag == RULE_DELETED)
				{
					if(entry == service_index_alloc.service_rule[i].head)
					{
						service_index_alloc.service_rule[i].head = entry->next;
						free(entry);
						entry = service_index_alloc.service_rule[i].head;
					}
					else
					{
						preentry->next = entry->next;
						free(entry);
						entry = preentry->next;
					}
				}
				else
				{
					preentry = entry;
					entry = entry->next;
				}
			}
			
#if 0            
            for(k = service_index_alloc.rule_entry_max-1; k >= 0; k--)
            {
                for(m = CLASSMAP_PHASE-1; m >= 0; m--)
                {
                    if(service_index_alloc.service_rule[i].entry[k][m][unit].flag != RULE_NO_USED)
                    {
                        first_free_entryId--;  
                        if(service_index_alloc.service_rule[i].entry[k][m][unit].rule != NULL)
                        {
                            (* service_index_alloc.nam_set_entry)(first_free_entryId, 
                                service_index_alloc.service_rule[i].entry[k][m][unit].rule, del_flag, m, unit);
                        }

                    }
                }
            }
#endif            
            service_index_alloc.service_rule[i].service_index_flag = 2;
        }
    }
    
    return 0;
}

int install_index_to_last(int index, unsigned char unit, char del_flag)
{
    int i = 0;
    entry_rule *entry = NULL;

    for(i = index+1; i < CMAP_MAX_RULE_NUM-1; i++)
    {
        if(service_index_alloc.service_rule[i].service_index_flag == 2)
        {
            for(entry = service_index_alloc.service_rule[i].head;
                      entry; entry = entry->next)
            {
                if(entry->flag == RULE_USED)
                {
                    entry->entryId = first_free_entryId[entry->unit];
					if(entry->rule)
                         (* service_index_alloc.nam_set_entry)(entry->entryId, 
                            entry->rule, del_flag, entry->phase, entry->unit);
                     first_free_entryId[entry->unit]++;
                }
            }      
            service_index_alloc.service_rule[i].service_index_flag = 1;
        }
    }
    return 0;
}

int acl_entry_alloc(int index, unsigned char unit, int offset, unsigned char lkphase)
{
    unsigned int entryId = 0;
    entry_rule *entry = NULL;

    if(service_index_alloc.service_rule[index].service_index_flag == 0)
    {        
        free_last_to_index(index, unit, ACL_TCAM_DEL);        
        service_index_alloc.service_rule[index].service_index_flag = 1;
        entry = malloc(sizeof(entry_rule));
        entry->entryId = first_free_entryId[unit];
        entry->flag = RULE_USED;
        entry->rule = NULL;
        entry->offset = offset;
        entry->unit = unit;
        entry->phase = lkphase;
        entry->next = service_index_alloc.service_rule[index].head;
        service_index_alloc.service_rule[index].head = entry;
        entryId = first_free_entryId[unit];
        first_free_entryId[unit]++;  
        install_index_to_last(index, unit, ACL_TCAM_SET);
    }
    else if(service_index_alloc.service_rule[index].service_index_flag == 1)
    {
        entry_rule *preentry;
        for(entry = service_index_alloc.service_rule[index].head;
                  entry; entry = entry->next)
        {
            if((entry->phase == lkphase) && (entry->offset == offset)
                && (entry->unit == unit))
            {
                if(entry->flag != RULE_USED)
                {
                    entry->flag = RULE_USED;
                    entry->rule = NULL;
                    return entry->entryId;
                }
                else
                {
                    npd_syslog_acl_err("ACL alloc a entry existed unit %d, offset %d , phase %d\n",
                        unit, offset, lkphase);
                    return -1;
                }
            }
            preentry = entry;
        }
        free_last_to_index(index, unit, ACL_TCAM_DEL);        
        service_index_alloc.service_rule[index].service_index_flag = 1;
        entry = malloc(sizeof(entry_rule));
        entry->entryId = first_free_entryId[unit];
        entry->flag = RULE_USED;
        entry->rule = NULL;
        entry->next = NULL;
        entry->offset = offset;
        entry->unit = unit;
        entry->phase = lkphase;
        preentry->next = entry;
        entryId = first_free_entryId[unit];
        first_free_entryId[unit]++;  
        install_index_to_last(index, unit, ACL_TCAM_SET);
    }
    return entryId;
}

void acl_entry_loop_op(
    int index, 
    unsigned char devnum, 
    unsigned char lkphase, 
    void (*op)(void *rule, void *param, unsigned char phase, int entryId), 
    void *arg
    )
{
    entry_rule *entry = NULL;
    if(service_index_alloc.service_rule[index].service_index_flag == 1)
    {
        for(entry = service_index_alloc.service_rule[index].head;
                  entry; entry = entry->next)
        {
            if((entry->phase == lkphase)
                && (entry->unit == devnum))
            {
                if(entry->flag == RULE_USED)
                {
                    (*op)(entry->rule, arg, lkphase, entry->entryId);
                }
            }
        }
    }
    return ;    
}
void* acl_entry_get(int index, unsigned char unit, int offset, unsigned char lkphase, int *entryId)
{
    entry_rule *entry = NULL;
    if(service_index_alloc.service_rule[index].service_index_flag == 1)
    {
        for(entry = service_index_alloc.service_rule[index].head;
                  entry; entry = entry->next)
        {
            if((entry->phase == lkphase) && (entry->offset == offset)
                && (entry->unit == unit))
            {
                if(entry->flag != RULE_NO_USED)
                {
				    *entryId = entry->entryId;
                    return entry->rule;
                }
            }
        }       
    }
    *entryId = -1;
    return NULL;
}

int acl_entry_id_free(int index,  int offset, unsigned char unit, unsigned char lkphase)
{
    int del_flag = ACL_TCAM_DEL;
    int entryId = 0;
    entry_rule *entry = NULL;

    if(service_index_alloc.service_rule[index].service_index_flag == 1)
    {
        for(entry = service_index_alloc.service_rule[index].head;
                  entry; entry = entry->next)
        {
            if((entry->phase == lkphase) && (entry->offset == offset)
                && (entry->unit == unit))
            {
                entry->flag = RULE_DELETED;
                entryId = entry->entryId;
                if(NULL != entry->rule)
                {
                    (* service_index_alloc.nam_set_entry)(entryId, 
                       entry->rule, del_flag, lkphase, unit);
                }
                if(entry->rule)
                {
                    free(entry->rule);
                    entry->rule = NULL;
                }
                
            }
        }        
    }
    else
    {
        return -1;
    }
    return 0;
}

int acl_entry_del(int index, int entry_max, int offset, char unit)
{
    entry_rule *entry = NULL;
    
    free_last_to_index(index - 1, unit, ACL_TCAM_DEL);
    
    service_index_alloc.service_rule[index].service_index_flag = 0;
    entry = service_index_alloc.service_rule[index].head;
    while(entry)
    {
        if(entry->rule)
            free(entry->rule);
        service_index_alloc.service_rule[index].head = entry->next;
        free(entry);
        entry = service_index_alloc.service_rule[index].head;
    }
    
    install_index_to_last(index, unit, ACL_TCAM_SET);
    return 0;
}

void* acl_rule_entry_get(int index, int offset, unsigned char unit, unsigned char lkphase, int rule_size)
{
    entry_rule *entry = NULL;
    
    if(service_index_alloc.service_rule[index].service_index_flag == 1)
    {
        for(entry = service_index_alloc.service_rule[index].head;
                  entry; entry = entry->next)
        {
            if((entry->phase == lkphase) && (entry->offset == offset)
                && (entry->unit == unit))
            {
                if(entry->flag == RULE_USED)
                {
                    entry->rule = (void *)malloc(rule_size);
                    memset(entry->rule, 0, rule_size);
                }
                return entry->rule;
            }
        }       
    }
    else
    {
        return NULL;
    }
    return NULL;
}
#endif
int npd_classmap_fpId_get(int service_policy_index, int unit, unsigned int type)
{
	
	return -1;
}


int npd_classmap_fpId_check(int service_policy_index, unsigned int type)
{

	return -1;
}



int npd_classmap_fpId_del(int service_policy_index, unsigned int type)
{
	return MATCH_RETURN_CODE_SUCCESS;
}

int npd_classmap_port_group_get(int service_policy_index, int *portGrpId)
{
	int i = 0;
	unsigned int    portgroupId = 0;
	struct port_group_s  portGrpEnt;

	portGrpEnt.service_policy_index = service_policy_index;
	
	if (0 != dbtable_hash_search(port_group_hash_index, &portGrpEnt, NULL, &portGrpEnt))
	{
		dbtable_array_insert(port_group_array_index, &portgroupId, &portGrpEnt);
		portGrpEnt.portGroupId = portgroupId + 1;  /*avoid portGroupId = 0*/
        
		for (i = 0; i< ACL_PHASE_MAX; i++)
		{
            int j;
            for(j = 0; j < PORT_GROUP_MAX_RULE; j++)
            {
			    portGrpEnt.AclPhaseId[i][j][0] = -1;
			    portGrpEnt.AclPhaseId[i][j][1] = -1;
//			    portGrpEnt.AclPhaseId[i][j][2] = -1;
//			    portGrpEnt.AclPhaseId[i][j][3] = -1;
            }
		}
        portGrpEnt.head = NULL;
		dbtable_array_update(port_group_array_index, portgroupId, &portGrpEnt, &portGrpEnt);
	}

	*portGrpId = portGrpEnt.portGroupId;

	return MATCH_RETURN_CODE_SUCCESS;
}

int npd_classmap_port_group_del(int service_policy_index)
{
	struct port_group_s  portGrpEnt;

	portGrpEnt.service_policy_index = service_policy_index;
	
	if (0 != dbtable_hash_search(port_group_hash_index, &portGrpEnt, NULL, &portGrpEnt))
	{
        /*
		for (i = 0; i < ACL_PHASE_MAX; i++)
		{
			if (portGrpEnt.AclPhaseId[i] != -1)
			{
				nam_FP_index_free(portGrpEnt.AclPhaseId[i]);
			}
		}
		*/
		dbtable_hash_delete(port_group_hash_index, &portGrpEnt, &portGrpEnt);
	}

	return MATCH_RETURN_CODE_SUCCESS;
}


int class_map_check_exist(const char* map_name)
{
    int ret = 0;
	struct class_map_index_s class_map;
	
	memset(&class_map, 0, sizeof(struct class_map_index_s));

    strcpy(class_map.map_name, map_name);
    ret = dbtable_hash_search(class_map_master_name, &class_map, NULL, &class_map);
    if(0 == ret)
    {
        return CLASSMAP_RETURN_CODE_EXIST;
    }
    else
    {
        return CLASSMAP_RETURN_CODE_NOTEXIST;
    }    
}

int class_map_find_by_name(const char* map_name, struct class_map_index_s* class_map)
{
    int ret = MATCH_RETURN_CODE_SUCCESS;

	if( map_name == NULL || class_map == NULL )
	{
		return DIFFSERV_RETURN_CODE_ERROR;
	}
	
    strcpy(class_map->map_name, map_name);
    ret = dbtable_hash_search(class_map_master_name, class_map, NULL, class_map);
    if (0 == ret)
    {
        return CLASSMAP_RETURN_CODE_EXIST;
    }
    else
    {
        return CLASSMAP_RETURN_CODE_NOTEXIST;
    }
}

int class_map_create(const char* map_name)
{
    int 			ret = MATCH_RETURN_CODE_SUCCESS;
    unsigned int	id = 0;
    struct class_map_index_s class_map;
    struct class_map_index_s class_map_1;
	memset(&class_map, 0, sizeof(struct class_map_index_s));

    strcpy(class_map.map_name, map_name);    
    class_map.is_binded = 0;
    
    ret = dbtable_hash_search(class_map_master_name, &class_map, NULL, &class_map);
    
	if (0 != ret)
    {
        if(strncmp("ACL_", map_name, 4) == 0)
        {
            id = strtoul(&class_map.map_name[4], NULL, 0);
            ret = dbtable_array_get(class_map_master, id + CLASSMAP_ACL_ID_INDEX_START, &class_map_1);
            if(ret != 0)
            {
                class_map.index = id + CLASSMAP_ACL_ID_INDEX_START;
                ret = dbtable_array_insert_byid(class_map_master, class_map.index, &class_map);
                if (0 != ret)
                {
                	syslog_ax_acl_err("Class map cannot be insert to detable(class_map_create)!\n");
                    return DIFFSERV_RETURN_CODE_ERROR;
                }
            }
        }
        else  if(strncmp("SG_", map_name, 3) == 0)
        {
            if(!strcmp(map_name, SG_DEFAULT_DENY))
            {
                id = SG_DEFAULT_DENY_ID;
            }
            else if(!strcmp(map_name, SG_DEFAULT_PERMIT))
            {
                id = SG_DEFAULT_PERMIT_ID;
            }
            else
            {
                id = strtoul(&class_map.map_name[3], NULL, 0);
            }
            ret = dbtable_array_get(class_map_master, id + CLASSMAP_SERVICE_SG_INDEX_START, &class_map_1);
            if(ret != 0)
            {
                class_map.index = id + CLASSMAP_SERVICE_SG_INDEX_START;
                ret = dbtable_array_insert_byid(class_map_master, class_map.index, &class_map);
                if (0 != ret)
                {
                	syslog_ax_acl_err("Class map cannot be insert to detable(class_map_create)!\n");
                    return DIFFSERV_RETURN_CODE_ERROR;
                }
            }
        }
#ifdef HAVE_PORTAL        
        else if(strncmp("PORTAL_STA_", map_name, 11) == 0)
        {
            ret = dbtable_array_insert_after(class_map_master, &id, &class_map, CLASSMAP_SERVICE_PORTAL_STA_INDEX_START);
            if (0 != ret)
            {
                syslog_ax_acl_err("Class map cannot be insert to detable(class_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
            class_map.index = (int)id;
            ret = dbtable_array_update(class_map_master, id, NULL, &class_map);
            if (0 != ret)
            {
            	syslog_ax_acl_err("Class map cannot update in detable(class_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
        }
        else if(strncmp("PORTAL_INTF_", map_name, 12) == 0)
        {
            ret = dbtable_array_insert_after(class_map_master, &id, &class_map, CLASSMAP_SERVICE_PORTAL_INTF_INDEX_START);
            if (0 != ret)
            {
                syslog_ax_acl_err("Class map cannot be insert to detable(class_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
            class_map.index = (int)id;
            ret = dbtable_array_update(class_map_master, id, NULL, &class_map);
            if (0 != ret)
            {
            	syslog_ax_acl_err("Class map cannot update in detable(class_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
        }
#endif        
        else
        {
            ret = dbtable_array_insert_after(class_map_master, &id, &class_map, CLASSMAP_SERVICE_INDEX_START);
            if (0 != ret)
            {
                syslog_ax_acl_err("Class map cannot be insert to detable(class_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
            class_map.index = (int)id;
            ret = dbtable_array_update(class_map_master, id, NULL, &class_map);
            if (0 != ret)
            {
            	syslog_ax_acl_err("Class map cannot update in detable(class_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
        }
        
    }
    else if(class_map.is_binded != 0)
    {
        return CLASSMAP_RETURN_CODE_EXIST;
    }

	return MATCH_RETURN_CODE_SUCCESS;
}

int class_map_delete (const char* map_name)
{
	int ret = MATCH_RETURN_CODE_SUCCESS;
	struct class_map_index_s	db_index;
	struct class_map_rule_s		rule;

	memset(&db_index, 0, sizeof(struct class_map_index_s));
	memset(&rule, 0, sizeof(struct class_map_rule_s));

    {
        struct time_range_info_s time_range_info;
        if(TRUE == npd_is_acl_associate_time_range_info((char*)map_name,(char*)time_range_info.name))
        {
            dbtable_hash_search(time_range_info_name,&time_range_info,NULL,&time_range_info);
            time_range_info.acl_bind_count--;
            ret = dbtable_hash_insert(time_range_info_name,&time_range_info);
            if(0 != ret)
            {
                syslog_ax_acl_err("can't acl_bind_count--");
                return FALSE;
            }
        }
    }

	strcpy(db_index.map_name, map_name);
	ret = dbtable_hash_search(class_map_master_name, &db_index, NULL, &db_index);
	if(0 != ret)
	{
		return CLASSMAP_RETURN_CODE_NOTEXIST;
	}

	if(db_index.is_binded > 0)
	{
		return CLASSMAP_RETURN_CODE_BINDED;
	}
    
	strcpy(rule.map_name, db_index.map_name);
	rule.index = db_index.index;

	ret = dbtable_hash_head_key(class_map_match_rule, &rule, &rule, &class_map_rule_filter);
    while(0 == ret)
    {
        if((0 == strcmp(rule.cmd_name, "l4port_range_src")) 
            || (0 == strcmp(rule.cmd_name, "l4port_range_dst")))
        {
            npd_l4port_range_del_by_cmd_arg(rule.cmd_arg);
        }
        ret = dbtable_hash_delete(class_map_match_rule, &rule, &rule);
		if(0 != ret)
		{
			syslog_ax_acl_err("Class map rule cannot be remove from detable(class_map_rename)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}
		
        ret = dbtable_hash_head_key(class_map_match_rule, &rule, &rule, &class_map_rule_filter);
    }

    ret = dbtable_array_delete(class_map_master, db_index.index, &db_index);
	if (0 != ret)
	{
		syslog_ax_acl_err("Class map cannot be remove from detable(class_map_delete)!\n");
		return DIFFSERV_RETURN_CODE_ERROR;
	}
	
    return MATCH_RETURN_CODE_SUCCESS;
}

int class_map_rename(const char* oldmapname, const char* newmapname)
{
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	const char*	oldmap_name = oldmapname;
	const char*	newmap_name = newmapname;
	struct class_map_index_s	old_map;
	struct class_map_index_s	new_map;
	struct class_map_rule_s		class_rule;
	struct class_map_rule_s		temp_class_rule;
	struct class_map_rule_s		del_class_rule;

	memset(&old_map, 0, sizeof(struct class_map_index_s));
	memset(&new_map, 0, sizeof(struct class_map_index_s));
	memset(&class_rule, 0, sizeof(struct class_map_rule_s));
	memset(&temp_class_rule, 0, sizeof(struct class_map_rule_s));
	memset(&del_class_rule, 0, sizeof(struct class_map_rule_s));
	
	if(CLASSMAP_RETURN_CODE_EXIST != class_map_find_by_name(oldmap_name, &old_map)) 
	{
		return CLASSMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != old_map.is_binded)
	{
		return CLASSMAP_RETURN_CODE_BINDED;
	}
	else if (CLASSMAP_RETURN_CODE_EXIST == class_map_check_exist(newmap_name))
	{
		return CLASSMAP_RETURN_CODE_EXIST;
	}
	else
	{
		if (MATCH_RETURN_CODE_SUCCESS != class_map_create(newmap_name))
		{
			return CLASSMAP_RETURN_CODE_CREATE_ERROR;
		}
		
		if (CLASSMAP_RETURN_CODE_EXIST != class_map_find_by_name(newmap_name, &new_map))
		{
			return CLASSMAP_RETURN_CODE_NOTEXIST;
		}

		strcpy(class_rule.map_name, old_map.map_name);
		class_rule.index = old_map.index;
				
		ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
		while(0 == ret)
		{
			memcpy(&temp_class_rule, &class_rule, sizeof(struct class_map_rule_s));
			strcpy(temp_class_rule.map_name, new_map.map_name);
			temp_class_rule.index = new_map.index;
			ret = dbtable_hash_delete(class_map_match_rule, &temp_class_rule, &del_class_rule);
			if(0 != ret)
			{
				syslog_ax_acl_err("Class map cannot be remove from detable(class_map_rename)!\n");
				return DIFFSERV_RETURN_CODE_ERROR;
			}
					
			ret = dbtable_hash_insert(class_map_match_rule, &temp_class_rule);
			if(0 != ret)
			{
				syslog_ax_acl_err("Class map cannot be insert to detable(class_map_rename)!\n");
				return DIFFSERV_RETURN_CODE_ERROR;
			}
			
			ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
		}

		ret = class_map_delete(oldmap_name);
		if (0 != ret)
		{
			return CLASSMAP_RETURN_CODE_DELETEERROR;
		}
	}

	return MATCH_RETURN_CODE_SUCCESS;
}

/* Add match statement to route map. */
int class_map_add_match (struct class_map_index_s* index, const char* match_name,
                     const char* match_arg, const char* match_mask)
{
	int			ret = MATCH_RETURN_CODE_SUCCESS;
	int 		ni = 0;
	rule_cmd_t* cmd = NULL;
	struct class_map_index_s* 	db_index = (struct class_map_index_s*)index;
	struct class_map_rule_s		rule;
	struct class_map_rule_s		del_rule;
    int i;
    char tmp_phase;

	memset(&rule, 0, sizeof(struct class_map_rule_s));
	memset(&del_rule, 0, sizeof(struct class_map_rule_s));

	cmd = class_map_lookup_match(match_name);
	if(NULL == cmd)
	{
		return CLASSMAP_RETURN_CODE_NOTFIND;
	}

	strcpy(rule.map_name, db_index->map_name);
	strcpy(rule.cmd_name, match_name);
	rule.index = db_index->index;

	ret = dbtable_hash_search(class_map_match_rule, &rule, (unsigned int(*)(void*, void*))class_map_rule_simple_cmp, &del_rule);
	if(0 == ret)
	{
		ret = dbtable_hash_delete(class_map_match_rule, &del_rule, &del_rule);
		if(0 != ret)
		{
			syslog_ax_acl_err("Class map rule cannot be remove from detable(class_map_add_match)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}
	}

	if(NULL != cmd->arg)
	{
		for (ni = 0; ni < cmd->arg->preset_value_num; ni++)
		{
			if(strncmp(cmd->arg->value[ni]->preset_value_name, match_arg, strlen(match_arg)) == 0)
			{
				(*cmd->func_compile)((char *)cmd->arg->value[ni]->preset_value, rule.cmd_arg, &rule.lk_phase);
                (*cmd->func_compile)((char *)match_mask, rule.cmd_mask, &tmp_phase);
				break;
			}
		}

		if ((ni >= cmd->arg->preset_value_num) && (cmd->func_compile))
		{
			(*cmd->func_compile)((char *)match_arg, rule.cmd_arg, &rule.lk_phase);
            (*cmd->func_compile)((char *)match_mask, rule.cmd_mask, &tmp_phase);
		}
        
	}
	else if(cmd->func_compile)
	{
		(*cmd->func_compile)((char *)match_arg, rule.cmd_arg, &rule.lk_phase);
		
		(*cmd->func_compile)((char *)match_mask, rule.cmd_mask, &tmp_phase);
	}
	else
	{
		return CLASSMAP_RETURN_CODE_ADDMATCH_ERROR;
	}
    if(0 != strcmp("time-range",match_name))
    {    for(i = 0; i < sizeof(rule.cmd_arg); i++)
        {
            rule.cmd_arg[i] = rule.cmd_arg[i]&rule.cmd_mask[i];
        }
    }
    else
    {
        struct time_range_info_s time_range_info;
        strcpy(time_range_info.name,rule.cmd_arg);
        dbtable_hash_search(time_range_info_name,&time_range_info,NULL,&time_range_info);
        time_range_info.acl_bind_count++;
        ret = dbtable_hash_insert(time_range_info_name,&time_range_info);
        if(0 != ret)
        {
            syslog_ax_acl_err("%% can't acl_bind_count++");
            return FALSE;
        }
    }
   
	ret = dbtable_hash_insert(class_map_match_rule, &rule);
	if(0 != ret)
	{
		syslog_ax_acl_err("%% Class map rule cannot be insert to detable(class_map_add_match)!\n");
		return DIFFSERV_RETURN_CODE_ERROR;
	}
	
	return MATCH_RETURN_CODE_SUCCESS;
}

int class_map_delete_match (struct class_map_index_s *index, const char *match_name)
{
	int 		ret = MATCH_RETURN_CODE_SUCCESS;
	struct class_map_rule_s rule;
	struct class_map_rule_s del_rule;

	memset(&rule, 0, sizeof(struct class_map_rule_s));
	memset(&del_rule, 0, sizeof(struct class_map_rule_s));

	strcpy(rule.map_name, index->map_name);
	rule.index = index->index;
	strcpy(rule.cmd_name, match_name);
	ret = dbtable_hash_search(class_map_match_rule, &rule, (unsigned int(*)(void*, void*))class_map_rule_simple_cmp, &rule);
	if(0 != ret)
	{
		return CLASSMAP_RETURN_CODE_RULE_EMPTY;
	}
	ret = dbtable_hash_delete(class_map_match_rule, &rule, &del_rule);
	if(0 != ret)
	{
		syslog_ax_acl_err("Class map rule cannot be remove from detable(class_map_delete_match)!\n");
		return DIFFSERV_RETURN_CODE_ERROR;
	}
    if((0 ==  strcmp(match_name, "l4port_range_src"))
        || (0 == strcmp(match_name, "l4port_range_dst")))
    {
        npd_l4port_range_del_by_cmd_arg(rule.cmd_arg);
    }
	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_check_exist(const char * map_name)
{
    int ret = MATCH_RETURN_CODE_SUCCESS;
    struct policy_map_index_s map;

	memset(&map, 0, sizeof(struct policy_map_index_s));

    strcpy(map.map_name, map_name);
    ret = dbtable_hash_search(policy_map_name, &map, NULL, &map);
    if(0 == ret)
    {
        return POLICYMAP_RETURN_CODE_EXIST;
    }
    else
    {
        return POLICYMAP_RETURN_CODE_NOTEXIST;
    } 

	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_find_by_name(const char* map_name, struct policy_map_index_s* policy_name)
{
    int ret = MATCH_RETURN_CODE_SUCCESS;

	if( map_name == NULL || policy_name == NULL )
	{
		return DIFFSERV_RETURN_CODE_ERROR;
	}
	
    strcpy(policy_name->map_name, map_name);
    ret = dbtable_hash_search(policy_map_name, policy_name, NULL, policy_name);
    if(0 == ret)
    {
        return POLICYMAP_RETURN_CODE_EXIST;
    }
    else
    {
        return POLICYMAP_RETURN_CODE_NOTEXIST;
    }

	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_create(const char * map_name)
{
    int				ret = MATCH_RETURN_CODE_SUCCESS;
	unsigned int	id = 0;
    struct policy_map_index_s map;
    struct policy_map_index_s map_1;

	memset(&map, 0, sizeof(struct policy_map_index_s));
    strcpy(map.map_name, map_name);
	map.is_deployed = 0;
	map.class_map_index = -1;
    
    ret = dbtable_hash_search(policy_map_name, &map, NULL, &map);
    if(0 != ret)
    {
        if(strncmp("ACL_", map_name, 4) == 0)
        {
            id = strtoul(&map.map_name[4], NULL, 0) + CLASSMAP_ACL_ID_INDEX_START;
            map.index = id;
            ret = dbtable_array_get(policy_map_index, id, &map_1);
            if(ret != 0)
            {
                ret = dbtable_array_insert_byid(policy_map_index, id, &map);
                if(0 != ret)
                {
                    syslog_ax_acl_err("Policy map cannot be insert to detable(policy_map_create)!\n");
                    return DIFFSERV_RETURN_CODE_ERROR;
                }
            }
        }
        else if(strncmp("SG_", map_name, 3) == 0)
        {
            if(!strcmp(map_name, SG_DEFAULT_DENY))
            {
                id = CLASSMAP_SERVICE_SG_INDEX_START + SG_DEFAULT_DENY_ID;
            }
            else if(!strcmp(map_name, SG_DEFAULT_PERMIT))
            {
                id = CLASSMAP_SERVICE_SG_INDEX_START + SG_DEFAULT_PERMIT_ID;
            }
            else
            {
                id = strtoul(&map.map_name[3], NULL, 0) + CLASSMAP_SERVICE_SG_INDEX_START;
            }            
            map.index = id;
            ret = dbtable_array_get(policy_map_index, id, &map_1);
            if(ret != 0)
            {
                ret = dbtable_array_insert_byid(policy_map_index, id, &map);
                if(0 != ret)
                {
                    syslog_ax_acl_err("Policy map cannot be insert to detable(policy_map_create)!\n");
                    return DIFFSERV_RETURN_CODE_ERROR;
                }
            }
        }
#ifdef HAVE_PORTAL        
        else if(strncmp("PORTAL_STA_", map_name, 11) == 0)
        {
            ret = dbtable_array_insert_after(policy_map_index, &id, &map, CLASSMAP_SERVICE_PORTAL_STA_INDEX_START);
            if(0 != ret)
            {
            	syslog_ax_acl_err("Policy map cannot be insert to detable(policy_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
            map.index = (unsigned int)id;
            ret = dbtable_array_update(policy_map_index, id, NULL, &map);
            if(0 != ret)
            {
            	syslog_ax_acl_err("Policy map cannot update in detable(policy_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
        }
        else if(strncmp("PORTAL_INTF_", map_name, 12) == 0)
        {
            ret = dbtable_array_insert_after(policy_map_index, &id, &map, CLASSMAP_SERVICE_PORTAL_INTF_INDEX_START);
            if(0 != ret)
            {
            	syslog_ax_acl_err("Policy map cannot be insert to detable(policy_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
            map.index = (unsigned int)id;
            ret = dbtable_array_update(policy_map_index, id, NULL, &map);
            if(0 != ret)
            {
            	syslog_ax_acl_err("Policy map cannot update in detable(policy_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
        }
#endif        
        else
        {
            ret = dbtable_array_insert_after(policy_map_index, &id, &map, CLASSMAP_SERVICE_INDEX_START);
            if(0 != ret)
            {
            	syslog_ax_acl_err("Policy map cannot be insert to detable(policy_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
            map.index = (unsigned int)id;
            ret = dbtable_array_update(policy_map_index, id, NULL, &map);
            if(0 != ret)
            {
            	syslog_ax_acl_err("Policy map cannot update in detable(policy_map_create)!\n");
                return DIFFSERV_RETURN_CODE_ERROR;
            }
        }
    }
    else
    {
		return POLICYMAP_RETURN_CODE_EXIST;
    } 
	
	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_delete (const char* map_name)
{
    struct policy_map_index_s policy_index;
    struct policy_map_rule_s rule;
	struct policy_map_rule_s del_rule;
	struct class_map_index_s class_map;
    int ret = MATCH_RETURN_CODE_SUCCESS;

	memset(&policy_index, 0, sizeof(struct policy_map_index_s));
	memset(&rule, 0, sizeof(struct policy_map_rule_s));
	memset(&del_rule, 0, sizeof(struct policy_map_rule_s));
	memset(&class_map, 0, sizeof(struct class_map_index_s));

    strcpy(policy_index.map_name, map_name);
    ret = dbtable_hash_search(policy_map_name, &policy_index, NULL, &policy_index);

    if(0 != ret)
    {
        return POLICYMAP_RETURN_CODE_NOTEXIST;
    }

    if(0 != policy_index.is_deployed)
    {
        return POLICYMAP_RETURN_CODE_DEPLOYED;
    }

    if(strlen(policy_index.acl_group) > 0)
    {
        return POLICYMAP_RETURN_CODE_BIND_ACL_GROUP;
    }
    
    if (-1 != policy_index.class_map_index)
	{
		ret = dbtable_array_get(class_map_master, policy_index.class_map_index, &class_map);
	    if(0 != ret)
	    {
	        return CLASSMAP_RETURN_CODE_NOTEXIST;
	    }

		if (class_map.is_binded > 0)
		{
			class_map.is_binded--;
		}
		else
		{
			return CLASSMAP_RETURN_CODE_NOTBINDED;
		}
		
		ret = dbtable_array_update(class_map_master, class_map.index, NULL, &class_map);
		if(0 != ret)
		{
			syslog_ax_acl_err("Class map cannot update in detable(policy_map_delete)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}
	}
    
    strcpy(rule.map_name, policy_index.map_name);
    rule.index = policy_index.index;

    ret = dbtable_hash_head_key(policy_map_set_rule, &rule, &rule, &policy_map_rule_filter);
    while(0 == ret)
    {    
        #if 0
        if (0 == strcmp("policer", rule.cmd_name))
        {
            unsigned int param0;            
            QOS_POLICER_STC policer = { 0 };
            memcpy(&param0, rule.cmd_arg, sizeof(unsigned int));
            
            ret = npd_policer_get_by_index(param0, &policer);
            if(0 != ret)
                return QOS_RETURN_CODE_POLICER_NOT_EXISTED;

            policer.swPortNum--;
            ret = npd_policer_set_by_index(param0, &policer);
        }
        ret = dbtable_hash_delete(policy_map_set_rule, &rule, &del_rule);
		if(0 != ret)
		{
			syslog_ax_acl_err("Policy map cannot be delete from detable(policy_map_delete)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}
        #endif
        ret = policy_map_delete_set(&policy_index, rule.cmd_name);
		ret = dbtable_hash_head_key(policy_map_set_rule, &rule, &rule, &policy_map_rule_filter);
    }

    ret = dbtable_array_delete(policy_map_index, policy_index.index, &policy_index);
	if(0 != ret)
	{
		syslog_ax_acl_err("Policy map cannot be delete from detable(policy_map_delete)!\n");
		return DIFFSERV_RETURN_CODE_ERROR;
	}

    return MATCH_RETURN_CODE_SUCCESS; 
}

int policy_map_rename(const char* oldmapname, const char* newmapname)
{
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	const char*	pNameOld = oldmapname;
	const char*	pNameNew = newmapname;
    struct policy_map_index_s	old_policymap;
	struct policy_map_index_s	new_policymap;
    struct policy_map_rule_s	policyrule;
    struct policy_map_rule_s	temp_policyrule;
    struct policy_map_rule_s	del_policyrule;
	struct class_map_index_s	class_map;

	memset(&old_policymap, 0, sizeof(struct policy_map_index_s));
	memset(&new_policymap, 0, sizeof(struct policy_map_index_s));
	memset(&policyrule, 0, sizeof(struct policy_map_rule_s));
	memset(&temp_policyrule, 0, sizeof(struct policy_map_rule_s));
	memset(&del_policyrule, 0, sizeof(struct policy_map_rule_s));
	memset(&class_map, 0, sizeof(struct class_map_index_s));


	if(POLICYMAP_RETURN_CODE_NOTEXIST == policy_map_find_by_name(pNameOld, &old_policymap)) 
	{
		return POLICYMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != old_policymap.is_deployed)
	{
		return POLICYMAP_RETURN_CODE_DEPLOYED;
	}
	else if(POLICYMAP_RETURN_CODE_NOTEXIST != policy_map_check_exist(pNameNew)) 
	{
		return POLICYMAP_RETURN_CODE_EXIST;
	}
	else 
	{
		if (MATCH_RETURN_CODE_SUCCESS == policy_map_create(pNameNew))
		{
			ret = policy_map_find_by_name(pNameNew, &new_policymap);
			if(POLICYMAP_RETURN_CODE_EXIST == ret) 
			{
				if (-1 != old_policymap.class_map_index)
				{
					new_policymap.class_map_index = old_policymap.class_map_index;
					ret = dbtable_array_get(class_map_master, old_policymap.class_map_index, &class_map);
				    if(0 != ret)
				    {
				        return CLASSMAP_RETURN_CODE_NOTEXIST;
				    }

					class_map.is_binded++;
					ret = dbtable_array_update(class_map_master, class_map.index, NULL, &class_map);
					if(0 != ret)
					{
						syslog_ax_acl_err("Class map cannot update in detable(policy_map_rename)!\n");
						return DIFFSERV_RETURN_CODE_ERROR;
					}

				}
				
    		    ret = dbtable_array_update(policy_map_index, new_policymap.index, NULL, &new_policymap);
		        if(0 != ret)
		        {
		        	syslog_ax_acl_err("Policy map cannot update in detable(policy_map_rename)!\n");
		            return DIFFSERV_RETURN_CODE_ERROR;
		        }

				strcpy(policyrule.map_name, old_policymap.map_name);
				policyrule.index = old_policymap.index;
			
				ret = dbtable_hash_head_key(policy_map_set_rule, &policyrule, &policyrule, policy_map_rule_filter);
				while(0 == ret)
				{
					memcpy(&temp_policyrule, &policyrule, sizeof(struct policy_map_rule_s));
					strcpy(temp_policyrule.map_name, new_policymap.map_name);
					temp_policyrule.index = new_policymap.index;
					ret = dbtable_hash_delete(policy_map_set_rule, &temp_policyrule, &del_policyrule);
					if(0 != ret)
					{
						syslog_ax_acl_err("Policy map cannot be delete from detable(policy_map_rename)!\n");
						return DIFFSERV_RETURN_CODE_ERROR;
					}
					ret = dbtable_hash_insert(policy_map_set_rule, &temp_policyrule);
					if(0 != ret)
					{
						syslog_ax_acl_err("Policy map cannot be insert to detable(policy_map_rename)!\n");
						return DIFFSERV_RETURN_CODE_ERROR;
					}
					ret = dbtable_hash_next_key(policy_map_set_rule, &policyrule, &policyrule, policy_map_rule_filter);
				}
				
				ret = policy_map_delete(old_policymap.map_name);
				if(0 == ret)
				{
					return MATCH_RETURN_CODE_SUCCESS;
				}
			}
			else
			{
				return POLICYMAP_RETURN_CODE_NOTFIND;
			}
		}
		else
		{
			return POLICYMAP_RETURN_CODE_CREATE_ERROR;
		}
	}
	
	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_add_set(struct policy_map_index_s *index, const char *set_name, const char *set_arg)
{
	int ret = MATCH_RETURN_CODE_SUCCESS;
	int	ni = 0;
	rule_cmd_t*	cmd = NULL;
	struct policy_map_index_s*	db_index = (struct policy_map_index_s*)index;
	struct policy_map_rule_s rule;
	struct policy_map_rule_s del_rule;

	memset(&rule, 0, sizeof(struct policy_map_rule_s));
	memset(&del_rule, 0, sizeof(struct policy_map_rule_s));

	strcpy(rule.map_name, db_index->map_name);
	rule.index = db_index->index;

	strcpy(rule.cmd_name, set_name);
	ret = dbtable_hash_search(policy_map_set_rule, &rule, policy_map_rule_simple_cmp, &del_rule);
	if (0 == ret)
	{
		ret = dbtable_hash_delete(policy_map_set_rule, &del_rule, &del_rule);
		if(0 != ret)
		{
			syslog_ax_acl_err("policy map cannot be remove from detable(policy_map_add_set)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}
	}
    if(!strcmp(set_name,"policer"))
    {
        QOS_POLICER_STC policer = {0};
        int id = atoi(set_arg);

        ret = npd_policer_get_by_index(id, &policer);
        if(0 != ret)
            return QOS_RETURN_CODE_POLICER_NOT_EXISTED;

        policer.swPortNum++;
        ret = npd_policer_set_by_index(id, &policer);
    }

    if(!strcmp(set_name,"qos-profile"))
    {
        QOS_PROFILE_STC     profilePtr;
        unsigned int id = atoi(set_arg);
        if( 0 != npd_qosprofile_get_by_index(id, &profilePtr)) 
            return QOS_RETURN_CODE_PROFILE_NOT_EXISTED;
    }
#ifdef HAVE_ROUTE
	if(!strcmp(set_name, "policy-route"))
	{
		unsigned int ip = 0;
        unsigned int l3index[16];
    	unsigned int net_g_index = 0;
    	unsigned int intfCount = 16;
		struct service_policy_route_s pb;
		struct arp_snooping_item_s item;

        memset(&pb, 0, sizeof(struct service_policy_route_s));
        memset(&item, 0, sizeof(struct arp_snooping_item_s));
        
	    ip = lib_get_ip_from_string((char*)set_arg);
    	ret = npd_intf_addr_ifindex_get_bynet((unsigned int*)&l3index, &intfCount, ip);
        
 		ret = npd_intf_netif_get_by_ifindex(l3index[0], &net_g_index);
   
		strcpy(pb.policy_map_name, db_index->map_name);
		pb.nexthopv4 = ip;
		dbtable_hash_insert(ser_policy_pb_index, &pb);

		item.ipAddr = ip;
		if(NPD_NETIF_VLAN_TYPE == net_g_index)
		    item.vid = npd_netif_vlan_get_vid(net_g_index);
		else
		{
			item.vid = NPD_PORT_L3INTF_VLAN_ID;
			item.ifIndex = net_g_index;
		}
		npd_arp_snooping_solicit_send(&item);
	}
#endif        
	strcpy(rule.cmd_name, set_name);
	cmd = policy_map_lookup_set(set_name);
	if (NULL == cmd)
	{
		return POLICYMAP_RETURN_CODE_NOTFIND;
	}

	if(NULL != cmd->arg)
	{
		for (ni = 0; ni < cmd->arg->preset_value_num; ni++)
		{
			if (strncmp(cmd->arg->value[ni]->preset_value_name, set_arg, strlen(set_arg)) == 0)
			{
				(*cmd->func_compile)((char *)cmd->arg->value[ni]->preset_value, rule.cmd_arg, &rule.lk_phase);
				break;
			}
		}

		if ((ni >= cmd->arg->preset_value_num) && (cmd->func_compile))
		{
			(*cmd->func_compile)((char *)set_arg, rule.cmd_arg, &rule.lk_phase);
		}
	}
	else if(cmd->func_compile)
	{
		(*cmd->func_compile)((char *)set_arg, rule.cmd_arg, &rule.lk_phase);
	}
    
	ret = dbtable_hash_insert(policy_map_set_rule, &rule);
	if(0 != ret)
	{
		syslog_ax_acl_err("policy map cannot insert to detable(policy_map_add_set)!\n");
		return DIFFSERV_RETURN_CODE_ERROR;
	}


	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_delete_set(struct policy_map_index_s* index, const char* set_name)
{
	int ret = MATCH_RETURN_CODE_SUCCESS;
	struct policy_map_index_s*	policy_map = (struct policy_map_index_s*)index;
	struct policy_map_rule_s	rule;
	struct policy_map_rule_s	del_rule;

	memset(&rule, 0, sizeof(struct policy_map_rule_s));
	memset(&del_rule, 0, sizeof(struct policy_map_rule_s));


	strcpy(rule.map_name, policy_map->map_name);
	strcpy(rule.cmd_name, set_name);
	rule.index = policy_map->index;
	ret = dbtable_hash_search(policy_map_set_rule, &rule, (unsigned int (*)(void*, void*))policy_map_rule_simple_cmp, &del_rule);
	if(0 != ret)
	{
		return POLICYMAP_RETURN_CODE_RULE_EMPTY;
	}
 	ret = dbtable_hash_delete(policy_map_set_rule, &del_rule, &del_rule);
	if(0 != ret)
	{
		syslog_ax_acl_err("Policy map cannot remove from detable(policy_map_delete_set)!\n");
		return DIFFSERV_RETURN_CODE_ERROR;
	}
    if(0 == strcmp(set_name,"policer"))
    {
        QOS_POLICER_STC policer = {0};
        int id = *(int*)(del_rule.cmd_arg);

        ret = npd_policer_get_by_index(id, &policer);
        if(0 != ret)
            return QOS_RETURN_CODE_SUCCESS;

        policer.swPortNum--;
        ret = npd_policer_set_by_index(id, &policer);
    }
#ifdef HAVE_ROUTE
	if(!strcmp(set_name, "policy-route"))
	{
		struct service_policy_route_s pb;

        memset(&pb, 0, sizeof(struct service_policy_route_s));
		strcpy(pb.policy_map_name, policy_map->map_name);
		dbtable_hash_delete(ser_policy_pb_index, &pb, &pb);
	}  
#endif    
	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_class(const char* classname, const char* policyname)
{
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	const char*	class_name = classname;
	const char*	policy_name = policyname;
	struct class_map_index_s class_map;
	struct policy_map_index_s policy_map;

	memset(&class_map, 0, sizeof(struct class_map_index_s));
	memset(&policy_map, 0, sizeof(struct policy_map_index_s));
	
	if(CLASSMAP_RETURN_CODE_NOTEXIST == class_map_find_by_name(class_name, &class_map)) 
	{
		return CLASSMAP_RETURN_CODE_NOTEXIST;
	}
	else if(POLICYMAP_RETURN_CODE_NOTEXIST == policy_map_find_by_name(policy_name, &policy_map)) 
	{
		return POLICYMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != policy_map.is_deployed)
	{
		return POLICYMAP_RETURN_CODE_DEPLOYED;
	}
	else if (policy_map.class_map_index == class_map.index)
	{
		return MATCH_RETURN_CODE_SUCCESS;
	}
	else if (-1 != policy_map.class_map_index)
	{
		return POLICYMAP_RETURN_CODE_CLASSMAP_EXIST;
	}
	else
	{
		class_map.is_binded++;
		policy_map.class_map_index = class_map.index;
	
		ret = dbtable_array_update(class_map_master, class_map.index, NULL, &class_map);
		if(0 != ret)
		{
			syslog_ax_acl_err("Class map cannot update in detable(policy_map_class)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}

		ret = dbtable_array_update(policy_map_index, policy_map.index, NULL, &policy_map);
		if(0 != ret)
		{
			syslog_ax_acl_err("Policy map cannot update in detable(policy_map_class)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}
	}

	return MATCH_RETURN_CODE_SUCCESS;
}

int policy_map_no_class(const char* classname, const char* policyname)
{
	int ret = 0;
	struct class_map_index_s	class_map;
	struct policy_map_index_s	policy_map;
	struct policy_map_rule_s	policy_rule;

	memset(&class_map, 0, sizeof(struct class_map_index_s));
	memset(&policy_map, 0, sizeof(struct policy_map_index_s));
	memset(&policy_rule, 0, sizeof(struct policy_map_rule_s));

	if(CLASSMAP_RETURN_CODE_NOTEXIST == class_map_find_by_name(classname, &class_map)) 
	{
		return CLASSMAP_RETURN_CODE_NOTEXIST;
	}
	else if(POLICYMAP_RETURN_CODE_NOTEXIST == policy_map_find_by_name(policyname, &policy_map)) 
	{
		return POLICYMAP_RETURN_CODE_NOTEXIST;
	}
	else if (class_map.index != policy_map.class_map_index)
	{
		return POLICYMAP_RETURN_CODE_NOTTHIS_CLASSMAP;
	}
	else if(0 != policy_map.is_deployed)
	{
		return POLICYMAP_RETURN_CODE_DEPLOYED;
	}
	else if (-1 == policy_map.class_map_index)
	{
		return POLICYMAP_RETURN_CODE_CLASSMAP_NOTEXIST;
	}
	else
	{
		strcpy(policy_rule.map_name, policy_map.map_name);
		policy_rule.index = policy_map.index;
		ret = dbtable_hash_head_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
		if (0 == ret)
		{
			return POLICYMAP_RETURN_CODE_HAS_ADD_ACTION;
		}

		if (class_map.is_binded > 0)
		{
			class_map.is_binded--; 
			ret = dbtable_array_update(class_map_master, class_map.index, NULL, &class_map);
			if(0 != ret)
			{
				syslog_ax_acl_err("Class map cannot update in detable(policy_map_no_class)!\n");
				return DIFFSERV_RETURN_CODE_ERROR;
			}
		}
		else
		{
			return CLASSMAP_RETURN_CODE_BIND_NUM_ERROR;
		}

		policy_map.class_map_index = -1;
		ret = dbtable_array_update(policy_map_index, policy_map.index, NULL, &policy_map);
		if(0 != ret)
		{
			syslog_ax_acl_err("Policy map cannot update in detable(policy_map_no_class)!\n");
			return DIFFSERV_RETURN_CODE_ERROR;
		}
	}
	return MATCH_RETURN_CODE_SUCCESS;
}

int service_policy_find_by_name_dir(const char* map_name, int dir, struct service_policy_s* service_policy)
{
	
    int ret = MATCH_RETURN_CODE_SUCCESS;
	struct service_policy_s find_policy ;

	if( map_name == NULL || service_policy == NULL )
	{
		return DIFFSERV_RETURN_CODE_ERROR;
	}
	memset(&find_policy, 0x0, sizeof(struct service_policy_s));
    strcpy(find_policy.policy_map_name, map_name);
	find_policy.dir_type = !dir;
    ret = dbtable_hash_search(service_policy_name, &find_policy, NULL, service_policy);
    if(0 == ret)
    {
        return SERVICEPOLICY_RETURN_CODE_DIR_ERROR;
    }
	find_policy.dir_type = dir;
   	ret = dbtable_hash_search(service_policy_name, &find_policy, NULL, service_policy);
	if (0 != ret)
	{
		return SERVICEPOLICY_RETURN_CODE_NOEXIST;
	}

	return MATCH_RETURN_CODE_SUCCESS;
}


int service_policy_create_bmp(char *policy_name, int dir, npd_pbmp_t portbmp)
{
    int 			ret = QOS_RETURN_CODE_SUCCESS;
    unsigned int 	id = 0;
    struct service_policy_s		service;
    struct policy_map_index_s	policy;
    struct class_map_index_s   class;
	QOS_PORT_CFG_STC			eth_flag;

	memset(&service, 0, sizeof(struct service_policy_s));
	memset(&policy, 0, sizeof(struct policy_map_index_s));
    memset(&class, 0, sizeof(struct class_map_index_s));
	memset(&eth_flag, 0, sizeof(QOS_PORT_CFG_STC));

	if (QOS_RETURN_CODE_SUCCESS == ret)
	{
		strcpy(policy.map_name, policy_name);
	    ret = dbtable_hash_search(policy_map_name, &policy, NULL, &policy);
	    if(0 != ret)
	    {
	        return POLICYMAP_RETURN_CODE_NOTEXIST;
	    }

        if(policy.class_map_index != -1)
            ret = dbtable_array_get(class_map_master, policy.class_map_index, &class);

	    strcpy(service.policy_map_name, policy_name);
        service.dir_type = !dir;
        ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
        if(0 == ret)
        {
            return SERVICEPOLICY_RETURN_CODE_DIR_ERROR;
        }
        
	    service.dir_type = dir;
        
	    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);

		if (0 != ret)
		{
			policy.is_deployed = 1;
			ret = dbtable_array_update(policy_map_index, policy.index, NULL, &policy);
			if (0 != ret)
		    {
		    	npd_syslog_acl_dbg("Policy map cannot update in detable(service_policy_create)!\n");
		        return DIFFSERV_RETURN_CODE_ERROR;
		    }

            if(-1 != policy.class_map_index)
            {
                class.is_deployed = 1;
                ret = dbtable_array_update(class_map_master, class.index, NULL, &class);
            }
			
		    service.service_policy_index = -1;
		    service.policy_index = policy.index;
		    NPD_PBMP_ASSIGN(service.group, portbmp);
		    ret = dbtable_array_insert_after(service_policy_index, &id, &service, CLASSMAP_SERVICE_INDEX_START);
		    
		    if (0 != ret)
		    {
		    	npd_syslog_acl_dbg("service policy cannot be insert to detable(service_policy_create)!\n");
		        return DIFFSERV_RETURN_CODE_ERROR;
		    }

		    service.service_policy_index = (int)id;
			service.dir_type = dir;
			
		    ret = dbtable_array_update(service_policy_index, id, NULL, &service);
		    if (0 != ret)
		    {
        	    syslog_ax_acl_err("service policy cannot update in detable(service_policy_create)!\n");
		        return DIFFSERV_RETURN_CODE_ERROR;
    		}
    	}
    	else
    	{
            if(policy.is_deployed != 1)
            {
    			policy.is_deployed = 1;
    			ret = dbtable_array_update(policy_map_index, policy.index, NULL, &policy);
    			if (0 != ret)
    		    {
    		    	npd_syslog_acl_dbg("Policy map cannot update in detable(service_policy_create)!\n");
    		        return DIFFSERV_RETURN_CODE_ERROR;
    		    }
    
                if(-1 != policy.class_map_index)
                {
                    class.is_deployed = 1;
                    ret = dbtable_array_update(class_map_master, class.index, NULL, &class);
                }
            }
			
			NPD_PBMP_OR(service.group, portbmp);
			ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
	        if(0 != ret)
	        {
	        	syslog_ax_acl_err("service policy cannot update in detable(service_policy_create)!\n");
		        return DIFFSERV_RETURN_CODE_ERROR;
			}
        }
	}
	else
	{
		return ret;
	}
	return MATCH_RETURN_CODE_SUCCESS;
}
int service_policy_create(char* policy_name, int dir, unsigned int netif_index)
{
    int 			ret = QOS_RETURN_CODE_SUCCESS;
    unsigned int 	id = 0;
    struct service_policy_s		service;
    struct policy_map_index_s	policy;
    struct class_map_index_s    class;
	QOS_PORT_CFG_STC			eth_flag;

	memset(&service, 0, sizeof(struct service_policy_s));
	memset(&policy, 0, sizeof(struct policy_map_index_s));
    memset(&class, 0, sizeof(struct class_map_index_s));
	memset(&eth_flag, 0, sizeof(QOS_PORT_CFG_STC));

    if((NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
        && !NPD_ACL_BASED_VLAN_SUPPORT)
        return SERVICE_POLICY_RETURN_ACL_BASED_VLAN_NOT_SUPPORT;
    
    if(NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
    {
        ret = npd_vlan_acl_vid_conflict_check(policy_name, netif_index);
        if(MATCH_RETURN_CODE_SUCCESS != ret)
            return ret;            
    }
    
	if (QOS_RETURN_CODE_SUCCESS == ret)
	{
		strcpy(policy.map_name, policy_name);
	    ret = dbtable_hash_search(policy_map_name, &policy, NULL, &policy);
	    if(0 != ret)
	    {
	        return POLICYMAP_RETURN_CODE_NOTEXIST;
	    }

        if(policy.class_map_index != -1)
            ret = dbtable_array_get(class_map_master, policy.class_map_index, &class);
        else
            return POLICYMAP_RETURN_CODE_CLASSMAP_NOTEXIST;
            
	    strcpy(service.policy_map_name, policy_name);
        service.dir_type = !dir;
        ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
        if(0 == ret)
        {
            return SERVICEPOLICY_RETURN_CODE_DIR_ERROR;
        }
        
	    service.dir_type = dir;
        
	    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);

		if (0 != ret)
		{
		    int array_id = 0;
            unsigned int vid = 0;
			policy.is_deployed = 1;
            unsigned int op_ret = 0;
			ret = dbtable_array_update(policy_map_index, policy.index, NULL, &policy);
			if (0 != ret)
		    {
		    	npd_syslog_acl_dbg("Policy map cannot update in detable(service_policy_create)!\n");
		        return DIFFSERV_RETURN_CODE_ERROR;
		    }

            if(-1 != policy.class_map_index)
            {
                class.is_deployed = 1;
                ret = dbtable_array_update(class_map_master, class.index, NULL, &class);
            }
			
		    service.service_policy_index = -1;
		    service.policy_index = policy.index;
            if(MATCH_RETURN_CODE_SUCCESS == npd_is_time_in_time_range(service.policy_map_name, &op_ret))
            {
                if(IN_TIME_RANGE == op_ret)
                {
                    service.time_range_set = IN_TIME_RANGE;
                }
            }
            switch(npd_netif_type_get(netif_index))
            {
                
                case NPD_NETIF_ETH_TYPE:
				case NPD_NETIF_TRUNK_TYPE:
                {
                    array_id = eth_port_array_index_from_ifindex(netif_index);
        		    NPD_PBMP_PORT_ADD(service.group, array_id);
                   
                    break ;
                }
                case NPD_NETIF_VLAN_TYPE:
                { 
                    
                    vid = npd_netif_vlan_get_vid(netif_index);
                    NPD_VBMP_VLAN_ADD(service.vlanbmp, vid);
                    
                    break ;
                }
                default :
                    return SERVICEPOLICY_RETURN_CODE_NOTSUPPORT;  
            }
			service.dir_type = dir;
            if(strncmp("ACL_", policy_name, 4) == 0)
            {
                id = strtoul(&service.policy_map_name[4], NULL, 10) + CLASSMAP_ACL_ID_INDEX_START;
                service.service_policy_index = id;
                ret = dbtable_array_insert_byid(service_policy_index, id, &service);
            }
            else if(strncmp("SG_", policy_name, 3) == 0)
            {
                 if(!strcmp(policy_name, SG_DEFAULT_DENY))
                {
                    id = CLASSMAP_SERVICE_SG_INDEX_START + SG_DEFAULT_DENY_ID;
                }
                else if(!strcmp(policy_name, SG_DEFAULT_PERMIT))
                {
                    id = CLASSMAP_SERVICE_SG_INDEX_START + SG_DEFAULT_PERMIT_ID;
                }
                else
                {
                    id = strtoul(&service.policy_map_name[3], NULL, 0) + CLASSMAP_SERVICE_SG_INDEX_START;
                }               
                service.service_policy_index = id;
                ret = dbtable_array_insert_byid(service_policy_index, id, &service);
            }
#ifdef HAVE_PORTAL            
            else if(strncmp("PORTAL_STA_", policy_name, 11) == 0)
            {
    		    ret = dbtable_array_insert_after(service_policy_index, &id, &service, CLASSMAP_SERVICE_PORTAL_STA_INDEX_START);
    		    if (0 != ret)
    		    {
    		    	npd_syslog_acl_dbg("service policy cannot be insert to detable(service_policy_create)!\n");
    		        return DIFFSERV_RETURN_CODE_ERROR;
    		    }
		        service.service_policy_index = id;
    		    ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
    		    if (0 != ret)
    		    {
            	    syslog_ax_acl_err("service policy cannot update in detable(service_policy_create)!\n");
    		        return DIFFSERV_RETURN_CODE_ERROR;
        		}
            }
            else if(strncmp("PORTAL_INTF_", policy_name, 12) == 0)
            {
    		    ret = dbtable_array_insert_after(service_policy_index, &id, &service, CLASSMAP_SERVICE_PORTAL_INTF_INDEX_START);
    		    if (0 != ret)
    		    {
    		    	npd_syslog_acl_dbg("service policy cannot be insert to detable(service_policy_create)!\n");
    		        return DIFFSERV_RETURN_CODE_ERROR;
    		    }
		        service.service_policy_index = id;
    		    ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
    		    if (0 != ret)
    		    {
            	    syslog_ax_acl_err("service policy cannot update in detable(service_policy_create)!\n");
    		        return DIFFSERV_RETURN_CODE_ERROR;
        		}
            }
#endif            
            else
            {
                struct service_policy_s tmp_service;
                memset(&tmp_service, 0, sizeof(struct service_policy_s));
    		    ret = dbtable_array_insert_after(service_policy_index, &id, &service, CLASSMAP_SERVICE_INDEX_START);
    		    if (0 != ret)
    		    {
    		    	npd_syslog_acl_dbg("service policy cannot be insert to detable(service_policy_create)!\n");
    		        return DIFFSERV_RETURN_CODE_ERROR;
    		    }
		        service.service_policy_index = id;
    		    ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
    		    if (0 != ret)
    		    {
            	    syslog_ax_acl_err("service policy cannot update in detable(service_policy_create)!\n");
    		        return DIFFSERV_RETURN_CODE_ERROR;
        		}
            }
			
    	}
    	else
    	{
    		switch(npd_netif_type_get(netif_index))
    		{
    			case NPD_NETIF_ETH_TYPE:
				case NPD_NETIF_TRUNK_TYPE:
    			{
                    int array_index = eth_port_array_index_from_ifindex(netif_index);

                    if(policy.is_deployed != 1)
                    {
            			policy.is_deployed = 1;
            			ret = dbtable_array_update(policy_map_index, policy.index, NULL, &policy);
            			if (0 != ret)
            		    {
            		    	npd_syslog_acl_dbg("Policy map cannot update in detable(service_policy_create)!\n");
            		        return DIFFSERV_RETURN_CODE_ERROR;
            		    }
            
                        if(-1 != policy.class_map_index)
                        {
                            class.is_deployed = 1;
                            ret = dbtable_array_update(class_map_master, class.index, NULL, &class);
                        }
                    }
                    if(NPD_PBMP_MEMBER(service.group, array_index))
                    {
                        return SERVICEPOLICY_RETURN_CODE_PORTEXIST;
                    }
    				NPD_PBMP_PORT_ADD(service.group, array_index);
    				ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
    		        if(0 != ret)
    		        {
    		        	syslog_ax_acl_err("service policy cannot update in detable(service_policy_create)!\n");
    			        return DIFFSERV_RETURN_CODE_ERROR;
    				}
    				break;
    			}
                case NPD_NETIF_VLAN_TYPE:
                {
                    unsigned int vid = npd_netif_vlan_get_vid(netif_index);
                    if(policy.is_deployed != 1)
                    {
            			policy.is_deployed = 1;
            			ret = dbtable_array_update(policy_map_index, policy.index, NULL, &policy);
            			if (0 != ret)
            		    {
            		    	npd_syslog_acl_dbg("Policy map cannot update in detable(service_policy_create)!\n");
            		        return DIFFSERV_RETURN_CODE_ERROR;
            		    }
            
                        if(-1 != policy.class_map_index)
                        {
                            class.is_deployed = 1;
                            ret = dbtable_array_update(class_map_master, class.index, NULL, &class);
                        }
                    }
                    if(NPD_VBMP_MEMBER(service.vlanbmp, vid))
                    {
                        return SERVICEPOLICY_RETURN_CODE_VLANEXIST;
                    }
    				NPD_VBMP_VLAN_ADD(service.vlanbmp, vid);
    				ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
    		        if(0 != ret)
    		        {
    		        	syslog_ax_acl_err("service policy cannot update in detable(service_policy_create)!\n");
    			        return DIFFSERV_RETURN_CODE_ERROR;
    				}
    				break; 
                }
    			default:
    			{
    				return SERVICEPOLICY_RETURN_CODE_NOTSUPPORT;
    			}
            }
        }
	}
	else
	{
		return ret;
	}
#if 0
	eth_flag.classPolicyService.inServicePolicyID = service.service_policy_index + 1;
	npd_qos_netif_cfg_set_by_index(netif_index, &eth_flag);
#endif
	return MATCH_RETURN_CODE_SUCCESS;
}


int service_policy_create_reserved(char* policy_name, int dir, unsigned int id)
{
    int 			ret = DIFFSERV_RETURN_CODE_ERROR;
    struct service_policy_s		service;
    struct policy_map_index_s	policy;

	memset(&service, 0, sizeof(struct service_policy_s));
	memset(&policy, 0, sizeof(struct policy_map_index_s));

	strcpy(policy.map_name, policy_name);
    ret = dbtable_hash_search(policy_map_name, &policy, NULL, &policy);
    if(0 != ret)
    {
        return POLICYMAP_RETURN_CODE_NOTEXIST;
    }
    
    strcpy(service.policy_map_name, policy_name);
    service.dir_type = dir;
    
    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);

	if (0 != ret)
	{
	    service.service_policy_index = id;
	    service.policy_index = policy.index;
        service.reserved = TRUE;
	    ret = dbtable_array_insert_byid(service_policy_index, id, &service);
	    
	    if (0 != ret)
	    {
	    	npd_syslog_acl_dbg("service policy cannot be insert to detable(service_policy_create)!\n");
	        return DIFFSERV_RETURN_CODE_ERROR;
	    }

	}
	return MATCH_RETURN_CODE_SUCCESS;
}

int service_policy_destroy(char* map_name, int dir, unsigned int netif_index)
{
    int 	ret = DIFFSERV_RETURN_CODE_ERROR;
    struct service_policy_s		service;
	struct policy_map_index_s	policy_name;
    struct class_map_index_s   class;
	QOS_PORT_CFG_STC			eth_flag;

    if((NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
        && !NPD_ACL_BASED_VLAN_SUPPORT)
        return SERVICE_POLICY_RETURN_ACL_BASED_VLAN_NOT_SUPPORT;
    

	memset(&service, 0, sizeof(struct service_policy_s));
	memset(&policy_name, 0, sizeof(struct policy_map_index_s));
	memset(&eth_flag, 0, sizeof(QOS_PORT_CFG_STC));

    strcpy(service.policy_map_name, map_name);
    service.dir_type = dir;
    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
    if(0 != ret)
        return SERVICEPOLICY_RETURN_CODE_NOEXIST;
    {
        switch(npd_netif_type_get(netif_index))
        {
            case NPD_NETIF_ETH_TYPE:
			case NPD_NETIF_TRUNK_TYPE:
			{
                int array_index = eth_port_array_index_from_ifindex(netif_index);
                if(!NPD_PBMP_MEMBER(service.group, array_index))
                {
                    return SERVICEPOLICY_RETURN_CODE_PORTNOTEXIST;
                }
                NPD_PBMP_PORT_REMOVE(service.group, array_index);
                ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
                if(0 != ret)
		        {
		            return DIFFSERV_RETURN_CODE_ERROR;
				}
				
                if(NPD_PBMP_IS_NULL(service.group) && NPD_VBMP_IS_NULL(service.vlanbmp))
                {
					strcpy(policy_name.map_name, map_name);
  					ret = dbtable_hash_search(policy_map_name, &policy_name, NULL, &policy_name);
					if(0 != ret)
			        {
			            return POLICYMAP_RETURN_CODE_NOTEXIST;
					}
					policy_name.is_deployed = 0;
					ret = dbtable_array_update(policy_map_index, policy_name.index, NULL, &policy_name);
					if(0 != ret)
			        {
			        	syslog_ax_acl_err("Policy map cannot update in detable(service_policy_destroy)!\n");
			            return DIFFSERV_RETURN_CODE_ERROR;
			        }

                    if(policy_name.class_map_index != -1)
                    {
                        dbtable_array_get(class_map_master, policy_name.class_map_index, &class);
                        class.is_deployed = 0;
                        dbtable_array_update(class_map_master, policy_name.class_map_index, NULL, &class);
                    }

                    if(service.reserved == FALSE)
                    {
                        ret = dbtable_array_delete(service_policy_index, service.service_policy_index, &service);
    					if(0 != ret)
    			        {
    			        	syslog_ax_acl_err("service policy cannot remove from detable(service_policy_destroy)!\n");
    			            return DIFFSERV_RETURN_CODE_ERROR;
    					}
                    }
                }
                break;
            }
            case NPD_NETIF_VLAN_TYPE:
            {
                unsigned int vid = npd_netif_vlan_get_vid(netif_index);
                if(!NPD_VBMP_MEMBER(service.vlanbmp, vid))
                {
                    return SERVICEPOLICY_RETURN_CODE_VLANNOTEXIST;
                }
                NPD_VBMP_VLAN_REMOVE(service.vlanbmp, vid);
                ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
                if(0 != ret)
		        {
		            return DIFFSERV_RETURN_CODE_ERROR;
				}
				
                if(NPD_PBMP_IS_NULL(service.group) && NPD_VBMP_IS_NULL(service.vlanbmp))
                {
					strcpy(policy_name.map_name, map_name);
  					ret = dbtable_hash_search(policy_map_name, &policy_name, NULL, &policy_name);
					if(0 != ret)
			        {
			            return POLICYMAP_RETURN_CODE_NOTEXIST;
					}
					policy_name.is_deployed = 0;
					ret = dbtable_array_update(policy_map_index, policy_name.index, NULL, &policy_name);
					if(0 != ret)
			        {
			        	syslog_ax_acl_err("Policy map cannot update in detable(service_policy_destroy)!\n");
			            return DIFFSERV_RETURN_CODE_ERROR;
			        }

                    if(policy_name.class_map_index != -1)
                    {
                        dbtable_array_get(class_map_master, policy_name.class_map_index, &class);
                        class.is_deployed = 0;
                        dbtable_array_update(class_map_master, policy_name.class_map_index, NULL, &class);
                    }

                    if(service.reserved == FALSE)
                    {
                        ret = dbtable_array_delete(service_policy_index, service.service_policy_index, &service);
    					if(0 != ret)
    			        {
    			        	syslog_ax_acl_err("service policy cannot remove from detable(service_policy_destroy)!\n");
    			            return DIFFSERV_RETURN_CODE_ERROR;
    					}
                    }
                }
                break;
            }
            default:
			{
			    return POLICYMAP_RETURN_CODE_NOSUPPORT;
            }
        }
    }
#if 0
	eth_flag.classPolicyService.inServicePolicyID = 0;
	npd_qos_netif_cfg_set_by_index(netif_index, &eth_flag);
#endif	
	return MATCH_RETURN_CODE_SUCCESS;
}


int service_policy_destroy_bmp(char* map_name, int dir, npd_pbmp_t portbmp)
{
    int 	ret = DIFFSERV_RETURN_CODE_ERROR;
    struct service_policy_s		service;
	struct policy_map_index_s	policy_name;
    struct class_map_index_s   class;
	QOS_PORT_CFG_STC			eth_flag;
	npd_pbmp_t applybmp;
    

	memset(&service, 0, sizeof(struct service_policy_s));
	memset(&policy_name, 0, sizeof(struct policy_map_index_s));
	memset(&eth_flag, 0, sizeof(QOS_PORT_CFG_STC));

    strcpy(service.policy_map_name, map_name);
    service.dir_type = dir;
    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
    if(0 != ret) {
        return SERVICEPOLICY_RETURN_CODE_NOEXIST;
    }
	else
	{
		NPD_PBMP_ASSIGN(applybmp, portbmp);
		NPD_PBMP_AND(applybmp, service.group);
		
        NPD_PBMP_XOR(service.group, applybmp);
        ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
		if(0 != ret)
        {
            return DIFFSERV_RETURN_CODE_ERROR;
		}
		
		if(NPD_PBMP_IS_NULL(service.group)) 
		{
			strcpy(policy_name.map_name, map_name);
			ret = dbtable_hash_search(policy_map_name, &policy_name, NULL, &policy_name);
			if(0 != ret)
			{
				return POLICYMAP_RETURN_CODE_NOTEXIST;
			}
			policy_name.is_deployed = 0;
			ret = dbtable_array_update(policy_map_index, policy_name.index, NULL, &policy_name);
			if(0 != ret)
			{
				syslog_ax_acl_err("Policy map cannot update in detable(service_policy_destroy)!\n");
				return DIFFSERV_RETURN_CODE_ERROR;
			}

			if(policy_name.class_map_index != -1)
			{
				dbtable_array_get(class_map_master, policy_name.class_map_index, &class);
				class.is_deployed = 0;
				dbtable_array_update(class_map_master, policy_name.class_map_index, NULL, &class);
			}

			if(service.reserved == FALSE)
			{
				ret = dbtable_array_delete(service_policy_index, service.service_policy_index, &service);
				if(0 != ret)
				{
					syslog_ax_acl_err("service policy cannot remove from detable(service_policy_destroy)!\n");
					return DIFFSERV_RETURN_CODE_ERROR;
				}
			}
		}
	}
	
	return MATCH_RETURN_CODE_SUCCESS;
}

long service_policy_handle_insert(void* new)
{
	char	lookupPhase = 0;
	int		ret = 0;
	struct service_policy_s*	entry = (struct service_policy_s *)new;
	struct policy_map_rule_s	policy;
	struct policy_map_index_s	policy_index;
	struct class_map_rule_s		class_rule;
    unsigned int                op_ret = 0;
	if (-1 == entry->service_policy_index)
	{
		return -1;
	}
	
	memset(&policy, 0, sizeof(struct policy_map_rule_s));
	memset(&policy_index, 0, sizeof(struct policy_map_index_s));
	memset(&class_rule, 0, sizeof(struct class_map_rule_s));

	strcpy(policy.map_name, entry->policy_map_name);
	policy.index = entry->policy_index;

	strcpy(policy_index.map_name, entry->policy_map_name); 
    if(MATCH_RETURN_CODE_SUCCESS == npd_is_time_in_time_range(policy.map_name, &op_ret)
        && (OUT_TIME_RANGE == entry->time_range_set))
    {
            return 0;
    }


	ret = dbtable_hash_search(policy_map_name, &policy_index, NULL, &policy_index);
	if (0 != ret)
	{
		return -1;
	}

	class_rule.index = policy_index.class_map_index;
	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
	while( 0 == ret )
	{
		rule_cmd_t*	cmd = NULL;
		
		char tempphase;
       
		cmd = class_map_lookup_match(class_rule.cmd_name);
		if (NULL != cmd)
		{
			if (NULL != cmd->func_phase)
			{
				tempphase = (*cmd->func_phase)(entry->service_policy_index, &class_rule);
			}
			else
				tempphase = class_rule.lk_phase;
		}

        if((0 == lookupPhase) && (tempphase != (ACL_PHASE2_E|ACL_PHASE1_E))
			&& (tempphase != ACL_PHASE12_E))
            lookupPhase = tempphase;
		else if(tempphase == ACL_PHASE12_E)
			lookupPhase = (ACL_PHASE2_E|ACL_PHASE1_E);
        else if(lookupPhase != (lookupPhase & tempphase))
		     lookupPhase |= tempphase;
		ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
	}

	if(TRUE == npd_class_map_have_dst_src_ip6(policy_index.class_map_index))
	{
        if(entry->dir_type == ACL_DIRECTION_INGRESS_E)
        {
            lookupPhase = ACL_PHASEIP6_IN_E; 
        }
        if(entry->dir_type == ACL_DIRECTION_EGRESS_E)
        {
            lookupPhase = ACL_PHASEIP6_EG_E;
        }
    }

	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
	/* if it contain some rules? */
	if (0 == ret)
	{		
        if(lookupPhase == 0)
            lookupPhase = ACL_PHASE2_E;
        if(NPD_PBMP_NOT_NULL(entry->group))
		    ret = (*qos_hw_rule_create)(entry->service_policy_index, entry->dir_type, &(entry->group), lookupPhase);
        else if(NPD_VBMP_NOT_NULL(entry->vlanbmp))
        {
            ret = (*qos_hw_rule_vlan_create)(entry->service_policy_index, entry->dir_type, &(entry->vlanbmp), lookupPhase);
        }
        while (0 == ret)			/* match  */
		{
			rule_cmd_t*	cmd = NULL;

			cmd = class_map_lookup_match(class_rule.cmd_name);
			if (NULL != cmd)
			{
				if (NULL != cmd->func_apply)
				{
					(*cmd->func_apply)(entry->service_policy_index, &class_rule);
				}
			}
		
			ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
		}

		ret = dbtable_hash_head_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
		while (0 == ret)
		{
			rule_cmd_t *cmd = NULL;

			cmd = policy_map_lookup_set(policy.cmd_name);
			if (NULL != cmd)
			{
				if (cmd->func_apply)
				{
					(*cmd->func_apply)(entry->service_policy_index, &policy);
				}
			}
			
			ret = dbtable_hash_next_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
		}
		if(entry->dir_type == ACL_DIRECTION_INGRESS_E)
		    ret = (*qos_hw_rule_install_entry)(entry->service_policy_index, lookupPhase);
        if(entry->dir_type == ACL_DIRECTION_EGRESS_E)
        {
            if(lookupPhase == ACL_PHASEIP6_EG_E)
            {
     		    ret = (*qos_hw_rule_install_entry)(entry->service_policy_index, ACL_PHASEIP6_EG_E);
            }
            else
            {
                ret = (*qos_hw_rule_install_entry)(entry->service_policy_index, ACL_PHASE3_E);
            }
        }
           
	}
	else
	{
		return -1;
	}

	return ret;
}

long service_policy_handle_update(void *new, void *old)
{
	char	lookupPhase = 0;
	int		intf_flag = FALSE;
	int		ret = 0;
    char    time_range_name[16] = {0};
	struct service_policy_s*	entryNew = (struct service_policy_s *)new;
	struct service_policy_s*	entryOld = (struct service_policy_s *)old;
	struct policy_map_rule_s	policy;
	struct policy_map_index_s	policy_index;
	struct class_map_rule_s		class_rule;
    unsigned int op_ret = 0;
    int process_switch = -1;     

	memset(&policy, 0, sizeof(struct policy_map_rule_s));
	memset(&policy_index, 0, sizeof(struct policy_map_index_s));
	memset(&class_rule, 0, sizeof(struct class_map_rule_s));

	strcpy(policy.map_name, entryNew->policy_map_name);
	policy.index = entryNew->policy_index;

    /*decide whether the time range is matched, or the time range is IN_TIME_RANGE*/
    if(TIME_RANGE_RETURN_BIND_WITH_TIME_RANGE == npd_is_acl_associate_time_range_info(entryNew->policy_map_name, time_range_name))
    {
        int flag = 0; 
        if((NPD_PBMP_NEQ(entryNew->group, entryOld->group))
            || (NPD_VBMP_NEQ(entryNew->vlanbmp, entryOld->vlanbmp)))
        {
            flag = 1;           
        }
        npd_is_time_in_time_range(entryNew->policy_map_name, &op_ret);
        if(OUT_TIME_RANGE == op_ret)
        {
            struct service_policy_s service;
            memset(&service, 0, sizeof(struct service_policy_s));

            strncpy(service.policy_map_name, entryNew->policy_map_name, 32);
            service.dir_type = entryNew->dir_type;
            op_ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
            ret = (*qos_hw_rule_destroy)(service.service_policy_index, service.dir_type, NULL);
            return ret;
        }
        if(IN_TIME_RANGE == op_ret && flag == 1)
        {
            process_switch = 1;
        }
        if(IN_TIME_RANGE == op_ret && flag == 0)
        {
            process_switch = 0;
        }
    }
	strcpy(policy_index.map_name, entryNew->policy_map_name);
	ret = dbtable_hash_search(policy_map_name, &policy_index, NULL, &policy_index);
	if(0 != ret)
	{
		return -1;
	}
    
	class_rule.index = policy_index.class_map_index;
    {
    	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
    	while( 0 == ret )
    	{
    		rule_cmd_t*	cmd = NULL;
    		char tempphase;
           
    		cmd = class_map_lookup_match(class_rule.cmd_name);
    		if (NULL != cmd)
    		{
    			if (NULL != cmd->func_phase)
    			{
    				tempphase = (*cmd->func_phase)(entryNew->service_policy_index, &class_rule);
    			}
    			else
    				tempphase = class_rule.lk_phase;
    		}
    
            if((0 == lookupPhase) && (tempphase != (ACL_PHASE2_E|ACL_PHASE1_E))
    			&& (tempphase != ACL_PHASE12_E))
                lookupPhase = tempphase;
    		else if(tempphase == ACL_PHASE12_E)
    			lookupPhase = (ACL_PHASE2_E|ACL_PHASE1_E);
            else if(lookupPhase != (lookupPhase & tempphase))
    		     lookupPhase |= tempphase;

    		ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
    	}
    }
	npd_syslog_dbg("service policy update final lookupPhase %x", lookupPhase);
	if(TRUE == npd_class_map_have_dst_src_ip6(policy_index.class_map_index))
	{
        if(entryNew->dir_type == ACL_DIRECTION_INGRESS_E)
        {
            lookupPhase = ACL_PHASEIP6_IN_E; 
        }
        if(entryNew->dir_type == ACL_DIRECTION_EGRESS_E)
        {
            lookupPhase = ACL_PHASEIP6_EG_E;
        }
    }
	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
	/* if it contain some rules? */
    if(0 == ret)
	{
		npd_pbmp_t diff;
        npd_vbmp_t diffvlan;
        int port_error_ret = 0;
        int vlan_error_ret = 0;
		NPD_PBMP_ASSIGN(diff, entryOld->group);
        NPD_VBMP_ASSIGN(diffvlan, entryOld->vlanbmp);
        
        if(((-1 != entryNew->service_policy_index) && (-1 == entryOld->service_policy_index))
                || ((NPD_PBMP_IS_NULL(entryOld->group) && !NPD_PBMP_IS_NULL(entryNew->group)))
                || (0 == process_switch))
		{
            if(!NPD_PBMP_IS_NULL(entryNew->group))
            {
                if(lookupPhase == 0)
                    lookupPhase = ACL_PHASE2_E;
    			if(qos_hw_rule_create == NULL)
    				return ret;
    			ret = (*qos_hw_rule_create)(entryNew->service_policy_index, entryNew->dir_type, &(entryNew->group), lookupPhase);

    			while(0 == ret)
    			{
    				rule_cmd_t*	cmd = NULL;

    				cmd = class_map_lookup_match(class_rule.cmd_name);
    				if(NULL != cmd)
    				{
    					if(NULL != cmd->func_apply)
    					{
    						(*cmd->func_apply) (entryNew->service_policy_index, &class_rule);
    					}
    				}

    				ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
    			}

    			ret = dbtable_hash_head_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
    			while(0 == ret)
    			{
    				rule_cmd_t*	cmd = NULL;

    				cmd = policy_map_lookup_set(policy.cmd_name);
    				if(NULL != cmd)
    				{
    					if(NULL != cmd->func_apply)
    					{
    						(*cmd->func_apply)(entryNew->service_policy_index, &policy);
    					}
    				}
    				
    				ret = dbtable_hash_next_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
    			}
    			
        		if(entryNew->dir_type == ACL_DIRECTION_INGRESS_E)
        		    ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, lookupPhase);
                if(entryNew->dir_type == ACL_DIRECTION_EGRESS_E)
                {
                    if(lookupPhase == ACL_PHASEIP6_EG_E)
                    {
             		    ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASEIP6_EG_E);
                    }
                    else
                    {
                        ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASE3_E);
                    }
                }
            }
			
		}
		else if((!NPD_PBMP_EQ(diff, entryNew->group))
                || (1 == process_switch) )
		{
			int array_id = 0;
			unsigned int netif_index = 0;
			NPD_PBMP_XOR(diff, entryNew->group);
			NPD_PBMP_ITER(diff, array_id)
			{
				netif_index = eth_port_array_index_to_ifindex(array_id);
				if(NPD_PBMP_MEMBER(entryNew->group, array_id))
				{
					intf_flag = (*qos_hw_rule_add_intf)(entryNew->service_policy_index, entryNew->dir_type, netif_index, lookupPhase);
					if (0 == intf_flag)
					{
						ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
    					while(0 == ret)
    					{					
    						rule_cmd_t*	cmd = NULL;

    						cmd = class_map_lookup_match(class_rule.cmd_name);
    						if(NULL != cmd)
    						{
    							if(NULL != cmd->func_apply)
    							{
    								(*cmd->func_apply) (entryNew->service_policy_index, &class_rule);
    							}
    						}

    						ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
    					}

    					ret = dbtable_hash_head_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
    					while(0 == ret)
    					{
    						rule_cmd_t*	cmd = NULL;

    						cmd = policy_map_lookup_set(policy.cmd_name);
    						if(NULL != cmd)
    						{
    							if(NULL != cmd->func_apply)
    							{
    								(*cmd->func_apply)(entryNew->service_policy_index, &policy);
    							}
    						}
    						
    						ret = dbtable_hash_next_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
    					}
						
                		if(entryNew->dir_type == ACL_DIRECTION_INGRESS_E)
                		    ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, lookupPhase);
                        if(entryNew->dir_type == ACL_DIRECTION_EGRESS_E)
                        {
                            if(lookupPhase == ACL_PHASEIP6_EG_E)
                            {
                 		        ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASEIP6_EG_E);
                            }
                            else
                            {
                                ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASE3_E);
                            }
                        }
    				}
				}
				else
				{
                    ret = (*qos_hw_rule_del_intf)(entryNew->service_policy_index, entryNew->dir_type, netif_index, lookupPhase); 
                }
			}
		}
		else
		{
			port_error_ret = -1;
		}
        if(NPD_ACL_BASED_VLAN_SUPPORT)
        {
            if(((-1 != entryNew->service_policy_index) && (-1 == entryOld->service_policy_index))
                    || ((NPD_VBMP_IS_NULL(entryOld->vlanbmp) && !NPD_VBMP_IS_NULL(entryNew->vlanbmp)))
                    || (0 == process_switch))
            {
                if((!NPD_VBMP_IS_NULL(entryNew->vlanbmp)) && (NULL != qos_hw_rule_vlan_create))
                {
                    ret = (*qos_hw_rule_vlan_create)(entryNew->service_policy_index, entryNew->dir_type, &(entryNew->vlanbmp), lookupPhase);
                    while(0 == ret)
        			{
        				rule_cmd_t*	cmd = NULL;
        				cmd = class_map_lookup_match(class_rule.cmd_name);
        				if(NULL != cmd)
        				{
                            if(NULL != cmd->func_apply)
        					{
                                (*cmd->func_apply) (entryNew->service_policy_index, &class_rule);
        					}
        				}
                        ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
                    }
                    ret = dbtable_hash_head_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
        			while(0 == ret)
        			{
        				rule_cmd_t*	cmd = NULL;
        				cmd = policy_map_lookup_set(policy.cmd_name);
        				if(NULL != cmd)
        				{
        					if(NULL != cmd->func_apply)
        					{
        						(*cmd->func_apply)(entryNew->service_policy_index, &policy);
        					}
        				}
                        ret = dbtable_hash_next_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
        			}
            		if(entryNew->dir_type == ACL_DIRECTION_INGRESS_E)
                        ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, lookupPhase);
                    if(entryNew->dir_type == ACL_DIRECTION_EGRESS_E)
                    {
                        if(ACL_PHASEIP6_EG_E == lookupPhase)
                        {
                            ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASEIP6_EG_E);
                        }
                        else
                        {
                            ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASE3_E);
                        }
                    }
                }
            }
            else if((!NPD_VBMP_EQ(diffvlan, entryNew->vlanbmp))
                    || (1 == process_switch) )
            {
                int vid = 2;

                NPD_VBMP_XOR(diffvlan, entryNew->vlanbmp);
                NPD_VBMP_ITER(diffvlan, vid)
                {
                    if(NPD_VBMP_MEMBER(entryNew->vlanbmp, vid))
                    {

                        intf_flag = (*qos_hw_rule_add_vlan)(entryNew->service_policy_index, entryNew->dir_type, vid, lookupPhase);
                        if(0 == intf_flag)
                        {
    						ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
        					while(0 == ret)
        					{					
        						rule_cmd_t*	cmd = NULL;
        						cmd = class_map_lookup_match(class_rule.cmd_name);
        						if(NULL != cmd)
        						{
                                    if(NULL != cmd->func_apply)
        							{
        								(*cmd->func_apply) (entryNew->service_policy_index, &class_rule);
        							}
        						}
        						ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
                            }
                            ret = dbtable_hash_head_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
        					while(0 == ret)
        					{
        						rule_cmd_t*	cmd = NULL;
        						cmd = policy_map_lookup_set(policy.cmd_name);
        						if(NULL != cmd)
        						{
                                    if(NULL != cmd->func_apply)
        							{
        								(*cmd->func_apply)(entryNew->service_policy_index, &policy);
        							}
        						}
        						ret = dbtable_hash_next_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
                            }
                            if(entryNew->dir_type == ACL_DIRECTION_INGRESS_E)
                    	        ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, lookupPhase);
                            if(entryNew->dir_type == ACL_DIRECTION_EGRESS_E)
                            {
                                if(ACL_PHASEIP6_EG_E == lookupPhase)
                                {
                                    ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASEIP6_EG_E); 
                                }
                                else
                                {
                                    ret = (*qos_hw_rule_install_entry)(entryNew->service_policy_index, ACL_PHASE3_E);
                                }
                            }
                        }
                    }
                    else
                    {
                        ret = (*qos_hw_rule_del_vlan)(entryNew->service_policy_index, entryNew->dir_type, vid, lookupPhase);
                    }
                }
            }
            else
            {
                vlan_error_ret = -1;
            }
        }

        if((-1 == port_error_ret) && (0 == (vlan_error_ret + NPD_ACL_BASED_VLAN_SUPPORT)))
            return -1;
        else
            return 0;

        
	}

	return ret;
}

long service_policy_handle_delete(void *new)
{
	int ret = 0;

    struct service_policy_s *entry = (struct service_policy_s *)new;
    if(qos_hw_rule_destroy == NULL)
		return ret;
    ret = (*qos_hw_rule_destroy)(entry->service_policy_index, entry->dir_type, &(entry->group));

	return ret;
}
#ifdef HAVE_ROUTE
long policy_route_nexthop_update(void *new, void *old)
{
	struct service_policy_route_s *item = (struct service_policy_route_s*)new;
	struct service_policy_s entry;
	int ret = 0;
    if(qos_hw_rule_install_entry == NULL)
		return 0;
    memset(&entry, 0, sizeof(struct service_policy_s));
	strcpy(entry.policy_map_name, item->policy_map_name);
	entry.dir_type = ACL_DIRECTION_INGRESS_E;
	ret = dbtable_hash_search(service_policy_name, &entry, NULL, &entry);
	if(0 == ret)
	{
    	struct policy_map_rule_s	policy;
    
    	if (-1 == entry.service_policy_index)
    	{
    		return 0;
    	}
    	
    	memset(&policy, 0, sizeof(struct policy_map_rule_s));
    	strcpy(policy.map_name, entry.policy_map_name);
    	policy.index = entry.policy_index;
    	{		
    
    		ret = dbtable_hash_head_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
    		while (0 == ret)
    		{
    			rule_cmd_t *cmd = NULL;
				
				if(strcmp(policy.cmd_name, "policy-route"))
				{
                    ret = dbtable_hash_next_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
					continue;
				}
    
    			cmd = policy_map_lookup_set(policy.cmd_name);
    			if (NULL != cmd)
    			{
    				if (cmd->func_apply)
    				{
    					(*cmd->func_apply)(entry.service_policy_index, &policy);
    				}
					break;
    			}
    			
    			ret = dbtable_hash_next_key(policy_map_set_rule, &policy, &policy, policy_map_rule_filter);
    		}
   		    ret = (*qos_hw_rule_install_entry)(entry.service_policy_index, ACL_PHASE2_E);
    	}		
	}
	return 0;
}
long policy_route_nexthop_insert(void *new)
{
	return policy_route_nexthop_update(new, NULL);
}
long policy_route_nexthop_delete(void *new, void *old)
{
	return 0;
}
#endif
int showruning_l4port_range_fun(const char* cmd_name, const char *cmd_arg ,char *cursor)
{  
    char            index_string[L4_PORT_RANGE_OP_TOTOAL][4];
    int             port_range_index[L4_PORT_RANGE_OP_TOTOAL];
    struct l4port_range_s port_range[L4_PORT_RANGE_OP_TOTOAL];
    int     totalLen = 0;
    char    namestr[32];
    int     uni = 0;
    int     ret = 0;
    char *  endptr = NULL;
    
    
    if((NULL == cmd_arg) || (NULL == cursor) || (NULL == cmd_name))
        return totalLen;

    memset(index_string, -1, sizeof(index_string));
    memset(port_range_index, -1, sizeof(port_range_index));
    memset(port_range, 0, sizeof(port_range));
    memset(namestr, 0, sizeof(namestr));

    if(0 == strcmp(cmd_name, "l4port_range_src"))
    {
        sprintf(namestr, "srcport");
    }
    else if(0 == strcmp(cmd_name, "l4port_range_dst"))
    {
        sprintf(namestr, "dstport");
    }
    
    for(uni = 0; uni < L4_PORT_RANGE_OP_TOTOAL; uni ++)
    {
        memcpy(index_string[uni], &cmd_arg[uni * 4], 4);
        port_range_index[uni] = strtol(index_string[uni], &endptr, 10);
        if(-1 != port_range_index[uni])
        {
            ret = dbtable_array_get(l4port_range_master, port_range_index[uni], &port_range[uni]);
			if(0 != ret)
				port_range_index[uni] = -1;
		}
    }

    if((-1 != port_range_index[0])
        && (-1 != port_range_index[1])
        && (port_range[0].protocol == port_range[1].protocol))
    {

        if(TCP_PROTOCOL == port_range[0].protocol)
            totalLen = sprintf(cursor, "tcp %s range %d %d\n", namestr,
                        port_range[0].l4port, port_range[1].l4port);
        else
            totalLen = sprintf(cursor, "udp %s range %d %d\n", namestr, 
                        port_range[0].l4port, port_range[1].l4port);

        return totalLen;
    }

    for(uni = 0; uni < L4_PORT_RANGE_OP_TOTOAL; uni++)
    {
        if(-1 != port_range_index[uni])
        {
            if(TCP_PROTOCOL == port_range[uni].protocol)
            {
                if(LITTLE_THAN == port_range[uni].operation)
                {
                    totalLen = sprintf(cursor, "tcp %s lt %d\n", namestr,
                        port_range[uni].l4port);
                }
                else if(GREAT_THAN == port_range[uni].operation)
                {
                    totalLen = sprintf(cursor, "tcp %s gt %d\n", namestr,
                        port_range[uni].l4port);
                }
                else if(NOT_EQUAL == port_range[uni].operation)
                {
                    totalLen = sprintf(cursor, "tcp %s neq %d\n", namestr,
                        port_range[uni].l4port);
                }
            }
            else
            {
                if(LITTLE_THAN == port_range[uni].operation)
                {
                    totalLen = sprintf(cursor, "udp %s lt %d\n", namestr,
                        port_range[uni].l4port);
                }
                else if(GREAT_THAN == port_range[uni].operation)
                {
                    totalLen = sprintf(cursor, "udp %s lt %d\n", namestr,
                        port_range[uni].l4port);
                }
                else if(NOT_EQUAL == port_range[uni].operation)
                {
                    totalLen = sprintf(cursor, "udp %s lt %d\n", namestr,
                        port_range[uni].l4port);
                }
            }
        }
    }

    return totalLen;
}

int showrunning_name_cmp_fun(preset_arg_t node, char** cursor, unsigned int data)
{
	char	datatostr[16] = {0};
	int		ni = 0;
	int		totalLen = 0;
	unsigned int	value = data;

	memset(datatostr, 0, 16);
	sprintf(datatostr, "%d", value);
						
	for (ni = 0; ni < node.preset_value_num; ni++)
	{
		if (0 == strcmp(node.value[ni]->preset_value, datatostr))
		{
			totalLen = sprintf(*cursor, " %s\n", node.value[ni]->preset_value_name);
			break;
		}
	}

	if (ni >= node.preset_value_num)
	{
		totalLen = sprintf(*cursor, " %u\n", value);
	}

	return totalLen;
}

int show_name_cmp_fun(preset_arg_t node, char** cp_info, unsigned int data)
{
	char	datatostr[16] = {0};
	int		ni = 0;
	unsigned int	value = data;

	memset(datatostr, 0, 16);
	sprintf(datatostr, "%d", value);
						
	for (ni = 0; ni < node.preset_value_num; ni++)
	{
		if (0 == strcmp(node.value[ni]->preset_value, datatostr))
		{
			sprintf(*cp_info, "%s", node.value[ni]->preset_value_name);
			break;
		}
	}

	if (ni >= node.preset_value_num)
	{
		sprintf(*cp_info, "%u", value);
	}

	return 0;
}
long l4port_range_handle_insert(void* new)
{
    struct l4port_range_s *ps = (struct l4port_range_s *)new;
    int ret = 0;
	
    if(TCP_PROTOCOL == ps->protocol)
    {
        npd_tcp_cmp_used ++;
    }
    else if(UDP_PROTOCOL == ps->protocol)
    {
        npd_udp_cmp_used ++;
    }
    else
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }
	if(NULL != qos_hw_l4port_range_alloc)
    	ret = (*qos_hw_l4port_range_alloc)(ps->index, ps->protocol);
    return ret;
}

long l4port_range_handle_delete(void* new)
{
    struct l4port_range_s *ps = (struct l4port_range_s *)new;
    int ret = 0;
    if(TCP_PROTOCOL == ps->protocol)
    {
        npd_tcp_cmp_used --;
    }
    else if(UDP_PROTOCOL == ps->protocol)
    {
        npd_udp_cmp_used --;
    }
    else
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }
	if(NULL != qos_hw_l4port_range_free)
		ret = (*qos_hw_l4port_range_free)(ps->index, ps->protocol);
        
    return ret;
}

int npd_policy_route_update_by_nhp(unsigned int ipAddr, int valid)
{
	struct service_policy_route_s temp_item;
	
	int tmp_ret = -1;
	memset(&temp_item, 0, sizeof(temp_item));

	temp_item.nexthopv4 = ipAddr;
	tmp_ret = dbtable_hash_head_key(ser_policy_pb_hash, &temp_item, &temp_item, NULL);
	while(0 == tmp_ret)	
	{		
		temp_item.change_coount++;		
		dbtable_hash_update(ser_policy_pb_hash, NULL, &temp_item);						
		tmp_ret = dbtable_hash_next_key(ser_policy_pb_hash, &temp_item, &temp_item, NULL);
	}

	return tmp_ret;
}

unsigned int acl_group_create(const char * group_name)
{
    int ret = 0;
    int totalcount = 0;
    unsigned int uni = 0;
    struct acl_group_stc acl_group;
    struct acl_group_stc tmp_group;

    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 == ret)
    {
        if(1 == acl_group.is_deployed)
        {
            return DIFFSERV_RETURN_CODE_ACL_GROUP_CONFIG_FAILE;
        }        
    }
    else
    {
        totalcount = dbtable_array_totalcount(acl_group_index);
        for(uni = 0; uni < totalcount; uni++)
        {
            memset(&tmp_group, 0, sizeof(struct acl_group_stc));
            ret = dbtable_array_get(acl_group_index, uni, &tmp_group);
            if(0 != ret)
                break;
        }
        acl_group.group_index = (int)uni;
        ret = dbtable_array_insert_byid(acl_group_index, uni, &acl_group);
        if(0 != ret)
        {
            return DIFFSERV_RETURN_CODE_ACL_GROUP_ITEM_FULL;
        }
    }

    return MATCH_RETURN_CODE_SUCCESS;  
}

unsigned int acl_group_delete(const char * group_name)
{
    int ret = 0;
    unsigned int uni = 0;
    struct acl_group_stc acl_group;
    struct policy_map_index_s policy_map;

    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 == ret)
    {
        if(1 == acl_group.is_deployed)
        {
            return DIFFSERV_RETURN_CODE_ACL_GROUP_DEPLOYED;
        }
        for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
        {
            if(0 == acl_group.acl_index[uni])
            {
                continue;
            }
            memset(&policy_map, 0, sizeof(struct policy_map_index_s));
            ret = dbtable_array_get(policy_map_index, acl_group.acl_index[uni], &policy_map);
            memset(policy_map.acl_group, 0, sizeof(policy_map.map_name));
            ret = dbtable_array_update(policy_map_index, acl_group.acl_index[uni], &policy_map, &policy_map);
        }
        ret = dbtable_array_delete(acl_group_index, acl_group.group_index, &acl_group);
    }
    else
    {
        return DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }

    return MATCH_RETURN_CODE_SUCCESS;      
}
unsigned int acl_group_add_rule(const char * group_name, const char *map_name)
{
    int ret = 0;
    unsigned int uni = 0;
    unsigned int id = 0;
    struct acl_group_stc acl_group;
    struct policy_map_index_s map;

	memset(&map, 0, sizeof(struct policy_map_index_s));
    strncpy(map.map_name, map_name, 32);

    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 != ret)
    {
        return DIFFSERV_RETURN_CODE_ACL_GROUP_CONFIG_FAILE;
    }

    if(strncmp("ACL_", map_name, 4) == 0)
    {
        id = strtoul(&map.map_name[4], NULL, 0) + CLASSMAP_ACL_ID_INDEX_START;
        map.index = id;
        ret = dbtable_array_get(policy_map_index, id, &map);
        if(ret != 0)
        {
            return DIFFSERV_RETURN_ACL_GROUP_RULE_NOTEXIST;
        }

        if(1 == map.is_deployed)
        {
            return POLICYMAP_RETURN_CODE_DEPLOYED;
        }
        
        if(strlen(map.acl_group) > 0)
        {
            return POLICYMAP_RETURN_CODE_BIND_ACL_GROUP;
        }
        
        for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
        {
            if(id == acl_group.acl_index[uni])
            {
                return DIFFSERV_RETURN_ACL_GROUP_RULE_EXIST;
                
            }
        }
        for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
        {
            if(0 == acl_group.acl_index[uni])
            {
                break;
            }
        }

        if(ACL_GROUP_MAX_RULES == uni)
        {
            return DIFFSERV_RETURN_ACL_GROUP_RULE_FULL;
        }
        
        acl_group.acl_index[uni] = id;
        ret = dbtable_array_update(acl_group_index, acl_group.group_index, NULL, &acl_group);

        strncpy(map.acl_group, group_name, 32);
        ret = dbtable_array_update(policy_map_index, id, &map, &map);
    }

    return MATCH_RETURN_CODE_SUCCESS;      
}
unsigned int acl_group_delete_rule(const char * group_name, const char *map_name)
{
    int ret = 0;
    unsigned int uni = 0;
    unsigned int id = 0;
    struct acl_group_stc acl_group;
    struct policy_map_index_s map;

	memset(&map, 0, sizeof(struct policy_map_index_s));
    strncpy(map.map_name, map_name, 32);

    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 != ret)
    {
        return DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }

    if(strncmp("ACL_", map_name, 4) == 0)
    {
        id = strtoul(&map.map_name[4], NULL, 0) + CLASSMAP_ACL_ID_INDEX_START;
        map.index = id;
        ret = dbtable_array_get(policy_map_index, id, &map);
        if(ret != 0)
        {
            return DIFFSERV_RETURN_ACL_GROUP_RULE_NOTEXIST;
        }

        for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
        {
            if(id == acl_group.acl_index[uni])
            {
                acl_group.acl_index[uni] = 0;
                break;
            }
        }

        if(ACL_GROUP_MAX_RULES == uni)
        {
            return DIFFSERV_RETURN_ACL_GROUP_RULE_NOTEXIST;
        }
        ret = dbtable_array_update(acl_group_index, acl_group.group_index, NULL, &acl_group);
        
        memset(map.acl_group, 0, sizeof(map.acl_group));
        ret = dbtable_array_update(policy_map_index, id, &map, &map);
    }

    return MATCH_RETURN_CODE_SUCCESS;      
}
unsigned int acl_group_deploy(const char *group_name, int dir, unsigned int netif_index)
{
    struct acl_group_stc acl_group;
    struct policy_map_index_s policy_map;
    unsigned int vid = 0;
    unsigned int array_id = 0;
    unsigned int uni = 0;
    int ret = 0;
    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 != ret)
    {
        return DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }
    
    for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
    {
        if(0 != acl_group.acl_index[0])
        {
            break;
        }   
    }
    
    if(ACL_GROUP_MAX_RULES == uni)
        return DIFFSERV_RETURN_ACL_GROUP_EMPTY;
    
    if((1 == acl_group.is_deployed) && (dir != acl_group.dir_type))
    {
        return SERVICEPOLICY_RETURN_CODE_DIR_ERROR;
    }

    acl_group.dir_type = dir;
    acl_group.is_deployed = 1;
    switch(npd_netif_type_get(netif_index))
    {
        
        case NPD_NETIF_ETH_TYPE:
        {
            array_id = eth_port_array_index_from_ifindex(netif_index);
            if(NPD_PBMP_MEMBER(acl_group.portbmp, array_id))
            {
                return DIFFSERV_RETURN_CODE_ACL_GROUP_DEPLOYED;
            }
		    NPD_PBMP_PORT_ADD(acl_group.portbmp, array_id);
            break ;
        }
        case NPD_NETIF_VLAN_TYPE:
        { 
            vid = npd_netif_vlan_get_vid(netif_index);
            if(NPD_VBMP_MEMBER(acl_group.vlanbmp, vid))
            {
                return DIFFSERV_RETURN_CODE_ACL_GROUP_DEPLOYED;
            }
            NPD_VBMP_VLAN_ADD(acl_group.vlanbmp, vid);
            break ;
        }
        default :
            return SERVICEPOLICY_RETURN_CODE_NOTSUPPORT;  
    }

    /*deploy Acl*/
    for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
    {
        if(0 == acl_group.acl_index[0])
            continue;

        memset(&policy_map, 0, sizeof(struct policy_map_index_s));
        ret = dbtable_array_get(policy_map_index, acl_group.acl_index[uni], &policy_map);
        if(0 != ret)
        {
            continue;
        }

        service_policy_create(policy_map.map_name, dir, netif_index);
    }
    
    ret = dbtable_array_update(acl_group_index, acl_group.group_index, NULL, &acl_group);
    return MATCH_RETURN_CODE_SUCCESS;
}
unsigned int acl_group_undeploy(const char *group_name, int dir, unsigned int netif_index)
{
    struct acl_group_stc acl_group;
    struct policy_map_index_s policy_map;
    unsigned int vid = 0;
    unsigned int array_id = 0;
    unsigned int uni = 0;
    int ret = 0;
    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 != ret)
    {
        return DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }

    if(0 == acl_group.is_deployed)
    {
        return DIFFSERV_RETURN_CODE_ACL_GROUP_NOT_DEPLOYED;
    }
    
    if((1 == acl_group.is_deployed) && (dir != acl_group.dir_type))
    {
        return SERVICEPOLICY_RETURN_CODE_DIR_ERROR;
    }

    switch(npd_netif_type_get(netif_index))
    {
        
        case NPD_NETIF_ETH_TYPE:
        {
            array_id = eth_port_array_index_from_ifindex(netif_index);
            if(NPD_PBMP_MEMBER(acl_group.portbmp, array_id))
            {
    		    NPD_PBMP_PORT_REMOVE(acl_group.portbmp, array_id);
            }
            else
            {
                return DIFFSERV_RETURN_CODE_ACL_GROUP_NOT_DEPLOYED;
            }    
            break ;
        }
        case NPD_NETIF_VLAN_TYPE:
        { 
            vid = npd_netif_vlan_get_vid(netif_index);
            if(NPD_VBMP_MEMBER(acl_group.vlanbmp, vid))
            {
                NPD_VBMP_VLAN_REMOVE(acl_group.vlanbmp, vid);
            }
            else
            {
                return DIFFSERV_RETURN_CODE_ACL_GROUP_NOT_DEPLOYED;
            }
            break ;
        }
        default :
            return SERVICEPOLICY_RETURN_CODE_NOTSUPPORT;  
    }

    for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
    {
        if(0 == acl_group.acl_index[uni])
        {
            continue;
        }
        
        memset(&policy_map, 0, sizeof(struct policy_map_index_s));
        ret = dbtable_array_get(policy_map_index, acl_group.acl_index[uni], &policy_map);
        if(0 != ret)
        {
            continue;
        }

        service_policy_destroy(policy_map.map_name, dir, netif_index);
    }
    
    if(NPD_PBMP_IS_NULL(acl_group.portbmp) && NPD_VBMP_IS_NULL(acl_group.vlanbmp))
    {
        acl_group.dir_type = 0;/*init this member*/
        acl_group.is_deployed = 0;
    }
    
    ret = dbtable_array_update(acl_group_index, acl_group.group_index, NULL, &acl_group);
    return MATCH_RETURN_CODE_SUCCESS;
}
unsigned int acl_group_undeploy_all(const char *group_name)
{
    struct acl_group_stc acl_group;
    struct policy_map_index_s policy_map;
    unsigned int uni = 0;
    unsigned int array_id = 0;
    unsigned int netif_index = 0;
    int ret = 0;
    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 != ret)
    {
        return DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }

    if(0 == acl_group.is_deployed)
    {
        return MATCH_RETURN_CODE_SUCCESS;
    }
    
    for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
    {
        if(0 == acl_group.acl_index[uni])
            continue;

        memset(&policy_map, 0, sizeof(struct policy_map_index_s));
        ret = dbtable_array_get(policy_map_index, acl_group.acl_index[uni], &policy_map);
        if(0 != ret)
        {
            continue;
        }

        if(!NPD_PBMP_IS_NULL(acl_group.portbmp))
        {
        	NPD_PBMP_ITER(acl_group.portbmp, array_id)
        	{
        		netif_index = eth_port_array_index_to_ifindex(array_id);
                service_policy_destroy(policy_map.map_name, acl_group.dir_type, netif_index);
            }    
        }

        if(!NPD_VBMP_IS_NULL(acl_group.vlanbmp))
        {
        	NPD_VBMP_ITER(acl_group.vlanbmp, array_id)
        	{
        		netif_index = npd_netif_vlan_index(array_id);
                ret = service_policy_destroy(policy_map.map_name, acl_group.dir_type, netif_index);            
            }    
        }
    }
    
    NPD_VBMP_CLEAR(acl_group.vlanbmp);
    NPD_PBMP_CLEAR(acl_group.portbmp);
    acl_group.is_deployed = 0;
    acl_group.dir_type = 0;

    ret = dbtable_array_update(acl_group_index, acl_group.group_index, NULL, &acl_group);
    return MATCH_RETURN_CODE_SUCCESS;
}

int npd_is_rule_in_acl_group(int policy_index)
{
    int ret = 0;
    struct policy_map_index_s policy_map;
    memset(&policy_map, 0, sizeof(struct policy_map_index_s));
    ret = dbtable_array_get(policy_map_index, policy_index, &policy_map);
    if(0 != ret)
    {
        return FALSE;
    }

    if(strlen(policy_map.acl_group) > 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
int npd_acl_group_show_rule(unsigned int index, char * showStr)
{
    char *idstr = malloc(20);
    char *action = malloc(50);
    char *ethertype = malloc(50);
    char *ipprotocol = malloc(50);
    char *dstipstr = malloc(200);
    char *srcipstr = malloc(200);
    char *dstl4portstr = malloc(50);
    char *srcl4portstr = malloc(50);
    char *dstmacstr = malloc(50);
    char *srcmacstr = malloc(50);
    char *vlan_idstr = malloc(50);
    char *policerIdstr = malloc(50);
    char *redirctPortstr = malloc(50);
    char *profileIdstr = malloc(50);
    int   ret = 0;
    unsigned int eth_g_port = 0;
    char port_name[16];

	struct class_map_index_s	class_index ;
    struct policy_map_index_s	policy_index ;
	struct policy_map_rule_s	policy_rule ;
    struct class_map_rule_s    class_rule ;
    memset(&class_index, 0, sizeof(class_index));
    memset(&policy_index, 0, sizeof(policy_index));
    memset(&policy_rule, 0, sizeof(policy_rule));
    memset(&class_rule, 0, sizeof(class_rule));
    memset(port_name, 0, sizeof(port_name));

    if( !idstr||!action||!ethertype||!ipprotocol||!dstipstr||!srcipstr||!dstl4portstr||!profileIdstr
        ||!srcl4portstr||!dstmacstr||!srcmacstr||!vlan_idstr||!policerIdstr||!redirctPortstr
        ||!showStr)
    {
		if(idstr)
		{
            free(idstr);
		}
		if(action)
		{
            free(action);
		}
		if(ethertype)
		{
            free(ethertype);
		}
		if(ipprotocol)
		{
            free(ipprotocol);
		}
		if(dstipstr)
		{
            free(dstipstr);
		}
		if(srcipstr)
		{
            free(srcipstr);
		}
		if(dstl4portstr)
		{
            free(dstl4portstr);
		}
		if(srcl4portstr)
		{
            free(srcl4portstr);
		}
		if(dstmacstr)
		{
            free(dstmacstr);
		}
		if(srcmacstr)
		{
            free(srcmacstr);
		}
		if(vlan_idstr)
		{
            free(vlan_idstr);
		}
		if(policerIdstr)
		{
            free(policerIdstr);
		}
		if(redirctPortstr)
		{
            free(redirctPortstr);
		}
		if(profileIdstr)
		{
            free(profileIdstr);
		}
        return DIFFSERV_RETURN_CODE_ERROR;
    }
    memset(idstr, 0, 20);
    memset(action, 0, 50);
    memset(ethertype, 0, 50);
    memset(ipprotocol, 0, 50);
    memset(dstipstr, 0, 200);
    memset(srcipstr, 0, 200);
    memset(dstl4portstr, 0, 50);
    memset(srcl4portstr, 0, 50);
    memset(dstmacstr, 0, 50);
    memset(srcmacstr, 0, 50);
    memset(vlan_idstr, 0, 50);
    memset(policerIdstr, 0, 50);
    memset(redirctPortstr, 0, 50);
    memset(profileIdstr, 0, 50);

    ret = dbtable_array_get(policy_map_index, index, &policy_index);
    if((0 == ret) && (!strncmp(policy_index.map_name, "ACL_", 4)))
    {
        sprintf(idstr, "acl %s ", &policy_index.map_name[4]);

        ret = dbtable_array_get(class_map_master, policy_index.class_map_index, &class_index); 
		if (0 == ret)
		{
			strcpy(policy_rule.map_name, policy_index.map_name);
			policy_rule.index = policy_index.index;
			ret = dbtable_hash_head_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
			while (0 == ret)
			{
				if (0 == strcmp("drop", policy_rule.cmd_name))
				{
                    if(*((int*)policy_rule.cmd_arg) == 1)
					    sprintf(action, "deny ");
                    else
                        sprintf(action, "permit ");
				}
                else if(0 == strcmp("trap", policy_rule.cmd_name))
                {
                    sprintf(action, "trap ");
                }

                if ((0 == strcmp("mirror", policy_rule.cmd_name)) || (0 == strcmp("redirect", policy_rule.cmd_name)))
				{
					eth_g_port = *(unsigned int*)(policy_rule.cmd_arg);
					parse_eth_index_to_name(eth_g_port, port_name);
                    sprintf(action, "%s %s ", policy_rule.cmd_name, port_name); 
				}
				else if (0 == strcmp("qos-profile", policy_rule.cmd_name))
				{
                    sprintf(profileIdstr, "qos-profile %d ", *(unsigned int*)policy_rule.cmd_arg);
				}
				else if (0 == strcmp("policer", policy_rule.cmd_name))
				{
                    sprintf(policerIdstr, "policer %d ", *(unsigned int*)policy_rule.cmd_arg);
				}
                else if (0 == strcmp("policy-route", policy_rule.cmd_name))
                {
                    sprintf(action, "policy-route nexthop %d.%d.%d.%d ", (unsigned char)policy_rule.cmd_arg[0], (unsigned char)policy_rule.cmd_arg[1], (unsigned char)policy_rule.cmd_arg[2], (unsigned char)policy_rule.cmd_arg[3]);
                }
				ret = dbtable_hash_next_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
			}
		}
        sprintf(class_rule.map_name, policy_index.map_name);
        class_rule.index = policy_index.class_map_index;
        ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);

		while (0 == ret)
		{
            if (0 == strcmp("vlan-tag-num", class_rule.cmd_name))
            {
                if(*(unsigned int *)class_rule.cmd_arg == 0)
                {
                    sprintf(vlan_idstr, "vlan untagged ");
                }
            }
			if (0 == strcmp("outer-vlan", class_rule.cmd_name))
			{
				if(*(unsigned int*)class_rule.cmd_mask == 0)
                    sprintf(vlan_idstr, "vlan any ");
                else
                    sprintf(vlan_idstr, "vlan %d ", *(unsigned int*)class_rule.cmd_arg);
			}
			else if (0 == strcmp("srcip", class_rule.cmd_name)\
				 || 0 == strcmp("dstip", class_rule.cmd_name))
			{
				unsigned int value = 0;
				unsigned int mask = 0;
                int masklen = 0;
				char	arr_value[32];
				char	arr_mask[32];

				memset(&arr_value, 0, 32);
				memset(&arr_mask, 0, 32);
				
				memcpy(&value, class_rule.cmd_arg, 4);
				memcpy(&mask, class_rule.cmd_mask, 4);

				lib_get_string_from_ip(arr_value, (int)value);
                lib_get_masklen_from_mask(mask, &masklen);

     			if (0 == strcmp("srcip", class_rule.cmd_name))
     			{
                    if(0 == mask)
                        sprintf(srcipstr, "sip any ");
                    else
                        sprintf(srcipstr, "sip %s/%d ", arr_value, masklen);
     			}
                if(0 == strcmp("dstip", class_rule.cmd_name))
                {
                    if(0 == mask)
                        sprintf(dstipstr, "dip any ");
                    else
                        sprintf(dstipstr, "dip %s/%d ", arr_value, masklen);
                }
			}
#ifdef HAVE_NPD_IPV6
            else if(0 == strcmp("srcip6", class_rule.cmd_name)\
				 || 0 == strcmp("dstip6", class_rule.cmd_name))
            {
                int     maskLen = 0;
				char	strData[128];
				char	strMask[128];
                ip6_addr ip6Data;
                ip6_addr ip6Mask;

                memset(&ip6Data, 0, sizeof(&ip6Data));
                memset(&ip6Mask, 0, sizeof(&ip6Mask));
                memset(strData, 0, sizeof(strData));
                memset(strMask, 0, sizeof(strMask));
                memset(idstr, 0, sizeof(idstr));
                sprintf(idstr, "acl ipv6 %s ", &policy_index.map_name[4]);

				memcpy(ip6Data.u6_addr16, class_rule.cmd_arg, 16);
				memcpy(ip6Mask.u6_addr16, class_rule.cmd_mask, 16);

                lib_get_string_from_ipv6(strData, &ip6Data);
                lib_get_string_from_ipv6(strMask, &ip6Mask);
                lib_get_maskv6len_from_mask(&ip6Mask, &maskLen);

     			if (0 == strcmp("srcip6", class_rule.cmd_name))
     			{
                    if(0 == maskLen)
                        sprintf(srcipstr, "sip any ");
                    else
                        sprintf(srcipstr, "sip %s/%d ", strData, maskLen);
     			}
                if(0 == strcmp("dstip6", class_rule.cmd_name))
                {
                    if(0 == maskLen)
                        sprintf(dstipstr, "dip any ");
                    else
                        sprintf(dstipstr, "dip %s/%d ", strData, maskLen);
                }
            }
#endif
			else if (0 == strcmp("destination-address mac", class_rule.cmd_name)\
				 || 0 == strcmp("source-address mac", class_rule.cmd_name))
			{
				unsigned char	arr_value[6];
				unsigned char	arr_mask[6];

				memset(&arr_value, 0, 6);
				memset(&arr_mask, 0, 6);

				memcpy(&arr_value, class_rule.cmd_arg, 6);
				memcpy(&arr_mask, class_rule.cmd_mask, 6);

                
                if (0 == strcmp("destination-address mac", class_rule.cmd_name))
                {
                    int k;
                    for(k = 0; k < 6; k++)
                    {
                        if(arr_mask[k] != 0)
                            break;
                    }

                    if(k == 6)
                        sprintf(dstmacstr, "dmac any ");
                    else
                        sprintf(dstmacstr, "dmac %02x:%02x:%02x:%02x:%02x:%02x ", 
 									arr_value[0], arr_value[1], arr_value[2], arr_value[3], arr_value[4], arr_value[5]);
                }
				if(0 == strcmp("source-address mac", class_rule.cmd_name))
				{
                    int k;
                    for(k = 0; k < 6; k++)
                    {
                        if(arr_mask[k] != 0)
                            break;
                    }

                    if(k == 6)
                        sprintf(srcmacstr, "smac any ");
                    else
                        sprintf(srcmacstr, "smac %02x:%02x:%02x:%02x:%02x:%02x ", 
 									arr_value[0], arr_value[1], arr_value[2], arr_value[3], arr_value[4], arr_value[5]);
				}
			}
			else if (0 == strcmp("ethertype", class_rule.cmd_name))
			{
				unsigned int value = 0;

                memcpy(&value, class_rule.cmd_arg, 4);

                if(value == 0x0806)
                    sprintf(ethertype, "arp ");
                else if(value == 0x0800)
                {
                    if(strlen(ipprotocol) == 0)
                        sprintf(ipprotocol, "ip ");
                }
                else if(value != 0)
                    sprintf(ethertype, "ethertype 0x%04x ", value);
			}
			else if (0 == strcmp("protocol", class_rule.cmd_name))
			{
				unsigned int value = 0;
							
				memcpy(&value, class_rule.cmd_arg, 4);
                if(value == 17)
                    sprintf(ipprotocol, "udp ");
                if(value == 6)
                    sprintf(ipprotocol, "tcp ");
                if(value == 1)
                    sprintf(ipprotocol, "icmp ");
			}
			else if (0 == strcmp("dstl4port", class_rule.cmd_name)\
				 || 0 == strcmp("srcl4port", class_rule.cmd_name))
			{
				unsigned int	value = *(unsigned int*)class_rule.cmd_arg;

                if (0 == strcmp("dstl4port", class_rule.cmd_name))
                {
                    if(*(unsigned int*)class_rule.cmd_mask == 0)
                        sprintf(dstl4portstr, "dst-port any ");
                    else
                        sprintf(dstl4portstr, "dst-port %d ", value);
                }
				else if(0 == strcmp("srcl4port", class_rule.cmd_name))
				{
                    if(*(unsigned int*)class_rule.cmd_mask == 0)
                        sprintf(srcl4portstr, "src-port any ");
                    else
                        sprintf(srcl4portstr, "src-port %d ", value);
				}
			}
		  	ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
	    }
        if(strlen(dstmacstr) != 0)
        {
			if(0 == strncmp(ethertype, "arp", 3))
				sprintf(ethertype, "ethertype 0x0806 ");
			if(0 == strncmp(ipprotocol, "ip", 2))
			{
                memset(ipprotocol, 0, 50);
				sprintf(ethertype, "ethertype 0x0800 ");
			}
        }
        sprintf(showStr, "%s%s%s%s%s%s%s%s%s%s%s%s\n",
                          idstr, action, ethertype, ipprotocol, dstipstr, dstl4portstr, srcipstr, srcl4portstr,
                          dstmacstr, srcmacstr, vlan_idstr, policerIdstr);
        ret = MATCH_RETURN_CODE_SUCCESS;
        
    }
    else
    {
        ret = POLICYMAP_RETURN_CODE_NOTEXIST;
    }

    free(idstr);
    free(action);
    free(ethertype);
    free(ipprotocol);
    free(dstipstr);
    free(srcipstr);
    free(dstl4portstr);
    free(srcl4portstr);
    free(dstmacstr);
    free(srcmacstr);
    free(vlan_idstr);
    free(policerIdstr);
    free(redirctPortstr);
    free(profileIdstr);

    return ret;
}
int npd_acl_group_show_port(npd_pbmp_t portbmp, char *showStr)
{
    unsigned int array_index;
	char port_name[512];
	char port_string[512];
	unsigned int eth_g_index = 0;
    int start_port = 0;
    int end_port = 0;
    int end_array_id = 0;
    if(NULL == showStr)
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }
    memset(port_name, 0, sizeof(port_name));
    memset(port_string, 0, sizeof(port_string));
    NPD_PBMP_ITER(portbmp, array_index)
    {
		eth_g_index = eth_port_array_index_to_ifindex(array_index);
        if(0 == start_port)
        {
            start_port = end_port = eth_g_index;
            end_array_id = array_index;
            continue;
        }
        if(end_array_id == array_index-1)
        {
            int slot1 = eth_port_get_slot_by_ifindex(end_port);
            int slot2 = eth_port_get_slot_by_ifindex(eth_g_index);
            if(slot1 == slot2)
            {
                end_port = eth_g_index;
                end_array_id = array_index;
                continue;
            }
        }
        if(start_port == end_port)
        {
		    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s, ", port_string, port_name);
        }
        else
        {
		    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s-",port_string, port_name);
            parse_eth_index_to_name(end_port, port_name);
            sprintf(port_string,"%s%s, ",port_string, port_name);
        }
        start_port = end_port = eth_g_index;
        end_array_id = array_index;
        if(strlen(port_string)+15 < 39)
            continue;
        else
        {
            strcat(showStr, port_string);
            memset(port_string, 0, sizeof(port_string));
        }
    }
    
    if(start_port)
    {
        if(start_port == end_port)
        {
    	    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s", port_string, port_name);
        }
        else
        {
    	    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s-",port_string, port_name);
            parse_eth_index_to_name(end_port, port_name);
            sprintf(port_string,"%s%s",port_string, port_name);
        }                
        printf("vfork port_string = %s\n", port_string);
        strcat(showStr, port_string);
    }
    return MATCH_RETURN_CODE_SUCCESS;
}
int npd_acl_group_show_vlan(struct acl_group_stc acl_group, char *showStr)
{
	int start_vid = 0;
	int end_vid = 0;
    int length = 0;
    int bufLen = 0;
    int vid_str_len = 0;
    unsigned int vid = 0;
    char tmpBuf[1024];

    if(showStr == NULL)
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }
    memset(tmpBuf, 0, sizeof(tmpBuf));
    bufLen = strlen(showStr);
    NPD_VBMP_ITER(acl_group.vlanbmp, vid)
    {
        if(start_vid == 0)
        {
            start_vid = vid;
            end_vid = vid;
        }
        else if(vid == end_vid+1)
        {
            end_vid = vid;
        }
        else
        {
            if(0 == vid_str_len)
                sprintf(tmpBuf, "vlan ");
            if(start_vid == end_vid)
            {
                sprintf(tmpBuf, "%s%d", tmpBuf, start_vid);
            }
            else
            {
                sprintf(tmpBuf, "%s%d-%d", tmpBuf, start_vid, end_vid);
            }
            vid_str_len = strlen(tmpBuf);
            if(vid_str_len < 50) 
                sprintf(tmpBuf, "%s,", tmpBuf);
            else
            {
                sprintf(tmpBuf, "%s\n", tmpBuf);
                length += strlen(tmpBuf);
                if(length < bufLen)
                    strcat(showStr, tmpBuf);
                vid_str_len = 0;
            }
            start_vid = vid;
            end_vid = vid;
        }
    }
    if(start_vid != 0)
    {
        if(0 == vid_str_len)
        {
            sprintf(tmpBuf, "vlan ");
        }
        if(start_vid == end_vid)
        {
            sprintf(tmpBuf, "%s%d\n", tmpBuf, start_vid);
        }
        else
        {
            sprintf(tmpBuf, "%s%d-%d\n", tmpBuf, start_vid, end_vid);
        }
        length += strlen(tmpBuf);
//        if(length < bufLen)
            strcat(showStr, tmpBuf);
        vid_str_len = 0;
    }

    return MATCH_RETURN_CODE_SUCCESS;
}
int npd_show_l4port_range(const char* cmd_name, const char *cmd_arg, char *show_string)
{
    int uni = 0;
    int  port_range_index[L4_PORT_RANGE_OP_TOTOAL];
    char index_string[L4_PORT_RANGE_OP_TOTOAL][4];
    char *buf = NULL;
    char *endptr = NULL;
    int ret = 0;
    struct l4port_range_s port_range[L4_PORT_RANGE_OP_TOTOAL];
    if((NULL == cmd_name) || (NULL == cmd_arg) || (NULL == show_string))
        return DIFFSERV_RETURN_CODE_ERROR;

    buf = (char *)malloc(33);
    buf[32] = 0;

    for(uni = 0; uni < L4_PORT_RANGE_OP_TOTOAL; uni++)
    {
        memset(port_range_index, -1, sizeof(port_range_index));
        memset(index_string, -1, sizeof(index_string));
        memset(port_range, 0, sizeof(port_range));
    }

    for(uni=0; uni < L4_PORT_RANGE_OP_TOTOAL; uni++)
    {
        memcpy(index_string[uni], &cmd_arg[4*uni], 4);
        port_range_index[uni] = strtol(index_string[uni], &endptr, 10);
        if(-1 != port_range_index[uni])
        {
            ret = dbtable_array_get(l4port_range_master, port_range_index[uni], &port_range[uni]);
        }
    }
    if((-1 != port_range_index[0])
        &&(-1 != port_range_index[1])
        &&(port_range[0].protocol == port_range[1].protocol))
    {
        if(TCP_PROTOCOL == port_range[0].protocol)
            sprintf(buf, "TCP port range %d - %d", port_range[0].l4port, port_range[1].l4port);
        else if(UDP_PROTOCOL == port_range[0].protocol)
            sprintf(buf, "UDP port range %d - %d", port_range[0].l4port, port_range[1].l4port);
        else
            sprintf(buf, "INVALID RANGE INDEX %d - %d", port_range_index[0], port_range_index[1]);
    }
    else
    {
        if(-1 != port_range_index[0])
        {
            if(TCP_PROTOCOL == port_range[0].protocol)
                sprintf(buf, "TCP port gt %d", port_range[0].l4port);
            else if(UDP_PROTOCOL == port_range[0].protocol)
                sprintf(buf, "UDP port gt %d", port_range[0].l4port);
            else
                sprintf(buf, "INVALID GT INDEX %d", port_range_index[0]);
        }
        else if(-1 != port_range_index[1])
        {
            if(TCP_PROTOCOL == port_range[1].protocol)
                sprintf(buf, "TCP port lt %d", port_range[1].l4port);
            else if(UDP_PROTOCOL == port_range[1].protocol)
                sprintf(buf, "UDP port lt %d", port_range[1].l4port);
            else
                sprintf(buf, "INVALID LT INDEX %d", port_range_index[1]);
        }
        else if(-1 != port_range_index[2])
        {
            if(TCP_PROTOCOL == port_range[2].protocol)
                sprintf(buf, "TCP port neq %d", port_range[2].l4port);
            else if(UDP_PROTOCOL == port_range[2].protocol)
                sprintf(buf, "UDP port neq %d", port_range[2].l4port);
            else
                sprintf(buf, "INVALID NEQ INDEX %d", port_range_index[2]);

        }
    }

    strncpy(show_string, buf, 32);
    free(buf);
    return MATCH_RETURN_CODE_SUCCESS;
}

void class_qos_init()
{
    db_table_t *db = NULL;
    int ret = 0;

   ret = create_dbtable("timerangedb", 256, sizeof(struct time_range_info_s), 
							NULL,
							NULL,
							NULL,
							NULL, 
							NULL,
							NULL,
							NULL,
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&db);

    ret = dbtable_create_array_index("tmrangeindex", db, &time_range_info_master);

    ret = dbtable_create_hash_index("tmrangename", db, 256, 
					time_range_info_namehash_key,
					time_range_info_namehash_cmp,
					&time_range_info_name);
	
    ret = create_dbtable("l4portrangedb", 256, sizeof(struct l4port_range_s),
                            NULL,
                            NULL,
                            l4port_range_handle_insert,
                            l4port_range_handle_delete,
                            NULL, 
							NULL, 
							NULL,
        					NULL,
        					NULL,
						 	DB_SYNC_ALL,
						 	&db);
    ret = dbtable_create_array_index("l4portrangeindex", db, &l4port_range_master);

    ret = dbtable_create_hash_index("l4portrangename", db, 256,
                     l4port_range_namehash_key,
                     l4port_range_namehash_cmp,
                     &l4port_range_name);

    ret = create_dbtable("classmapdb", CMAP_MAX_RULE_NUM, sizeof(struct class_map_index_s), 
							NULL,
							NULL,
							NULL,
							NULL, 
							NULL,
							NULL,
							NULL,
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&db);

    ret = dbtable_create_array_index("cmapaindex", db, &class_map_master);

    ret = dbtable_create_hash_index("cmaphname", db, CMAP_MAX_RULE_NUM, 
					class_map_index_namehash_key,
					class_map_index_namehash_cmp,
					&class_map_master_name);

    ret = create_dbtable("cmapmatchruledb", MAX_CLASSMAP_RULE_NUM, sizeof(struct class_map_rule_s),
							NULL,
							NULL,
							NULL,
							NULL, 
							NULL,
							NULL,
							NULL,
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&db);
	
    ret = dbtable_create_hash_index("cmapmatchhname", db, MAX_CLASSMAP_RULE_NUM,
					class_map_rule_key, 
					class_map_rule_cmp, 
					&class_map_match_rule);

	ret = create_dbtable("policymapdb", CMAP_MAX_RULE_NUM, sizeof(struct policy_map_index_s), 
							NULL,
							NULL,
							NULL,
							NULL, 
							NULL,
							NULL,
							NULL,
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&db);

    ret = dbtable_create_array_index("pmapaincdex", db, &policy_map_index);

    ret = dbtable_create_hash_index("pmaphname", db, CMAP_MAX_RULE_NUM, 
					policy_map_index_namehash_key,
					policy_map_index_namehash_cmp,
					&policy_map_name);

    ret = create_dbtable("pmapsetruledb", MAX_CLASSMAP_RULE_NUM, sizeof(struct policy_map_rule_s),
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
							NULL,
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&db);
	
	ret = dbtable_create_hash_index("pmapsethname", db, MAX_CLASSMAP_RULE_NUM,
					policy_map_rule_key, 
					policy_map_rule_cmp, 
					&policy_map_set_rule);
#ifdef HAVE_ROUTE    

	ret = create_dbtable("serpolicypb", CMAP_MAX_RULE_NUM, sizeof(struct service_policy_route_s),
        		                  policy_route_nexthop_update,
        		                  NULL,
        		                  policy_route_nexthop_insert,
        		                  NULL,
								NULL, 
								NULL, 
								NULL,
            					NULL,
            					NULL,
							 	DB_SYNC_ALL,
							 	&db);
	ret = dbtable_create_hash_index("serpolicypbnamehash", db, CMAP_MAX_RULE_NUM,
		                      ser_policy_pb_name_hash,
		                      ser_policy_pb_cmp,
		                      &ser_policy_pb_index);
	ret = dbtable_create_hash_index("serpolicypbhash", db, CMAP_MAX_RULE_NUM,
		                      ser_policy_pb_ip_hash,
		                      ser_policy_pb_cmp,
		                      &ser_policy_pb_hash);
#endif
    ret = create_dbtable("serpolicydb", CMAP_MAX_RULE_NUM, sizeof(struct service_policy_s),
								service_policy_handle_update,
								NULL,
								service_policy_handle_insert,
								service_policy_handle_delete,
								NULL, 
								NULL, 
								NULL,
            					NULL,
            					NULL,
							 	DB_SYNC_ALL,
							 	&db);

	ret = dbtable_create_array_index("serpolicyaindex", db, &service_policy_index);

	ret = dbtable_create_hash_index("serpolicyhname", db, CMAP_MAX_RULE_NUM,
						service_policy_index_namehash_key, 
						service_policy_index_namehash_cmp, 
						&service_policy_name);

    ret = create_dbtable("aclgroupdb", 512, sizeof(struct acl_group_stc), 
							NULL, /*acl_group_handle_update,*/
							NULL,
							NULL, /*acl_group_handle_insert,*/
							NULL, 
							NULL,
							NULL,
							NULL,
        					NULL,
        					NULL,
							DB_SYNC_ALL,
							&db);

    ret = dbtable_create_array_index("aclgroupindex", db, &acl_group_index);

    ret = dbtable_create_hash_index("aclgroupname", db, 512, 
					acl_group_namehash_key,
					acl_group_namehash_cmp,
					&acl_group_name);



	ret = create_dbtable("portGroupdb", CMAP_MAX_RULE_NUM, sizeof(struct port_group_s), 
							NULL,
							NULL,
							NULL,
							NULL, 
							NULL,
							NULL,
							NULL,
        					NULL,
        					NULL,
							DB_SYNC_NONE,
							&db);

    ret = dbtable_create_array_index("portGrpArrindex", db, &port_group_array_index);
	
	ret = dbtable_create_hash_index("portGrpHashindex", db, CMAP_MAX_RULE_NUM,
						port_grp_index_key, 
						port_grp_index_cmp, 
						&port_group_hash_index);


 
}

/*NPD DBUS operation*/
DBusMessage * npd_dbus_create_acl_group(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &group_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = (unsigned int)acl_group_create(group_name);	


	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_delete_acl_group(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &group_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = (unsigned int)acl_group_delete(group_name);	


	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_delete_rule(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;
    char*           map_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &group_name,
					   DBUS_TYPE_STRING, &map_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	diffserv_check_golbal_start();

	ret = (unsigned int)acl_group_delete_rule(group_name, map_name);	

	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_add_rule(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;
    char*           map_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &group_name,
					   DBUS_TYPE_STRING, &map_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	diffserv_check_golbal_start();

	ret = (unsigned int)acl_group_add_rule(group_name, map_name);	

	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_add_desp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;
    char*           description = NULL;

    struct acl_group_stc acl_group;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &group_name,
					   DBUS_TYPE_STRING, &description,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	diffserv_check_golbal_start();

    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);
	ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 == ret)
    {
        strncpy(acl_group.desp, description, 128);
        ret = dbtable_array_update(acl_group_index, acl_group.group_index, &acl_group, &acl_group);
        if(0 == ret)
        {
            ret = MATCH_RETURN_CODE_SUCCESS;
        }
        else
        {
            ret = DIFFSERV_RETURN_CODE_ACL_GROUP_CONFIG_FAILE;
        }
    }
    else
    {
        ret = DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }
	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_delete_desp(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;

    struct acl_group_stc acl_group;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &group_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	diffserv_check_golbal_start();

    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);
	ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 == ret)
    {
        memset(acl_group.desp, 0, sizeof(acl_group.desp));
        ret = dbtable_array_update(acl_group_index, acl_group.group_index, &acl_group, &acl_group);
        if(0 == ret)
        {
            ret = MATCH_RETURN_CODE_SUCCESS;
        }
        else
        {
            ret = DIFFSERV_RETURN_CODE_ACL_GROUP_CONFIG_FAILE;
        }
    }
    else
    {
        ret = DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }

	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_deploy(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;
	unsigned int	eth_g_index = 0;
    int             dir = 0;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_UINT32, &dir,
							DBUS_TYPE_STRING, &group_name,
							DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();

	ret = acl_group_deploy(group_name, dir, eth_g_index);    

	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_undeploy(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;
	unsigned int	eth_g_index = 0;
    int             dir = 0;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_UINT32, &dir,
							DBUS_TYPE_STRING, &group_name,
							DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();

	ret = acl_group_undeploy(group_name, dir, eth_g_index);    

	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_undeploy_all(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			group_name = NULL;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
							DBUS_TYPE_STRING, &group_name,
							DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();

	ret = acl_group_undeploy_all(group_name);    

	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_group_show_detail(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    char *group_name = NULL;
    char *show_str   = NULL;
    struct acl_group_stc acl_group;
    unsigned int ret = 0;
    unsigned int uni = 0;
    unsigned int have_rules = 0;
    char tmp[256];
    
	DBusMessage*	reply = NULL;    
	DBusMessageIter	iter;
	DBusError		err;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &group_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}

		return NULL;
	}
    show_str = (char *)malloc(100 * SHOW_SERVICE_SIZE);
    
    memset(&acl_group, 0, sizeof(struct acl_group_stc));
    strncpy(acl_group.name, group_name, 32);

    ret = dbtable_hash_search(acl_group_name, &acl_group, NULL, &acl_group);
    if(0 != ret)
    {
        ret = DIFFSERV_RETURN_ACL_GROUP_NOTEXIST;
    }
    
    if(NULL != show_str)
    {
        memset(show_str, 0, sizeof(show_str));
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%-16s:\t%s\n", "Acl-group name", acl_group.name);
        strcat(show_str, tmp);

        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%-16s:\t%s\n", "Deploy status", acl_group.is_deployed?"deployed":"undeploy");
        strcat(show_str, tmp);

        if(1 == acl_group.is_deployed)
        {
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "%-16s:\t%s\n", "Direction", acl_group.dir_type?"Egress":"Ingress");
            strcat(show_str, tmp);
        }
        if(strlen(acl_group.desp) > 0)
        {
            unsigned int lineFlag = 1;
            unsigned int icount = 0;
            unsigned int length = 0;
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "%-16s:\n\t", "Description");
            strcat(show_str, tmp);

            length = strlen(acl_group.desp);
            do
            {
                memset(tmp, 0, sizeof(tmp));
                if(length >= 48)
                {
                    strncpy(tmp, &acl_group.desp[48*icount], 48);
                    strcat(show_str, tmp);
                    strcat(show_str, "\n\t");
                    length -= 48;
                    icount++;
                }
                else
                {
                    strncpy(tmp, &acl_group.desp[48*icount], length);
                    strcat(show_str, tmp);
                    strcat(show_str, "\n");
                    lineFlag = 0;
                }
            }while(lineFlag);
        }
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%-16s:\n", "Rule(s)");
        strcat(show_str, tmp);
        for(uni = 0; uni < ACL_GROUP_MAX_RULES; uni++)
        {
            if(0 == acl_group.acl_index[uni])
                continue;

            if(0 == have_rules)
            {
                have_rules = 1;
            }
            memset(tmp, 0, sizeof(tmp));
            ret = npd_acl_group_show_rule(acl_group.acl_index[uni], tmp);
            if(MATCH_RETURN_CODE_SUCCESS == ret)
            {
                strcat(show_str, tmp);
            }
        }

        if(!have_rules)
        {
            strcat(show_str, "\n\tNo rules in the group.\n");
        }
        
        if(NPD_PBMP_NOT_NULL(acl_group.portbmp))
        {
            char *portStr = NULL;
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "%-16s:\n", "Applied on port(s)");
            strcat(show_str, tmp);
            portStr = (char *)malloc(4096);
            memset(portStr, 0, sizeof(portStr));
            if(MATCH_RETURN_CODE_SUCCESS == npd_acl_group_show_port(acl_group.portbmp, portStr))
            {
                strcat(show_str, portStr);
            }
            strcat(show_str, "\n");
        }

        if(NPD_VBMP_NOT_NULL(acl_group.vlanbmp))
        {   
            char *vlanStr = NULL;
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "%-16s:\n", "Applied on Vlan(s)");
            strcat(show_str, tmp);

            vlanStr = (char *)malloc(4096);
            memset(vlanStr, 0, sizeof(vlanStr));
            ret = npd_acl_group_show_vlan(acl_group, vlanStr);
            if(MATCH_RETURN_CODE_SUCCESS == ret)
            {
                strcat(show_str, vlanStr);
            }
            strcat(show_str, "\n");
            free(vlanStr);
        }
    }
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);

    if(NULL != show_str)
    {
        free(show_str);
        show_str = NULL;
    }
    return reply;
}

DBusMessage * npd_dbus_acl_group_intf_show(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    unsigned int netif_index = 0;
    int totalcount = 0;
    char *show_str   = NULL;
    unsigned int have_acl_group = 0;
    struct acl_group_stc acl_group;
    unsigned int ret = 0;
    unsigned int uni = 0;
    unsigned int arr_index = 0;
    unsigned int vid = 0;
    unsigned int eth_or_vlan_flag = 0;
    char tmp[128];
    
	DBusMessage*	reply = NULL;    
	DBusMessageIter	iter;
	DBusError		err;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_UINT32, &netif_index,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}

		return NULL;
	}
    show_str = (char *)malloc(SHOW_SERVICE_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, sizeof(show_str));
        eth_or_vlan_flag = npd_netif_type_get(netif_index);
        switch(eth_or_vlan_flag)
        {
            case NPD_NETIF_TRUNK_TYPE:
                
            case NPD_NETIF_ETH_TYPE:
                arr_index = eth_port_array_index_from_ifindex(netif_index);
                break ;
            case NPD_NETIF_VLAN_TYPE:
                vid = npd_netif_vlan_get_vid(netif_index);
                break ;
            default:
                break ;
        }
        
        totalcount = dbtable_array_totalcount(acl_group_index);
        for(uni = 0; uni < totalcount; uni++)
        {
            memset(&acl_group, 0, sizeof(struct acl_group_stc));
            ret = dbtable_array_get(acl_group_index, uni, &acl_group);
            if(0 != ret)
                continue;

            if(NPD_NETIF_ETH_TYPE == eth_or_vlan_flag)
            {
                if(!NPD_PBMP_MEMBER(acl_group.portbmp, arr_index))
                    continue;
            }
            else if(NPD_NETIF_VLAN_TYPE == eth_or_vlan_flag)
            {
                if(!NPD_VBMP_MEMBER(acl_group.vlanbmp, vid))
                    continue;
            }
            
            if(0 == have_acl_group)
            {
                have_acl_group = 1;
                memset(tmp, 0, sizeof(tmp));
                sprintf(tmp, "%-32s %-12s\n", "NAME", "DIRECTION");
                strcat(show_str, tmp);
                strcat(show_str, "================================ ============\n");
            }

            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "%-32s %-12s\n", acl_group.name, acl_group.dir_type?"Egress":"Ingress");
            strcat(show_str, tmp);
        }
        if(0 == have_acl_group)
        {
            strcat(show_str, "No ACL applied in the interface.\n");
        }
    }

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);

    if(NULL != show_str)
    {
        free(show_str);
        show_str = NULL;
    }
    return reply;
}
DBusMessage* npd_dbus_acl_group_show_all(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	char*	showStr = NULL;
	char	tmp[128];
    struct acl_group_stc acl_group;
    unsigned int uni = 0;
    unsigned int index = 0;
    int     totalcount = 0;
    int     ret = 0;

    DBusMessage*	reply;
	DBusMessageIter	iter;

	showStr = (char*)malloc(NPD_ACL_RULE_SHOWRUN_CFG_SIZE * 10);
	if(NULL == showStr) 
	{
		syslog_ax_acl_err("memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_ACL_RULE_SHOWRUN_CFG_SIZE * 10);
    memset(&acl_group, 0, sizeof(struct acl_group_stc));

    totalcount = dbtable_array_totalcount(acl_group_index);
    for(uni = 0; uni < totalcount; uni++)
    {
        memset(&acl_group, 0, sizeof(struct acl_group_stc));
        ret = dbtable_array_get(acl_group_index, uni, &acl_group);
        if(0 != ret)
            continue;

        index++;
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%-6d\t%-32s\t%-12s\t\n", index, acl_group.name, acl_group.is_deployed?"Deployed":"Undeployed");
        strcat(showStr, tmp);
    }

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}
DBusMessage * npd_dbus_create_class_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			map_name = NULL;
	unsigned char	netFamily = 0;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING,&map_name,
					   DBUS_TYPE_BYTE,&netFamily,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	diffserv_check_golbal_start();
	ret = (unsigned int)class_map_create(map_name);	

	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_no_class_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	map_name = NULL;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING,&map_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();
	ret = (unsigned int)class_map_delete(map_name);
	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_rename_class_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	oldmap_name = NULL;
	char*	newmap_name = NULL;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
								DBUS_TYPE_STRING, &oldmap_name,
								DBUS_TYPE_STRING, &newmap_name,
					  			DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();
	ret = (unsigned int)class_map_rename(oldmap_name, newmap_name);
	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_class_map_add_match(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply = NULL; 
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	map_name = NULL;
	char*	match_name = NULL;
	char*	data = NULL;
	char*	mask = NULL;
	struct class_map_index_s class_map;

	memset(&class_map, 0, sizeof(struct class_map_index_s));
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &map_name,
					   DBUS_TYPE_STRING, &match_name,
					   DBUS_TYPE_STRING, &data,
					   DBUS_TYPE_STRING, &mask,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	if(CLASSMAP_RETURN_CODE_EXIST != class_map_find_by_name(map_name, &class_map)) 
	{
		ret = CLASSMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != class_map.is_deployed)
	{
		ret = CLASSMAP_RETURN_CODE_BINDED;
	}
	else
	{
		diffserv_check_golbal_start();
		ret = class_map_add_match(&class_map, match_name, data, mask);	
		diffserv_check_golbal_end();
	}

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}


DBusMessage * npd_dbus_class_map_delete_match(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply = NULL; 
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	map_name = NULL;
	char*	match_name = NULL;
	struct class_map_index_s class_map;

	memset(&class_map, 0, sizeof(struct class_map_index_s));
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING, &map_name,
					   DBUS_TYPE_STRING, &match_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	if(CLASSMAP_RETURN_CODE_EXIST != class_map_find_by_name(map_name, &class_map)) 
	{
		ret = CLASSMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != class_map.is_deployed)
	{
		ret = CLASSMAP_RETURN_CODE_BINDED;
	}
	else
	{
		diffserv_check_golbal_start();
		ret = class_map_delete_match(&class_map, match_name);	
		diffserv_check_golbal_end();
	}

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
	
DBusMessage * npd_dbus_create_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			map_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING,&map_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();
	ret = (unsigned int)policy_map_create(map_name);

	if (POLICYMAP_RETURN_CODE_EXIST == ret)
	{
		ret = MATCH_RETURN_CODE_SUCCESS;
	}
	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_no_policy_map(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			policy_name = NULL;
	struct policy_map_index_s policy_map;

	memset(&policy_map, 0, sizeof(struct policy_map_index_s));
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_STRING,&policy_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	diffserv_check_golbal_start();
	ret = (unsigned int)policy_map_delete (policy_name);
	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_rename_policy_map(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	pNameOld = NULL;
	char*	pNameNew = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
						DBUS_TYPE_STRING, &pNameOld,
						DBUS_TYPE_STRING, &pNameNew,
						DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();
	ret = (unsigned int)policy_map_rename(pNameOld, pNameNew);
	diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_policy_map_add_action(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	policy_name = NULL;
	char*	action_name = NULL;
	char*	data = NULL;
	struct policy_map_index_s policy_map;

	memset(&policy_map, 0, sizeof(struct policy_map_index_s));
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
					   DBUS_TYPE_STRING, &policy_name,
					   DBUS_TYPE_STRING, &action_name,
					   DBUS_TYPE_STRING, &data,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	if(POLICYMAP_RETURN_CODE_EXIST != policy_map_find_by_name(policy_name, &policy_map)) 
	{
		ret = POLICYMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != policy_map.is_deployed)
	{
		ret = POLICYMAP_RETURN_CODE_DEPLOYED;
	}
	else
	{
		diffserv_check_golbal_start();
		ret = policy_map_add_set(&policy_map, action_name, data);
		diffserv_check_golbal_end();
	}

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_policy_map_delete_action(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	policy_name = NULL;
	char*	action_name = NULL;
	struct policy_map_index_s policy_map;

	memset(&policy_map, 0, sizeof(struct policy_map_index_s));
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
					   DBUS_TYPE_STRING, &policy_name,
					   DBUS_TYPE_STRING, &action_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	if(POLICYMAP_RETURN_CODE_EXIST != policy_map_find_by_name(policy_name, &policy_map)) 
	{
		ret = POLICYMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != policy_map.is_deployed)
	{
		ret = POLICYMAP_RETURN_CODE_DEPLOYED;
	}
	else if (-1 == policy_map.class_map_index)
	{
		ret = POLICYMAP_RETURN_CODE_NOTCLASS;
	}
	else
	{
		diffserv_check_golbal_start();
		ret = policy_map_delete_set(&policy_map, action_name);
		diffserv_check_golbal_end();
	}

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_policy_map_class(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	class_name = NULL;
	char*	policy_name = NULL;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
					   DBUS_TYPE_STRING, &class_name,
					   DBUS_TYPE_STRING, &policy_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	diffserv_check_golbal_start();
	ret = (unsigned int)policy_map_class(class_name, policy_name);
	diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_policy_map_no_class(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	class_name = NULL;
	char*	policy_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
					   DBUS_TYPE_STRING, &class_name,
					   DBUS_TYPE_STRING, &policy_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	diffserv_check_golbal_start();
	ret = (unsigned int)policy_map_no_class(class_name, policy_name);
	diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}


DBusMessage * npd_dbus_service_policy_in(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			policy_name = NULL;
	unsigned int	eth_g_index = 0;
	struct policy_map_index_s policy_map;
    int dir;

	memset(&policy_map, 0, sizeof(struct policy_map_index_s));
    
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_UINT32, &dir,
							DBUS_TYPE_STRING, &policy_name,
							DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();
	/*diffserv_check_port_start(eth_g_index);*/
    strncpy(policy_map.map_name, policy_name, 32);
    ret = dbtable_hash_search(policy_map_name, &policy_map, NULL, &policy_map);
    if(strlen(policy_map.acl_group) > 0)
    {
        ret = POLICYMAP_RETURN_CODE_BIND_ACL_GROUP;
    }
    else
    {
    	ret = service_policy_create(policy_name, dir, eth_g_index);
    }
	/*diffserv_check_port_end();*/
	diffserv_check_golbal_end();

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

DBusMessage * npd_dbus_service_no_policy(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply;    
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*			policy_name = NULL;
	unsigned int	eth_g_index = 0;
	struct policy_map_index_s policy_map;
    int dir;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_UINT32, &dir,
							DBUS_TYPE_STRING, &policy_name,
							DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	diffserv_check_golbal_start();
	/*diffserv_check_port_start(eth_g_index);*/
	if(POLICYMAP_RETURN_CODE_NOTEXIST == policy_map_find_by_name(policy_name, &policy_map)) 
	{
		ret =SERVICEPOLICY_RETURN_CODE_NOEXIST;
	}
	else if (0 == policy_map.is_deployed)
	{
		ret = SERVICEPOLICY_RETURN_CODE_NOEXIST;
	}
    else if (strlen(policy_map.acl_group) > 0)
    {
        ret = POLICYMAP_RETURN_CODE_BIND_ACL_GROUP;
    }
	else 
	{									/* direction 0 equal in*/
		ret = service_policy_destroy(policy_name, dir, eth_g_index);
	}
	/*diffserv_check_port_end();*/
	diffserv_check_golbal_end();
/*
	counter = dbtable_hash_count(policy_map_name);
	if(counter > 0)
	    usleep(30*counter);
*/
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}


DBusMessage * npd_dbus_acl_show_cmap_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
 	unsigned int	name_count = 0;
	unsigned int	ret = MATCH_RETURN_CODE_SUCCESS;
    char *          show_str = NULL;
    struct class_map_index_s class_index;
	DBusMessage*	reply = NULL;    
	DBusMessageIter	iter;

	memset(&class_index, 0, sizeof(struct class_map_index_s));

    show_str = (char *)malloc(SHOW_SERVICE_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, SHOW_SERVICE_SIZE);
        ret = dbtable_hash_next(class_map_master_name, NULL, &class_index, NULL);
    	while (0 == ret)
    	{
            if ((0 != strncmp(class_index.map_name, "SG_DEFAULT_DENY", sizeof(class_index.map_name)))
                && (0 != strncmp(class_index.map_name, "SG_DEFAULT", sizeof(class_index.map_name)))
                && (0 != strncmp(class_index.map_name, "SG_", 3))
                && (0 != strncmp(class_index.map_name, "ACL_", 4))
                && (0 != strcmp(class_index.map_name, "VOICE VLAN"))
#ifdef HAVE_PORTAL               
                && (0 != strncmp(class_index.map_name, "PORTAL_", 7))
#endif          
               )
            {
                char tmp[1024];
                memset(tmp, 0, 1024);
                sprintf(tmp, "%-15d%-15s%-40s\n", name_count, "MATCH-ALL", class_index.map_name);
                strcat(show_str, tmp);
    		    name_count++;
            }

    		ret = dbtable_hash_next(class_map_master_name, &class_index, &class_index, NULL);
    	}
    }
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &name_count);    
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);

    if(NULL != show_str)
    {
        free(show_str);
        show_str = NULL;
    }
    return reply;
}

DBusMessage * npd_dbus_acl_show_pmap_list(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

 	unsigned int	name_count = 0;
	unsigned int	ret = MATCH_RETURN_CODE_SUCCESS;
    char *          show_str = NULL;
    struct policy_map_index_s	policy_index;
	DBusMessage*	reply = NULL;    
	DBusMessageIter	iter;

	memset(&policy_index, 0, sizeof(struct policy_map_index_s));

    show_str = (char *)malloc(SHOW_SERVICE_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, SHOW_SERVICE_SIZE);
    	ret = dbtable_hash_next(policy_map_name, NULL, &policy_index, NULL);
    	while (0 == ret)
    	{
            if ((0 != strncmp(policy_index.map_name, "SG_DEFAULT_DENY", sizeof(policy_index.map_name)))
                && (0 != strncmp(policy_index.map_name, "SG_DEFAULT", sizeof(policy_index.map_name)))
                && (0 != strncmp(policy_index.map_name, "SG_", 3))
                && (0 != strncmp(policy_index.map_name, "ACL_", 4))
                && (0 != strcmp(policy_index.map_name, "VOICE VLAN"))
#ifdef HAVE_PORTAL                 
                && (0 != strncmp(policy_index.map_name, "PORTAL_", 7))
#endif          
                )
            {
                char tmp[1024];
                memset(tmp, 0, 1024);
                sprintf(tmp, "%-15d%-40s%-15s\n", name_count, policy_index.map_name, policy_index.is_deployed?"applied":"not applied");
                strcat(show_str, tmp);
        		name_count++;
            }
    		ret = dbtable_hash_next(policy_map_name, &policy_index, &policy_index, NULL);
    	}
    }
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &name_count);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);        

    if(NULL != show_str)
    {
        free(show_str);
        show_str = NULL;
    }
	return reply;
}

DBusMessage * npd_dbus_acl_show_cmap_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	unsigned int	uni = 0;
	int 			ret = MATCH_RETURN_CODE_SUCCESS;
	const char*		map_name = NULL;
	char*			map_name_info[50];
	unsigned int	map_info_length = 0;
    unsigned int    etherTypeCurse = 0;
    int             haveIpv6 = 0;

	struct class_map_index_s class_index;
	struct class_map_rule_s class_rule;

	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
	
	memset(&class_index, 0, sizeof(struct class_map_index_s));
	memset(&class_rule, 0, sizeof(struct class_map_rule_s));

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,
					   DBUS_TYPE_STRING,&map_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	strcpy(class_index.map_name, map_name);
	ret = dbtable_hash_search(class_map_master_name, &class_index, NULL, &class_index);

	if (0 == ret)
	{
		map_name_info[map_info_length] = (char* )malloc(33);
		map_name_info[map_info_length][32] = 0;
		strncpy(map_name_info[map_info_length], class_index.map_name, 32);
		map_info_length++;

		map_name_info[map_info_length] = (char* )malloc(33);
		map_name_info[map_info_length][32] = 0;
		strncpy(map_name_info[map_info_length], ((class_index.is_binded == 0) ? "NO" : "YES"), 32);
		map_info_length++;
		
		strcpy(class_rule.map_name, class_index.map_name);
		class_rule.index = class_index.index;
	  	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
		while (0 == ret)
		{
			map_name_info[map_info_length] = (char* )malloc(33);
			map_name_info[map_info_length][32] = 0;
			strncpy(map_name_info[map_info_length], class_rule.cmd_name, 32);
			map_info_length++;

#ifdef HAVE_NPD_IPV6
			if (0 == strcmp("srcip6", class_rule.cmd_name)\
				 || 0 == strcmp("dstip6", class_rule.cmd_name))
			{
				char	arr_value[64];
				char	arr_mask[64];
                int maskLen = 0;
                
                char *data_arg = NULL;
                char *mask_arg = NULL;
                ip6_addr ip_data;
                ip6_addr ip_mask;

                memset(arr_value, 0, sizeof(arr_value));
                memset(arr_mask, 0, sizeof(arr_mask));
                memset(&ip_data, 0, sizeof(ip_data));
                memset(&ip_mask, 0, sizeof(ip_data));

                haveIpv6 = 1;
                
                memcpy(ip_data.u6_addr16, class_rule.cmd_arg, 16);
                memcpy(ip_mask.u6_addr16, class_rule.cmd_mask, 16);
                data_arg = lib_get_string_from_ipv6(arr_value, &ip_data);                        
                mask_arg = lib_get_string_from_ipv6(arr_mask, &ip_mask);
				map_name_info[map_info_length] = (char* )malloc(64);
                memset(map_name_info[map_info_length], 0, 64);
				strncpy(map_name_info[map_info_length], data_arg, 64);
				map_info_length++;

                lib_get_maskv6len_from_mask(&ip_mask, &maskLen);
				map_name_info[map_info_length] = (char* )malloc(64);
                memset(map_name_info[map_info_length], 0, 64);
                sprintf(map_name_info[map_info_length], "%d", maskLen);
				map_info_length++;
			}
			else
#endif
			if (0 == strcmp("srcip", class_rule.cmd_name)\
				 || 0 == strcmp("dstip", class_rule.cmd_name))
			{
				unsigned int value = 0;
				unsigned int mask = 0;
				char	arr_value[32];
				char	arr_mask[32];

				memset(arr_value, 0, 32);
				memset(arr_mask, 0, 32);
				
				memcpy(&value, class_rule.cmd_arg, 4);
				memcpy(&mask, class_rule.cmd_mask, 4);

				lib_get_string_from_ip(arr_value, (int)value);
				lib_get_string_from_ip(arr_mask, (int)mask);

				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], arr_value, 32);
				map_info_length++;
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], arr_mask, 32);
				map_info_length++;
			}
			else if (0 == strcmp("destination-address mac", class_rule.cmd_name)\
				 || 0 == strcmp("source-address mac", class_rule.cmd_name))
			{
				unsigned char	arr_value[6];
				unsigned char	arr_mask[6];
				char	mac_value[32];
				char	mac_mask[32];

				memset(arr_value, 0, 6);
				memset(arr_mask, 0, 6);

				memcpy(arr_value, class_rule.cmd_arg, 6);
				memcpy(arr_mask, class_rule.cmd_mask, 6);
				memcpy(mac_value, class_rule.cmd_arg, sizeof(class_rule.cmd_arg));
				memcpy(mac_mask, class_rule.cmd_mask, sizeof(class_rule.cmd_mask));

				sprintf(mac_value, "%02x:%02x:%02x:%02x:%02x:%02x",
									arr_value[0], arr_value[1], arr_value[2], arr_value[3], arr_value[4], arr_value[5]);

				sprintf(mac_mask, "%02x:%02x:%02x:%02x:%02x:%02x",
									arr_mask[0], arr_mask[1], arr_mask[2], arr_mask[3], arr_mask[4], arr_mask[5]);

				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], mac_value, 32);
				map_info_length++;
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], mac_mask, 32);
				map_info_length++;
			}
			else if (0 == strcmp("dstl4port", class_rule.cmd_name)\
					|| 0 == strcmp("srcl4port", class_rule.cmd_name))
			{
				unsigned int	value = 0;
				char	l4_port[32];
				char*	prt = l4_port;

				memset(l4_port, 0, sizeof(l4_port));
					
				memcpy(&value, class_rule.cmd_arg, 4);
				
				show_name_cmp_fun(ip_port_preset_arg, &prt, value);
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], l4_port, 32);
				map_info_length++;
			}
			else if (0 == strcmp("ethertype", class_rule.cmd_name))
			{
				unsigned int value = 0;
				char	ethertype_value[32];
				char*	prt = ethertype_value;

				memset(ethertype_value, 0, sizeof(ethertype_value));
				memcpy(&value, class_rule.cmd_arg, 4);
				
				show_name_cmp_fun(ethertype_preset_arg, &prt, value);
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], ethertype_value, 32);
                etherTypeCurse = map_info_length;
				map_info_length++;
			}
			else if (0 == strcmp("ip precedence", class_rule.cmd_name))
			{
				unsigned int value = 0;
				char	procedence_value[32];

				memset(procedence_value, 0, sizeof(procedence_value));
				memcpy(&value, class_rule.cmd_arg, 4);

				sprintf(procedence_value, "%u", value);
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], procedence_value, 32);
				map_info_length++;
			}
			else if (0 == strcmp("cos", class_rule.cmd_name))
			{
				unsigned int value = 0;
				char	cos_value[32];

				memset(cos_value, 0, sizeof(cos_value));
					
				memcpy(&value, class_rule.cmd_arg, 4);

				sprintf(cos_value, "%u", value);
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], cos_value, 32);
				map_info_length++;
			}
			else if (0 == strcmp("outer-vlan", class_rule.cmd_name))
			{
				unsigned int value = 0;
				char	vid_value[32];

				memset(vid_value, 0, sizeof(vid_value));
					
				memcpy(&value, class_rule.cmd_arg, 4);

				sprintf(vid_value, "%u", value);
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], vid_value, 32);
				map_info_length++;
			}
            else if (0 == strcmp("vlan-tag-num", class_rule.cmd_name))
            {
			    map_name_info[map_info_length] = (char* )malloc(33);
			    map_name_info[map_info_length][32] = 0;
                if(0 == *(unsigned int*)class_rule.cmd_arg)
                {
                    sprintf(map_name_info[map_info_length], "UNTAGGED");
                }
                else if(1 == *(unsigned int*)class_rule.cmd_arg)
                {
                    sprintf(map_name_info[map_info_length], "SINGLE-TAGGED");
                }
                else if(2 == *(unsigned int*)class_rule.cmd_arg)
                {
                    sprintf(map_name_info[map_info_length], "DOUBLE-TAGGED");
                    
                }

				map_info_length++;
            }
			else if (0 == strcmp("ip dscp", class_rule.cmd_name))
			{
				unsigned int value = 0;
				char	ipdscp[32];
				char*	prt = ipdscp;

				memset(ipdscp, 0, sizeof(ipdscp));
					
				memcpy(&value, class_rule.cmd_arg, 4);

				show_name_cmp_fun(dscp_preset_arg, &prt, value);
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], ipdscp, 32);
				map_info_length++;
			}
			else if (0 == strcmp("protocol", class_rule.cmd_name))
			{
				unsigned int value = 0;
				char		protocol_value[32];
				char*		prt = protocol_value;


				memset(protocol_value, 0, sizeof(protocol_value));
					
				memcpy(&value, class_rule.cmd_arg, 4);

				show_name_cmp_fun(ip_proto_preset_arg, &prt, value);

				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], protocol_value, 32);
				map_info_length++;
			}
			else if (0 == strcmp("ip tos", class_rule.cmd_name))
			{
				unsigned int	ip_tos = 0;
				unsigned int	ip_tos_mask = 0;
				char			tos_value[5];
				char			tos_mask[5];

				memcpy(&ip_tos, class_rule.cmd_arg, 4);
				memcpy(&ip_tos_mask, class_rule.cmd_mask, 4);

				sprintf(tos_value, "%u", ip_tos);
				sprintf(tos_mask, "%u", ip_tos_mask);
				
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], tos_value, 32);
				map_info_length++;
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], tos_mask, 32);
				map_info_length++;
			}
            else if(0 == strcmp("time-range", class_rule.cmd_name))
            {
                map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], class_rule.cmd_arg, 16);
				map_info_length++;
            }
            else if(0 == strcmp("l4port_range_src", class_rule.cmd_name))
            {
                map_name_info[map_info_length] = (char*)malloc(33);
                map_name_info[map_info_length][32] = 0;

                npd_show_l4port_range(class_rule.cmd_name, class_rule.cmd_arg, map_name_info[map_info_length]);
                map_info_length++;
            }
            else if(0 == strcmp("l4port_range_dst", class_rule.cmd_name))
            {
                map_name_info[map_info_length] = (char*)malloc(33);
                map_name_info[map_info_length][32] = 0;

                npd_show_l4port_range(class_rule.cmd_name, class_rule.cmd_arg, map_name_info[map_info_length]);
                map_info_length++;
            }
		  	ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
		}
        if(haveIpv6)
        {
            memset(map_name_info[etherTypeCurse], 0, 32);
            sprintf(map_name_info[etherTypeCurse], "ipv6");
        }
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &map_info_length);	
	for (uni = 0; uni < map_info_length; uni++)
	{
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &map_name_info[uni]);
	}
	
	for (uni = 0; uni < map_info_length; uni++)
	{
		free(map_name_info[uni]);
	}

	return reply;
}

DBusMessage * npd_dbus_acl_show_pmap_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	unsigned int	uni = 0;
	int 			ret = MATCH_RETURN_CODE_SUCCESS;
	unsigned int	eth_g_port = 0;
	const char*		map_name = NULL;
	char*			map_name_info[200];
	unsigned int	map_info_length = 0;

	struct class_map_index_s class_index;
	struct policy_map_index_s policy_index;
	struct policy_map_rule_s policy_rule;
	struct service_policy_s		service;
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
	
	memset(&class_index, 0, sizeof(struct class_map_index_s));
	memset(&policy_index, 0, sizeof(struct policy_map_index_s));
	memset(&policy_rule, 0, sizeof(struct policy_map_rule_s));
	memset(&service, 0, sizeof(struct service_policy_s));

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,
					   DBUS_TYPE_STRING,&map_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	strcpy(policy_index.map_name, map_name);
	ret = dbtable_hash_search(policy_map_name, &policy_index, NULL, &policy_index);

	if (0 == ret)
	{
		map_name_info[map_info_length] = (char* )malloc(33);
		map_name_info[map_info_length][32] = 0;
		strncpy(map_name_info[map_info_length], policy_index.map_name, 32);
		map_info_length++;

		map_name_info[map_info_length] = (char* )malloc(33);
		map_name_info[map_info_length][32] = 0;
		strncpy(map_name_info[map_info_length], ((policy_index.is_deployed  == 0) ? "NO" : "YES"), 32);
		map_info_length++;

		if (-1 != policy_index.class_map_index)
		{
			ret = dbtable_array_get(class_map_master, policy_index.class_map_index, &class_index);
			if (0 == ret)
			{
				map_name_info[map_info_length] = (char* )malloc(33);
				map_name_info[map_info_length][32] = 0;
				strncpy(map_name_info[map_info_length], class_index.map_name, 32);
				map_info_length++;

				strcpy(policy_rule.map_name, policy_index.map_name);
				policy_rule.index = policy_index.index;
	        	ret = dbtable_hash_head_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);

        		while (0 == ret)
        		{
        			if (0 == strcmp("drop", policy_rule.cmd_name))
        			{
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
                        if(*((unsigned int *)policy_rule.cmd_arg) == 1)
        				    strncpy(map_name_info[map_info_length], "drop", 32);
                        else
        				    strncpy(map_name_info[map_info_length], "transmit", 32);
        				map_info_length++;
        			}
                    else if(0 == strcmp("trap", policy_rule.cmd_name))
                    {
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], "trap", 32);
        				map_info_length++;
                    }
                    else if (0 == strcmp("policy_vlanadd", policy_rule.cmd_name))
        			{
                        unsigned int param0;                        
        				char	temp[32] = { 0 };
                        memcpy(&param0, policy_rule.cmd_arg, sizeof(unsigned int));                        
        				sprintf(temp, "%d", param0);
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], "vlanadd", 32);
        				map_info_length++;
                        map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], temp, 32);
        				map_info_length++;

        			}
        			else if ((0 == strcmp("mirror", policy_rule.cmd_name)) || (0 == strcmp("redirect", policy_rule.cmd_name)))
        			{
        				char	port[32];
        				memset(port, 0, 32);
        				memcpy(&eth_g_port, policy_rule.cmd_arg, 4);
        				parse_eth_index_to_name(eth_g_port, port);
        				
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], policy_rule.cmd_name, 32);
        				map_info_length++;
        				map_name_info[map_info_length] = (char* )malloc(33);
						map_name_info[map_info_length][32] = 0;
						strncpy(map_name_info[map_info_length], port, 32);
						map_info_length++;
        			}
                    else if (0 == strcmp("policy-route", policy_rule.cmd_name))
                    {
                        char ip_string[32];
                        memset(ip_string, 0, 32);
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], policy_rule.cmd_name, 32);
        				map_info_length++;

                        sprintf(ip_string, "%d.%d.%d.%d",
                                (unsigned char)policy_rule.cmd_arg[0], 
                                (unsigned char)policy_rule.cmd_arg[1], 
                                (unsigned char)policy_rule.cmd_arg[2], 
                                (unsigned char)policy_rule.cmd_arg[3]);
        				map_name_info[map_info_length] = (char* )malloc(33);
						map_name_info[map_info_length][32] = 0;
						strncpy(map_name_info[map_info_length], ip_string, 32);
						map_info_length++;
                    }
        			else if (0 == strcmp("qos-profile", policy_rule.cmd_name))
        			{
                        unsigned int param0;                        
        				char	temp[32] = { 0 };
                        memcpy(&param0, policy_rule.cmd_arg, sizeof(unsigned int));                        
        				sprintf(temp, "%d", param0);
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], "qos-profile", 32);
        				map_info_length++;
                        map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], temp, 32);
        				map_info_length++;
        			}
                    else if (0 == strcmp("policer", policy_rule.cmd_name))
                    {
                        unsigned int param0;                        
        				char	temp[32] = { 0 };
                        memcpy(&param0, policy_rule.cmd_arg, sizeof(unsigned int));                        
        				sprintf(temp, "%d", param0);
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], "policer", 32);
        				map_info_length++;
                        map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], temp, 32);
        				map_info_length++;
                    }
        			else if (0 == strcmp("police-simple", policy_rule.cmd_name))
        			{
        				unsigned int	cir = 0;
        				unsigned int	burst = 0;
        				unsigned int	confirmvalue = 0;
        				unsigned int	exceedvalue = 0;
        				unsigned int	confirm_flag = 0;
        				unsigned int	exceed_flag = 0;
        				unsigned int	value[7] = {0, 0, 0, 0, 0, 0, 0};
        				char	temp[32];
        				char	cmp_array[5][20] = {"drop", "transmit", "set-dscp", "set-cos", "set-precedence"};

        				memset(temp, 0, sizeof(temp));
        				memcpy(value, policy_rule.cmd_arg, sizeof(unsigned int) * 6);
        				
        				cir = value[0];
        				burst = value[1];
        				confirm_flag = value[2];
        				confirmvalue = value[3];
        				exceed_flag = value[4];
        				exceedvalue = value[5];

        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], policy_rule.cmd_name, 32);
        				map_info_length++;
        				
        				sprintf(temp, "%d", cir);
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], temp, 32);
        				map_info_length++;
        				
        				sprintf(temp, "%d", burst);
        				map_name_info[map_info_length] = (char* )malloc(33);
        				map_name_info[map_info_length][32] = 0;
        				strncpy(map_name_info[map_info_length], temp, 32);
        				map_info_length++;
				
        				if (0 != confirm_flag)
        				{
        					map_name_info[map_info_length] = (char* )malloc(33);
        					map_name_info[map_info_length][32] = 0;
        					strncpy(map_name_info[map_info_length], cmp_array[confirm_flag - 1], 32);
        					map_info_length++;

        					if (3 == confirm_flag || 4 == confirm_flag || 5 == confirm_flag)
        					{
        						memset(temp, 0, sizeof(temp));
        						sprintf(temp, "%u", confirmvalue);
        						map_name_info[map_info_length] = (char* )malloc(33);
        						map_name_info[map_info_length][32] = 0;
        						strncpy(map_name_info[map_info_length], temp, 32);
        						map_info_length++;
        					}
        				}
				
        				if (0 != exceed_flag)
        				{
        					map_name_info[map_info_length] = (char* )malloc(33);
        					map_name_info[map_info_length][32] = 0;
        					strncpy(map_name_info[map_info_length], "violate-action", 32);
        					map_info_length++;

        					map_name_info[map_info_length] = (char* )malloc(33);
        					map_name_info[map_info_length][32] = 0;
        					strncpy(map_name_info[map_info_length], cmp_array[exceed_flag - 1], 32);
        					map_info_length++;

        					if (3 == exceed_flag || 4 == exceed_flag || 5 == exceed_flag)
        					{
        						memset(temp, 0, sizeof(temp));
        						sprintf(temp, "%u", exceedvalue);
        						map_name_info[map_info_length] = (char* )malloc(33);
        						map_name_info[map_info_length][32] = 0;
        						strncpy(map_name_info[map_info_length], temp, 32);
        						map_info_length++;
        					}
						}
					}
					else if ((0 == strcmp("mark ip-dscp", policy_rule.cmd_name))
						|| (0 == strcmp("mark ip-precedence", policy_rule.cmd_name))
						|| (0 == strcmp("assign-queue", policy_rule.cmd_name))
						|| (0 == strcmp("mark-cos", policy_rule.cmd_name)))
					{
						char	temp_dscp[32];
						char*	prt = temp_dscp;

						map_name_info[map_info_length] = (char* )malloc(33);
						map_name_info[map_info_length][32] = 0;
						strncpy(map_name_info[map_info_length], policy_rule.cmd_name, 32);
						map_info_length++;

						memcpy(&eth_g_port, policy_rule.cmd_arg, 4);
						memset(temp_dscp, 0, sizeof(temp_dscp));
						show_name_cmp_fun(dscp_preset_arg, &prt, eth_g_port);
						map_name_info[map_info_length] = (char* )malloc(33);
						map_name_info[map_info_length][32] = 0;
						strncpy(map_name_info[map_info_length], temp_dscp, 32);
						map_info_length++;
					}
					else
					{
						break;
					}

					ret = dbtable_hash_next_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
				}

				strcpy(service.policy_map_name, policy_index.map_name);
				service.dir_type = 0;
				ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
                if (0 != ret)
                {
                    service.dir_type = 1;
				    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
                }
				if (0 == ret)
            	{
            		unsigned int eth_g_port = 0;
            		unsigned int array_id = 0;
                    unsigned int port_count = 0;
            		char	port[32];

                    if(!NPD_PBMP_IS_NULL(service.group))
                    {
                		memset(port, 0, sizeof(port));
                		map_name_info[map_info_length] = (char* )malloc(33);
                		map_name_info[map_info_length][32] = 0;
                		strncpy(map_name_info[map_info_length], "apply by port", 32);
                		map_info_length++;
                		map_name_info[map_info_length] = (char* )malloc(33);
                		map_name_info[map_info_length][32] = 0;
    
    					if(service.dir_type == ACL_DIRECTION_INGRESS_E)
                		    strncpy(map_name_info[map_info_length], "Ingress", 32);
    					if(service.dir_type == ACL_DIRECTION_EGRESS_E)
                		    strncpy(map_name_info[map_info_length], "Egress", 32);
    						
                		map_info_length++;
                    }

            		NPD_PBMP_ITER(service.group, array_id)
            		{
            			eth_g_port = eth_port_array_index_to_ifindex(array_id);	
                        if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(eth_g_port))
                        {
                			parse_eth_index_to_name(eth_g_port, port);
                			map_name_info[map_info_length] = (char* )malloc(33);
                			map_name_info[map_info_length][32] = 0;
                			strncpy(map_name_info[map_info_length], (char* )port, 32);
                			map_info_length++;
                            port_count++;
                        }
					}
                    /*acl only bind on slot, not port*/
                    if(0 == port_count)
                    {
                        /*should not include "Ingress" or "Egress"*/
                        map_info_length--;
                        free(map_name_info[map_info_length]);
                        map_info_length--;
                        /*should not include "apply by port"*/
                        free(map_name_info[map_info_length]);
                    }
				}
			}
		}
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &map_info_length);	
	for (uni = 0; uni < map_info_length; uni++)
	{
		dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &map_name_info[uni]);
	}
	
	for (uni = 0; uni < map_info_length; uni++)
	{
		free(map_name_info[uni]);
	}

	return reply;
}

DBusMessage * npd_dbus_acl_show_service_policy(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	unsigned int    index = 0;
    unsigned int    service_flag = 0; 
 	unsigned int	name_count = 0;
	unsigned int	ret = MATCH_RETURN_CODE_SUCCESS;
	unsigned int	eth_g_index = 0;
    unsigned int    eth_or_vlan_flag = 0;
    unsigned int    vid = 0;
    unsigned int    arr_index = 0;
 	char			tmp[128];    
    char*           show_str = NULL;
    char*           endptr = NULL;
    struct service_policy_s service_index;
	DBusMessage*	reply = NULL;    
	DBusMessageIter	iter;
	DBusError		err;
    int i;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
					   DBUS_TYPE_UINT32, &eth_g_index,
					   DBUS_TYPE_UINT32, &service_flag,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}

		return NULL;
	}
    show_str = (char *)malloc(SHOW_SERVICE_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, SHOW_SERVICE_SIZE);
        eth_or_vlan_flag = npd_netif_type_get(eth_g_index);
        switch(eth_or_vlan_flag)
        {
            case NPD_NETIF_TRUNK_TYPE:
                
            case NPD_NETIF_ETH_TYPE:
                arr_index = eth_port_array_index_from_ifindex(eth_g_index);
                break ;
            case NPD_NETIF_VLAN_TYPE:
                vid = npd_netif_vlan_get_vid(eth_g_index);
                break ;
            default:
                break ;
        }

        for(i = 0; i < dbtable_array_totalcount(service_policy_index); i++)
        {
            
    	    ret = dbtable_array_get(service_policy_index, i, &service_index);
            if(0 != ret)
                continue;
            if(NPD_NETIF_ETH_TYPE == eth_or_vlan_flag)
            {
                if(!NPD_PBMP_MEMBER(service_index.group, arr_index))
                    continue;
            }
            else if(NPD_NETIF_VLAN_TYPE == eth_or_vlan_flag)
            {
                if(!NPD_VBMP_MEMBER(service_index.vlanbmp, vid))
                    continue;
            }
            else if(NPD_NETIF_TRUNK_TYPE == eth_or_vlan_flag)
            {
                if(!NPD_PBMP_MEMBER(service_index.group, arr_index))
                    continue;
            }
            if(TRUE == npd_is_rule_in_acl_group(service_index.policy_index))
            {
                continue;
            }
            if(ACL_SERVICE == service_flag)
            {
                if(0 != strncmp("ACL_", service_index.policy_map_name, 4))
                {
                    continue;
                }
                memset(tmp, 0, sizeof(tmp));
                index = strtoul(&service_index.policy_map_name[4], &endptr, 10);
                sprintf(tmp, "%5s %6d %s\n", "ACL", index, service_index.dir_type? "Egress":"Ingress");
                strcat(show_str, tmp);
            }
            else
            {
                if((0 == strncmp("ACL_", service_index.policy_map_name, 4))
                    || (0 == strcmp("VOICE VLAN", service_index.policy_map_name))
                    || (0 == strncmp("SG_", service_index.policy_map_name, 3))
#ifdef HAVE_PORTAL                     
                    || (0 == strncmp("PORTAL_", service_index.policy_map_name, 7))
#endif
                   )
                {
                    continue;
                }
                memset(tmp, 0, sizeof(tmp));
                sprintf(tmp, "%5d %6s     %s\n", name_count, service_index.dir_type? "Egress":"Ingress", service_index.policy_map_name);
                strcat(show_str, tmp);
            }
    		name_count++;
        }
    }
	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &name_count);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);

    if(NULL != show_str)
    {
        free(show_str);
        show_str = NULL;
    }
    return reply;
}

char* voice_vlan_showrunning(char* showStr, int* safe_len)
{
    int ret;
	int		totalLen = 0;
	char*	cursor = showStr;
	struct class_map_index_s	class_index ;
    struct policy_map_index_s	policy_index ;
	struct policy_map_rule_s	policy_rule ;
    struct class_map_rule_s    class_rule ;
    char qosprofile[30] = {0};
    char vlanstr[30] = {0};
    char cosstr[15] = {0};
    int qos_remap = FALSE;

    memset(&class_index, 0, sizeof(class_index));
    memset(&policy_index, 0, sizeof(policy_index));
    memset(&policy_rule, 0, sizeof(policy_rule));
    memset(&class_rule, 0, sizeof(class_rule));
	
    sprintf(policy_index.map_name, "VOICE VLAN");

    ret = dbtable_hash_search(policy_map_name, &policy_index, NULL, &policy_index);
    if(ret != 0)
    {
		*safe_len = 0;
		return showStr;
    }
    ret = dbtable_array_get(class_map_master, policy_index.class_map_index, &class_index); 
	if (0 == ret)
	{
		strcpy(policy_rule.map_name, policy_index.map_name);
		policy_rule.index = policy_index.index;
		ret = dbtable_hash_head_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
		while (0 == ret)
		{
            if (0 == strcmp("qos-profile", policy_rule.cmd_name))
			{
                int id = *(unsigned int*)policy_rule.cmd_arg;

                qos_remap = TRUE;
                if(id != VOICE_VLAN_RSV_QOS_PROFILE_ID)
                {
                    sprintf(qosprofile, "qos-profile %d ", *(unsigned int*)policy_rule.cmd_arg);
					break;
                }
			}
			ret = dbtable_hash_next_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
		}
	}
    sprintf(class_rule.map_name, policy_index.map_name);
    class_rule.index = policy_index.class_map_index;
    ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
	while (0 == ret)
	{
		if (0 == strcmp("outer-vlan", class_rule.cmd_name))
		{
			if(*(unsigned int*)class_rule.cmd_mask != 0)
                sprintf(vlanstr, "vlan %d ", *(unsigned int*)class_rule.cmd_arg);
		}
		else if (0 == strcmp("vlan-tag-num", class_rule.cmd_name))
		{
			unsigned int value = 0;

            value = *(unsigned int*)class_rule.cmd_arg;

            if(0 == value) 
            {
                if(qos_remap)
                  sprintf(vlanstr, "vlan untagged");
                else
                  sprintf(vlanstr, "vlan none");
            }
		}
		else if (0 == strcmp("cos", class_rule.cmd_name))
		{
			unsigned int value = 0;

            value = *(unsigned int*)class_rule.cmd_arg;

            sprintf(cosstr, "vlan cos %d", value);

		}

	  	ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
	}
    if(strlen(vlanstr))
    {
        totalLen += sprintf(cursor, "voice %s\n", vlanstr);
        cursor = showStr + totalLen;
    }
    if(strlen(cosstr))
    {
        totalLen += sprintf(cursor, "voice %s\n", cosstr);
        cursor = showStr + totalLen;
    }
    if(strlen(qosprofile))
    {
        totalLen += sprintf(cursor, "voice vlan %s\n", qosprofile);
        cursor = showStr + totalLen;
    }
    totalLen += sprintf(cursor, "\n");
    *safe_len = totalLen;
    return showStr;
        
}
char* acl_rule_show_running_config(char* showStr, int* safe_len)
{
	char*	cursor = NULL;
	int 	totalLen = 0;
    int ret;
    unsigned int eth_g_port;
    char port_name[16]={0};
    char *idstr = malloc(20);
    char *action = malloc(50);
    char *ethertype = malloc(50);
    char *ipprotocol = malloc(50);
    char *dstipstr = malloc(200);
    char *srcipstr = malloc(200);
    char *dstl4portstr = malloc(50);
    char *srcl4portstr = malloc(50);
    char *dstmacstr = malloc(50);
    char *srcmacstr = malloc(50);
    char *vlan_idstr = malloc(50);
    char *policerIdstr = malloc(50);
    char *redirctPortstr = malloc(50);
    char *profileIdstr = malloc(50);
    int totalcount = dbtable_array_totalcount(policy_map_index);
    int i;

	struct class_map_index_s	class_index ;
    struct policy_map_index_s	policy_index ;
	struct policy_map_rule_s	policy_rule ;
    struct class_map_rule_s    class_rule ;
    memset(&class_index, 0, sizeof(class_index));
    memset(&policy_index, 0, sizeof(policy_index));
    memset(&policy_rule, 0, sizeof(policy_rule));
    memset(&class_rule, 0, sizeof(class_rule));

    if( !idstr||!action||!ethertype||!ipprotocol||!dstipstr||!srcipstr||!dstl4portstr||!profileIdstr
        ||!srcl4portstr||!dstmacstr||!srcmacstr||!vlan_idstr||!policerIdstr||!redirctPortstr)
    {
        *safe_len = 0;
		if(idstr)
		{
            free(idstr);
		}
		if(action)
		{
            free(action);
		}
		if(ethertype)
		{
            free(ethertype);
		}
		if(ipprotocol)
		{
            free(ipprotocol);
		}
		if(dstipstr)
		{
            free(dstipstr);
		}
		if(srcipstr)
		{
            free(srcipstr);
		}
		if(dstl4portstr)
		{
            free(dstl4portstr);
		}
		if(srcl4portstr)
		{
            free(srcl4portstr);
		}
		if(dstmacstr)
		{
            free(dstmacstr);
		}
		if(srcmacstr)
		{
            free(srcmacstr);
		}
		if(vlan_idstr)
		{
            free(vlan_idstr);
		}
		if(policerIdstr)
		{
            free(policerIdstr);
		}
		if(redirctPortstr)
		{
            free(redirctPortstr);
		}
		if(profileIdstr)
		{
            free(profileIdstr);
		}
        return showStr;
    }
    memset(idstr, 0, 20);
    memset(action, 0, 50);
    memset(ethertype, 0, 50);
    memset(ipprotocol, 0, 50);
    memset(dstipstr, 0, 200);
    memset(srcipstr, 0, 200);
    memset(dstl4portstr, 0, 50);
    memset(srcl4portstr, 0, 50);
    memset(dstmacstr, 0, 50);
    memset(srcmacstr, 0, 50);
    memset(vlan_idstr, 0, 50);
    memset(policerIdstr, 0, 50);
    memset(redirctPortstr, 0, 50);
    memset(profileIdstr, 0, 50);

	cursor = showStr;

//    ret = dbtable_hash_next(policy_map_name, NULL, &policy_index, NULL);
    for (i = 0; i < totalcount; i++)
	{
		if (*safe_len < (totalLen + 1000))
		{
			break;
		}
        ret = dbtable_array_get(policy_map_index, i, &policy_index);
        if(0 != ret)
            continue;
        if(0 != strncmp(policy_index.map_name, "ACL_", 4))
        {
//		    ret = dbtable_hash_next(policy_map_name, &policy_index, &policy_index, NULL);
            continue;
        }
        
        sprintf(idstr, "acl %s ", &policy_index.map_name[4]);
        
		ret = dbtable_array_get(class_map_master, policy_index.class_map_index, &class_index); 
		if (0 == ret)
		{
			strcpy(policy_rule.map_name, policy_index.map_name);
			policy_rule.index = policy_index.index;
			ret = dbtable_hash_head_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
			while (0 == ret)
			{
				if (0 == strcmp("drop", policy_rule.cmd_name))
				{
                    if(*((int*)policy_rule.cmd_arg) == 1)
					    sprintf(action, "deny ");
                    else
                        sprintf(action, "permit ");
				}
                else if(0 == strcmp("trap", policy_rule.cmd_name))
                {
                    sprintf(action, "trap ");
                }

                if ((0 == strcmp("mirror", policy_rule.cmd_name)) || (0 == strcmp("redirect", policy_rule.cmd_name)))
				{
					eth_g_port = *(unsigned int*)(policy_rule.cmd_arg);
					parse_eth_index_to_name(eth_g_port, (unsigned char*)port_name);
                    sprintf(action, "%s %s ", policy_rule.cmd_name, port_name); 
				}
				else if (0 == strcmp("qos-profile", policy_rule.cmd_name))
				{
                    sprintf(profileIdstr, "qos-profile %d ", *(unsigned int*)policy_rule.cmd_arg);
				}
				else if (0 == strcmp("policer", policy_rule.cmd_name))
				{
                    sprintf(policerIdstr, "policer %d ", *(unsigned int*)policy_rule.cmd_arg);
				}
                else if (0 == strcmp("policy-route", policy_rule.cmd_name))
                {
                    sprintf(action, "policy-route nexthop %d.%d.%d.%d ", (unsigned char)policy_rule.cmd_arg[0], (unsigned char)policy_rule.cmd_arg[1], (unsigned char)policy_rule.cmd_arg[2], (unsigned char)policy_rule.cmd_arg[3]);
                }
				ret = dbtable_hash_next_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
			}
		}
        sprintf(class_rule.map_name, policy_index.map_name);
        class_rule.index = policy_index.class_map_index;
        ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
		while (0 == ret)
		{
            if (0 == strcmp("vlan-tag-num", class_rule.cmd_name))
            {
                if(*(unsigned int *)class_rule.cmd_arg == 0)
                {
                    sprintf(vlan_idstr, "vlan untagged ");
                }
            }
			if (0 == strcmp("outer-vlan", class_rule.cmd_name))
			{
				if(*(unsigned int*)class_rule.cmd_mask == 0)
                    sprintf(vlan_idstr, "vlan any ");
                else
                    sprintf(vlan_idstr, "vlan %d ", *(unsigned int*)class_rule.cmd_arg);
			}
			else if (0 == strcmp("srcip", class_rule.cmd_name)\
				 || 0 == strcmp("dstip", class_rule.cmd_name))
			{
				unsigned int value = 0;
				unsigned int mask = 0;
                int masklen = 0;
				char	arr_value[32];
				char	arr_mask[32];

				memset(&arr_value, 0, 32);
				memset(&arr_mask, 0, 32);
				
				memcpy(&value, class_rule.cmd_arg, 4);
				memcpy(&mask, class_rule.cmd_mask, 4);

				lib_get_string_from_ip(arr_value, (int)value);
                lib_get_masklen_from_mask(mask, &masklen);

     			if (0 == strcmp("srcip", class_rule.cmd_name))
     			{
                    if(0 == mask)
                        sprintf(srcipstr, "sip any ");
                    else
                        sprintf(srcipstr, "sip %s/%d ", arr_value, masklen);
     			}
                if(0 == strcmp("dstip", class_rule.cmd_name))
                {
                    if(0 == mask)
                        sprintf(dstipstr, "dip any ");
                    else
                        sprintf(dstipstr, "dip %s/%d ", arr_value, masklen);
                }
			}
#ifdef HAVE_NPD_IPV6
            else if(0 == strcmp("srcip6", class_rule.cmd_name)\
				 || 0 == strcmp("dstip6", class_rule.cmd_name))
            {
                int     maskLen = 0;
				char	strData[128];
				char	strMask[128];
                ip6_addr ip6Data;
                ip6_addr ip6Mask;
                if(TRUE == npd_class_map_have_dst_src_ip6(policy_index.class_map_index))
                {
                    memset(&ip6Data, 0, sizeof(&ip6Data));
                    memset(&ip6Mask, 0, sizeof(&ip6Mask));
                    memset(strData, 0, sizeof(strData));
                    memset(strMask, 0, sizeof(strMask));
                    memset(idstr, 0, sizeof(idstr));
                    sprintf(idstr, "acl ipv6 %s ", &policy_index.map_name[4]);

    				memcpy(ip6Data.u6_addr16, class_rule.cmd_arg, 16);
    				memcpy(ip6Mask.u6_addr16, class_rule.cmd_mask, 16);

                    lib_get_string_from_ipv6(strData, &ip6Data);
                    lib_get_string_from_ipv6(strMask, &ip6Mask);
                    lib_get_maskv6len_from_mask(&ip6Mask, &maskLen);

         			if (0 == strcmp("srcip6", class_rule.cmd_name))
         			{
                        if(0 == maskLen)
                            sprintf(srcipstr, "sip any ");
                        else
                            sprintf(srcipstr, "sip %s/%d ", strData, maskLen);
         			}
                    if(0 == strcmp("dstip6", class_rule.cmd_name))
                    {
                        if(0 == maskLen)
                            sprintf(dstipstr, "dip any ");
                        else
                            sprintf(dstipstr, "dip %s/%d ", strData, maskLen);
                    }
                }
            }
#endif
			else if (0 == strcmp("destination-address mac", class_rule.cmd_name)\
				 || 0 == strcmp("source-address mac", class_rule.cmd_name))
			{
				unsigned char	arr_value[6];
				unsigned char	arr_mask[6];

				memset(&arr_value, 0, 6);
				memset(&arr_mask, 0, 6);

				memcpy(&arr_value, class_rule.cmd_arg, 6);
				memcpy(&arr_mask, class_rule.cmd_mask, 6);

                
                if (0 == strcmp("destination-address mac", class_rule.cmd_name))
                {
                    int k;
                    for(k = 0; k < 6; k++)
                    {
                        if(arr_mask[k] != 0)
                            break;
                    }

                    if(k == 6)
                        sprintf(dstmacstr, "dmac any ");
                    else
                        sprintf(dstmacstr, "dmac %02x:%02x:%02x:%02x:%02x:%02x ", 
 									arr_value[0], arr_value[1], arr_value[2], arr_value[3], arr_value[4], arr_value[5]);
                }
				if(0 == strcmp("source-address mac", class_rule.cmd_name))
				{
                    int k;
                    for(k = 0; k < 6; k++)
                    {
                        if(arr_mask[k] != 0)
                            break;
                    }

                    if(k == 6)
                        sprintf(srcmacstr, "smac any ");
                    else
                        sprintf(srcmacstr, "smac %02x:%02x:%02x:%02x:%02x:%02x ", 
 									arr_value[0], arr_value[1], arr_value[2], arr_value[3], arr_value[4], arr_value[5]);
				}
			}
			else if (0 == strcmp("ethertype", class_rule.cmd_name))
			{
				unsigned int value = 0;

                memcpy(&value, class_rule.cmd_arg, 4);

                if(value == 0x0806)
                    sprintf(ethertype, "arp ");
                else if(value == 0x0800)
                {
                    if(strlen(ipprotocol) == 0)
                        sprintf(ipprotocol, "ip ");
                }
                else if(value != 0)
                    sprintf(ethertype, "ethertype 0x%04x ", value);
			}
			else if (0 == strcmp("protocol", class_rule.cmd_name))
			{
				unsigned int value = 0;
							
				memcpy(&value, class_rule.cmd_arg, 4);
                if(value == 17)
                    sprintf(ipprotocol, "udp ");
                if(value == 6)
                    sprintf(ipprotocol, "tcp ");
                if(value == 1)
                    sprintf(ipprotocol, "icmp ");
			}
			else if (0 == strcmp("dstl4port", class_rule.cmd_name)\
				 || 0 == strcmp("srcl4port", class_rule.cmd_name))
			{
				unsigned int	value = *(unsigned int*)class_rule.cmd_arg;

                if (0 == strcmp("dstl4port", class_rule.cmd_name))
                {
                    if(*(unsigned int*)class_rule.cmd_mask == 0)
                        sprintf(dstl4portstr, "dst-port any ");
                    else
                        sprintf(dstl4portstr, "dst-port %d ", value);
                }
				else if(0 == strcmp("srcl4port", class_rule.cmd_name))
				{
                    if(*(unsigned int*)class_rule.cmd_mask == 0)
                        sprintf(srcl4portstr, "src-port any ");
                    else
                        sprintf(srcl4portstr, "src-port %d ", value);
				}
			}

		  	ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
	    }
        if(strlen(dstmacstr) != 0)
        {
			if(0 == strncmp(ethertype, "arp", 3))
				sprintf(ethertype, "ethertype 0x0806 ");
			if(0 == strncmp(ipprotocol, "ip", 2))
			{
                memset(ipprotocol, 0, 50);
				sprintf(ethertype, "ethertype 0x0800 ");
			}
        }
        totalLen += sprintf(cursor, "%s%s%s%s%s%s%s%s%s%s%s%s\n",
                          idstr, action, ethertype, ipprotocol, dstipstr, dstl4portstr, srcipstr, srcl4portstr,
                          dstmacstr, srcmacstr, vlan_idstr, policerIdstr);
		cursor = showStr + totalLen;

        if(strlen(profileIdstr))
        {
            totalLen += sprintf(cursor, "%s%s\n", idstr, profileIdstr);
            cursor = showStr + totalLen;
        }
		
//		ret = dbtable_hash_next(policy_map_name, &policy_index, &policy_index, NULL);
        memset(idstr, 0, 20);
        memset(action, 0, 50);
        memset(ethertype, 0, 50);
        memset(ipprotocol, 0, 50);
        memset(dstipstr, 0, 200);
        memset(srcipstr, 0, 200);
        memset(dstl4portstr, 0, 50);
        memset(srcl4portstr, 0, 50);
        memset(dstmacstr, 0, 50);
        memset(srcmacstr, 0, 50);
        memset(vlan_idstr, 0, 50);
        memset(policerIdstr, 0, 50);
        memset(redirctPortstr, 0, 50);
        memset(profileIdstr, 0, 50);        
	}

    totalLen += sprintf(cursor, "\n");

	*safe_len = totalLen;
    free(idstr);
    free(action);
    free(ethertype);
    free(ipprotocol);
    free(dstipstr);
    free(srcipstr);
    free(dstl4portstr);
    free(srcl4portstr);
    free(dstmacstr);
    free(srcmacstr);
    free(vlan_idstr);
    free(policerIdstr);
    free(redirctPortstr);
    free(profileIdstr);
	return showStr;    

}
char* acl_match_show_running_config(char* showStr, int* safe_len)
{
	char*	cursor = NULL;
	int 	ret = 0;
    int     i = 0;
	int 	totalLen = 0;
    int     class_totalcount = 0;
    struct class_map_index_s class_index;
	struct class_map_rule_s class_rule;

	memset(&class_index, 0, sizeof(struct class_map_index_s));
	memset(&class_rule, 0, sizeof(struct class_map_rule_s));

	cursor = showStr;

	if (*safe_len < (totalLen + 20))
	{
		*safe_len = totalLen;
		return showStr;
	}
    class_totalcount = dbtable_array_totalcount(class_map_master);
    for(i = 0; i < class_totalcount; i++)
    {
        ret = dbtable_array_get(class_map_master, i, &class_index);
    	if (0 == ret)
    	{
    		if (*safe_len < (totalLen + 1000))
    		{
    			break;
    		}
    	    if((!strncmp(class_index.map_name, "ACL_", 4))
                || !(strcmp(class_index.map_name, "VOICE VLAN"))
                || !(strncmp(class_index.map_name, "SERV_TC", strlen("SERV_TC")))
                || !(strncmp(class_index.map_name, "IPV6_", 5))			
                || !(strncmp(class_index.map_name, "SG_", strlen("SG_")))
#ifdef HAVE_PORTAL                 
                || !(strncmp(class_index.map_name, "PORTAL_", strlen("PORTAL_")))
#endif
              )
    	    {
                continue;
    	    }
    		totalLen += sprintf(cursor, "class-map match-all %s\n", class_index.map_name);
    		cursor = showStr + totalLen;
    		
    		strcpy(class_rule.map_name, class_index.map_name);
            class_rule.index = class_index.index;
            ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
            while (0 == ret)
            {
			    if (0 == strcmp("outer-vlan", class_rule.cmd_name))
    			{
    				totalLen += sprintf(cursor, " match %s", "vlan-id");
    			}
                else if((0 == strcmp("l4port_range_src", class_rule.cmd_name)) 
                        ||(0 == strcmp("l4port_range_dst", class_rule.cmd_name)))
                {
                    totalLen += sprintf(cursor, " match ");
                }
    			else
    			{
    				totalLen += sprintf(cursor, " match %s", class_rule.cmd_name);
    			}
    			cursor = showStr + totalLen;

    			if (0 == strcmp("srcip", class_rule.cmd_name)\
    				 || 0 == strcmp("dstip", class_rule.cmd_name))
    			{
    				unsigned int value = 0;
    				unsigned int mask = 0;
    				char	arr_value[32];
    				char	arr_mask[32];

    				memset(&arr_value, 0, 32);
    				memset(&arr_mask, 0, 32);
    				
    				memcpy(&value, class_rule.cmd_arg, 4);
    				memcpy(&mask, class_rule.cmd_mask, 4);

    				lib_get_string_from_ip(arr_value, (int)value);
    				lib_get_string_from_ip(arr_mask, (int)mask);

    				totalLen += sprintf(cursor, " %s %s\n", arr_value, arr_mask);
    				cursor = showStr + totalLen;								
    			}
    			else if (0 == strcmp("destination-address mac", class_rule.cmd_name)\
    				 || 0 == strcmp("source-address mac", class_rule.cmd_name))
    			{
    				unsigned char	arr_value[6];
    				unsigned char	arr_mask[6];

    				memset(&arr_value, 0, 6);
    				memset(&arr_mask, 0, 6);

    				memcpy(&arr_value, class_rule.cmd_arg, 6);
    				memcpy(&arr_mask, class_rule.cmd_mask, 6);

    				totalLen += sprintf(cursor, " %02x:%02x:%02x:%02x:%02x:%02x",
    									arr_value[0], arr_value[1], arr_value[2], arr_value[3], arr_value[4], arr_value[5]);
    				cursor = showStr + totalLen;

    				totalLen += sprintf(cursor, " %02x:%02x:%02x:%02x:%02x:%02x\n",
    									arr_mask[0], arr_mask[1], arr_mask[2], arr_mask[3], arr_mask[4], arr_mask[5]);
    				cursor = showStr + totalLen;
    			}
    			else if (0 == strcmp("ethertype", class_rule.cmd_name))
    			{
    				unsigned int value = 0;
    				
    				memcpy(&value, class_rule.cmd_arg, 4);

    				totalLen += showrunning_name_cmp_fun(ethertype_preset_arg, &cursor, value);
    				cursor = showStr + totalLen;
    			}
    			else if (0 == strcmp("dstl4port", class_rule.cmd_name)\
    				 || 0 == strcmp("srcl4port", class_rule.cmd_name))
    			{
    				unsigned int	value = 0;
    							
    				memcpy(&value, class_rule.cmd_arg, 4);

    				totalLen += showrunning_name_cmp_fun(ip_port_preset_arg, &cursor, value);
    				cursor = showStr + totalLen;
    			}
    			else if (0 == strcmp("cos", class_rule.cmd_name)\
    				 || 0 == strcmp("ip precedence", class_rule.cmd_name))
    			{
    				unsigned int value = 0;
    				
    				memcpy(&value, class_rule.cmd_arg, 4);

    				totalLen += sprintf(cursor, " %u\n", value);
    				cursor = showStr + totalLen;
    			}
    			else if (0 == strcmp("ip dscp", class_rule.cmd_name))
    			{
    				unsigned int value = 0;
    				
    				memcpy(&value, class_rule.cmd_arg, 4);

    				totalLen += showrunning_name_cmp_fun(dscp_preset_arg, &cursor, value);
    				cursor = showStr + totalLen;
    			}
    			else if (0 == strcmp("protocol", class_rule.cmd_name))
    			{
    				unsigned int value = 0;
    							
    				memcpy(&value, class_rule.cmd_arg, 4);

    				totalLen += showrunning_name_cmp_fun(ip_proto_preset_arg, &cursor, value);
    				cursor = showStr + totalLen;
    			}
    			else if (0 == strcmp("ip tos", class_rule.cmd_name))
    			{
    				unsigned int	ip_tos = 0;
    				unsigned int	ip_tos_mask = 0;

    				memcpy(&ip_tos, class_rule.cmd_arg, 4);
    				memcpy(&ip_tos_mask, class_rule.cmd_mask, 4);

    				totalLen += sprintf(cursor, " %u %u\n", ip_tos, ip_tos_mask);
    				cursor = showStr + totalLen;
    			}
    			else if (0 == strcmp("outer-vlan", class_rule.cmd_name))
    			{
    				unsigned int	vid = 0;

    				memcpy(&vid, class_rule.cmd_arg, 4);

    				totalLen += sprintf(cursor, " %u\n", vid);
    				cursor = showStr + totalLen;
    			}	
				else if (0 == strcmp("tcp-flag", class_rule.cmd_name))
				{
					char flagstr[5] = {0};
					switch(*(unsigned int*)class_rule.cmd_arg)
					{
						case 0x1:
							sprintf(flagstr, "fin");
							break;
						case 0x2:
							sprintf(flagstr, "syn");
							break;
						case 0x4:
							sprintf(flagstr, "rst");
							break;
					    case 0x8:
							sprintf(flagstr, "psh");
							break;
						case 0x10:
							sprintf(flagstr, "ack");
							break;
						case 0x20:
							sprintf(flagstr, "urg");
							break;
						default:
							break;
					}
					totalLen += sprintf(cursor, " %s\n", flagstr);
					cursor = showStr + totalLen;
				}
				else if (0 == strcmp("icmp-type", class_rule.cmd_name))
				{
					totalLen += sprintf(cursor, " %u\n", *(unsigned int*)class_rule.cmd_arg);
					cursor = showStr + totalLen;
				}
				else if (0 == strcmp("icmp-code", class_rule.cmd_name))
				{
					totalLen += sprintf(cursor, " %u\n", *(unsigned int*)class_rule.cmd_arg);
					cursor = showStr + totalLen;
				}
                else if (0 == strcmp("l4port_range_src", class_rule.cmd_name))
                {
                    totalLen +=  showruning_l4port_range_fun(class_rule.cmd_name, class_rule.cmd_arg, cursor);
                    cursor = showStr + totalLen;
                }
                else if (0 == strcmp("l4port_range_dst", class_rule.cmd_name))
                {
                    totalLen +=  showruning_l4port_range_fun(class_rule.cmd_name, class_rule.cmd_arg, cursor);
                    cursor = showStr + totalLen;
                }
                else if (0 == strcmp("time-range", class_rule.cmd_name))
                {
                    totalLen += sprintf(cursor, " %s\n", class_rule.cmd_arg);
                    cursor = showStr + totalLen;
                }

		  	    ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
    	    }

    		totalLen += sprintf(cursor, "exit\n");
    		cursor = showStr + totalLen;
    	}
    }
	*safe_len = totalLen;

	return showStr;
}

char* acl_action_show_running_config(char* showStr, int* safe_len)
{
	char*	cursor = NULL;
	int 	ret = 0;
	int 	totalLen = 0;
    int     action_i = 0;
	unsigned int	eth_g_port = 0;
	struct class_map_index_s	class_index;
    struct policy_map_index_s	policy_index;
	struct policy_map_rule_s	policy_rule;
    int action_totalcount;

	memset(&class_index, 0, sizeof(struct class_map_index_s));
	memset(&policy_index, 0, sizeof(struct policy_map_index_s));
	memset(&policy_rule, 0, sizeof(struct policy_map_rule_s));

	cursor = showStr;
    action_totalcount = dbtable_array_totalcount(class_map_master);
    for(action_i = 0; action_i < action_totalcount; action_i++)
    {
        ret = dbtable_array_get(policy_map_index, action_i, &policy_index);
    	if (0 == ret)
    	{
    		if (*safe_len < (totalLen + 1000))
    		{
    			break;
    		}
    	    if(!strncmp(policy_index.map_name, "ACL_", 4)
                || !strcmp(policy_index.map_name, "VOICE VLAN")
                || !strncmp(policy_index.map_name, "SERV_TC", strlen("SERV_TC"))
    			|| !(strncmp(policy_index.map_name, "IPV6_", 5))
                || !strncmp(policy_index.map_name, "SG_", strlen("SG_"))
#ifdef HAVE_PORTAL                 
                || !strncmp(policy_index.map_name, "PORTAL_", strlen("PORTAL_"))
#endif                
              )
    	    {
                continue;
    	    }

    		totalLen += sprintf(cursor, "policy-map %s\n", policy_index.map_name);
    		cursor = showStr + totalLen;

    		ret = dbtable_array_get(class_map_master, policy_index.class_map_index, &class_index); 
    		if (0 == ret)
    		{
    			totalLen += sprintf(cursor, " class-map %s\n", class_index.map_name);
    			cursor = showStr + totalLen;

    			strcpy(policy_rule.map_name, policy_index.map_name);
    			policy_rule.index = policy_index.index;
    			ret = dbtable_hash_head_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
    			while (0 == ret)
    			{
    				if (0 == strcmp("drop", policy_rule.cmd_name))
    				{
    					totalLen += sprintf(cursor, " drop\n");
    				}
    				else if ((0 == strcmp("mirror", policy_rule.cmd_name)) || (0 == strcmp("redirect", policy_rule.cmd_name)))
    				{
    					char	port[32];
    					memset(port, 0, 32);
    					memcpy(&eth_g_port, policy_rule.cmd_arg, 4);
    					parse_eth_index_to_name(eth_g_port, port);
    					totalLen += sprintf(cursor, " %s %s\n", policy_rule.cmd_name, port);
    				}
    				else if (0 == strcmp("mark ip-dscp", policy_rule.cmd_name))
    				{
    					memcpy(&eth_g_port, policy_rule.cmd_arg, 4);
    					totalLen += sprintf(cursor, " %s ", policy_rule.cmd_name);
    					cursor = showStr + totalLen;
    					totalLen += showrunning_name_cmp_fun(dscp_preset_arg, &cursor, eth_g_port);
    				}
    				else if (0 == strcmp("police-simple", policy_rule.cmd_name))
    				{
    					unsigned int	cir = 0;
    					unsigned int	burst = 0;
    					unsigned int	confirmvalue = 0;
    					unsigned int	exceedvalue = 0;
    					unsigned int	confirm_flag = 0;
    					unsigned int	exceed_flag = 0;
    					unsigned int	value[7] = {0, 0, 0, 0, 0, 0, 0};
    					char	cmp_array[5][20] = {"drop", "transmit", "set-dscp", "set-cos", "set-precedence"};
    					
    					memcpy(value, policy_rule.cmd_arg, sizeof(unsigned int) * 6);
    					
    					cir = value[0];
    					burst = value[1];
    					confirm_flag = value[2];
    					confirmvalue = value[3];
    					exceed_flag = value[4];
    					exceedvalue = value[5];

    					totalLen += sprintf(cursor, " %s cir %u cbs %u conform-action", policy_rule.cmd_name, cir, burst);
    					cursor = showStr + totalLen;
    					
    					if (0 != confirm_flag)
    					{
    						totalLen += sprintf(cursor, " %s", cmp_array[confirm_flag - 1]);
    						cursor = showStr + totalLen;

    						if (3 == confirm_flag || 4 == confirm_flag || 5 == confirm_flag)
    						{
    							totalLen += sprintf(cursor, " %u", confirmvalue);
    							cursor = showStr + totalLen;
    						}
    					}
    					
    					if (0 != exceed_flag)
    					{
    						totalLen += sprintf(cursor, " violate-action %s", cmp_array[exceed_flag - 1]);
    						cursor = showStr + totalLen;

    						if (3 == exceed_flag || 4 == exceed_flag || 5 == exceed_flag)
    						{
    							totalLen += sprintf(cursor, " %u\n", exceedvalue);
    							cursor = showStr + totalLen;
    						}
    						else if (1 == exceed_flag || 2 == exceed_flag)
    						{
    							totalLen += sprintf(cursor, "\n");
    							cursor = showStr + totalLen;
    						}
    					}
    					else
    					{
    						totalLen += sprintf(cursor, "\n");
    						cursor = showStr + totalLen;
    					}
    				}
    				else
    				{
    					memcpy(&eth_g_port, policy_rule.cmd_arg, 4);
    					totalLen += sprintf(cursor, " %s %u\n", policy_rule.cmd_name, eth_g_port);
    				}
    				
    				cursor = showStr + totalLen;
    				ret = dbtable_hash_next_key(policy_map_set_rule, &policy_rule, &policy_rule, &policy_map_rule_filter);
    			}
    			totalLen += sprintf(cursor, "exit\n");
    			cursor = showStr + totalLen;
    		}

    		totalLen += sprintf(cursor, "exit\n");
    		cursor = showStr + totalLen;
    	}
    }
	*safe_len = totalLen;

	return showStr;
}

int npd_time_range_search_by_name(char *name)
{
    int ret;
    struct time_range_info_s time_range_info;
    memset(&time_range_info, 0, sizeof(struct time_range_info_s));
    strncpy(time_range_info.name, name, 16);
    ret = dbtable_hash_search(time_range_info_name,&time_range_info,NULL,&time_range_info);
    if(0 != ret)
            return TIME_RANGE_RETURN_NAME_NOEXIST;
    return MATCH_RETURN_CODE_SUCCESS;

}

DBusMessage * npd_dbus_time_range_info_search_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret;
	char	*name = NULL;

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
					   DBUS_TYPE_STRING, &name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	diffserv_check_golbal_start();

    ret = npd_time_range_search_by_name(name);
    
	diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);

    return reply;
}

int npd_abs_time_range_create(char *name, char *start_time, char *end_time)
{
    char *buf = NULL;
    unsigned int ret = 0;
    struct tm tm_start;
    struct tm tm_end;
    struct time_range_info_s time_range_info;
    strncpy(time_range_info.name, name, 16);
    
    ret = dbtable_hash_search(time_range_info_name,&time_range_info,NULL,&time_range_info);
    if(ret == 0)
    {
        if(time_range_info.abs_time.flag == TIME_RANGE_ABS_TIME)
        {
            return TIME_RANGE_RETURN_ABS_TIME_HAS_EXISTED;
        }
    }
    else
    {
        time_range_info.acl_bind_count = 0;
        time_range_info.periodic_time.flag = 0;
    }
    buf = (char *)strptime(start_time, "%Y/%m/%d %H:%M:%S", &tm_start);
    if(buf != NULL)
    {
        time_range_info.abs_time.start_time.year = tm_start.tm_year + 1900;
        time_range_info.abs_time.start_time.month = tm_start.tm_mon + 1;
        time_range_info.abs_time.start_time.day = tm_start.tm_mday;
        time_range_info.abs_time.start_time.hh = tm_start.tm_hour;
        time_range_info.abs_time.start_time.mm = tm_start.tm_min;
    }
    else
    {
        return TIME_RANGE_RETURN_TIME_BUF_NULL;
    }
    
    buf = (char *)strptime(end_time, "%Y/%m/%d %H:%M:%S", &tm_end);
    if(buf != NULL)
    {
        time_range_info.abs_time.end_time.year = tm_end.tm_year + 1900;
        time_range_info.abs_time.end_time.month = tm_end.tm_mon + 1;
        time_range_info.abs_time.end_time.day = tm_end.tm_mday;
        time_range_info.abs_time.end_time.hh = tm_end.tm_hour;
        time_range_info.abs_time.end_time.mm = tm_end.tm_min;
    }
    else
    {
        return TIME_RANGE_RETURN_TIME_BUF_NULL;
    }
    time_range_info.abs_time.flag = TIME_RANGE_ABS_TIME;
    
    ret = dbtable_hash_insert(time_range_info_name, &time_range_info);

    if(ret == 0)
    {
        return MATCH_RETURN_CODE_SUCCESS;
    }
    else if(-1 == ret)
    {
        return TIME_RANGE_RETURN_ITEM_FULL;
    }
    else
    {
        return ret;
    }
    
}
int npd_periodic_time_range_create(char *name, char *start_time, char *end_time, unsigned int flag)
{
    char *buf = NULL;
    unsigned int ret = 0;
    struct tm tm_start;
    struct tm tm_end;
    struct time_range_info_s time_range_info;
    strncpy(time_range_info.name, name, 16);
    ret = dbtable_hash_search(time_range_info_name,&time_range_info,NULL,&time_range_info);
    if(ret == 0)
    {
        if(time_range_info.periodic_time.flag != 0)
        {
            return TIME_RANGE_RETURN_PERIODIC_TIME_HAS_EXISTED;
        }
    }
    else
    {
        time_range_info.acl_bind_count = 0;
        time_range_info.abs_time.flag = 0;
    }
    buf = (char *)strptime(start_time,"%Y/%m/%d %H:%M:%S",&tm_start);
    if(buf != NULL)
    {
        time_range_info.periodic_time.start_time.hh = tm_start.tm_hour;
        time_range_info.periodic_time.start_time.mm = tm_start.tm_min;

    }
    else
    {
        return TIME_RANGE_RETURN_TIME_BUF_NULL;
    }
    buf = (char *)strptime(end_time, "%Y/%m/%d %H:%M:%S", &tm_end);
    if(buf != NULL)
    {
        time_range_info.periodic_time.end_time.hh = tm_end.tm_hour;
        time_range_info.periodic_time.end_time.mm = tm_end.tm_min;

    }
    else
    {
        return TIME_RANGE_RETURN_TIME_BUF_NULL;
    }
    time_range_info.periodic_time.flag = flag ;
    
    ret = dbtable_hash_insert(time_range_info_name, &time_range_info);

    if(ret == 0)
    {
        return MATCH_RETURN_CODE_SUCCESS;        
    }
    else if(-1 == ret)
    {
        return TIME_RANGE_RETURN_ITEM_FULL;
    }
    else
    {
        return ret;
    }
}
DBusMessage * npd_dbus_time_range_info_create(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = -1;
	char*           name = NULL;
    char*           start_time = NULL;
    char*           end_time = NULL;
	unsigned int    flag;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
								DBUS_TYPE_STRING, &name,
								DBUS_TYPE_STRING, &start_time,
								DBUS_TYPE_STRING, &end_time,
								DBUS_TYPE_UINT32, &flag,
					            DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	diffserv_check_golbal_start();


    if(flag == TIME_RANGE_ABS_TIME)
    {
        ret = npd_abs_time_range_create(name, start_time, end_time);
    }
    else
    {
        ret = npd_periodic_time_range_create(name, start_time, end_time, flag);
    }
    
	diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}

int npd_is_acl_associate_time_range_info(char* map_name, char* time_range_name)
{
    unsigned int ret = 0;
    struct class_map_index_s class_index;
    struct policy_map_index_s policy_index;
	struct class_map_rule_s class_rule;

    if(time_range_name == NULL)
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }

    memset(&policy_index, 0, sizeof(struct policy_map_index_s));
    memset(&class_index, 0, sizeof(struct class_map_index_s));
	memset(&class_rule, 0, sizeof(struct class_map_rule_s));

	strncpy(policy_index.map_name, map_name, 32);
	ret = dbtable_hash_search(policy_map_name, &policy_index, NULL, &policy_index);
    if(0 != ret)
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }

    ret = dbtable_array_get(class_map_master, policy_index.class_map_index, &class_index);
    if(0 == ret)
    {
        strncpy(class_rule.map_name, class_index.map_name, 32);
		class_rule.index = class_index.index;
	  	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
        
        while(0 == ret)
        {
            if (0 == strcmp("time-range", class_rule.cmd_name))
            {
                strncpy(time_range_name, class_rule.cmd_arg, 16);
                return TIME_RANGE_RETURN_BIND_WITH_TIME_RANGE;
            }
            ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, NULL);
        }
    }
    else
        return CLASSMAP_RETURN_CODE_NOTEXIST;
    return TIME_RANGE_RETURN_NOT_BIND_WITH_TIME_RANGE;
    
}
DBusMessage * npd_dbus_is_acl_associate_time_range_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = -1;
	char*           map_name = NULL;
    char*           time_range_name = NULL;

    time_range_name = (char *)malloc(sizeof(char)*16);
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
								DBUS_TYPE_STRING, &map_name,
					            DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(time_range_name)
		    free(time_range_name);
		return NULL;
    }

	diffserv_check_golbal_start();

    ret = npd_is_acl_associate_time_range_info(map_name,time_range_name);
    
	diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
    free(time_range_name);
    
	return reply;
}
int npd_is_time_in_time_range(char *map_name, unsigned int *op_ret)
{
    unsigned int ret = 0;
    struct time_range_info_s time_range_info;
    time_t now;
    struct tm *timenow = NULL;
    char* time_range_name = NULL;
    int standard = 0;  
    int start = 0;     
    int end = 0;      
    int standard_day = 0;
    int start_day = 0;
    int end_day = 0;

    memset(&time_range_info, 0, sizeof(struct time_range_info_s));
    time_range_name = (char *)malloc(sizeof(char)*16);
    memset(time_range_name,0,16);

    ret = npd_is_acl_associate_time_range_info(map_name, time_range_name);
    if(ret != TIME_RANGE_RETURN_BIND_WITH_TIME_RANGE)
    {
        free(time_range_name);
        return ret;
    }

    strncpy(time_range_info.name, time_range_name, 16);
    ret = dbtable_hash_search(time_range_info_name,&time_range_info,NULL,&time_range_info);
    if(0 != ret)
    {
        free(time_range_name);
        return TIME_RANGE_RETURN_NAME_NOEXIST;
    }

    time(&now);
    timenow = localtime(&now);

    if(TIME_RANGE_ABS_TIME != time_range_info.abs_time.flag )
    {
        standard = (timenow->tm_hour)*60 + timenow->tm_min;
        start = (time_range_info.periodic_time.start_time.hh)*60  
                + time_range_info.periodic_time.start_time.mm;
        end = (time_range_info.periodic_time.end_time.hh)*60
              + time_range_info.periodic_time.end_time.mm;
        if(TIME_RANGE_EVERYDAY == time_range_info.periodic_time.flag)
        {
            if(standard>=start && standard<=end)
                *op_ret = IN_TIME_RANGE;
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(TIME_RANGE_WORKDAY == time_range_info.periodic_time.flag)
        {
            if(timenow->tm_wday <= 5 && timenow->tm_wday >= 1)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(TIME_RANGE_WEEKEND == time_range_info.periodic_time.flag) 
        {
            if(timenow->tm_wday == 6 || timenow->tm_wday == 0)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else
        {
            syslog_ax_acl_err("Unknow Error!\n");
        }
    }
    else 
    {
        standard_day = (timenow->tm_year+1900)*372 + (timenow->tm_mon)*31 + timenow->tm_mday;
        start_day = (time_range_info.abs_time.start_time.year)*372
                    + (time_range_info.abs_time.start_time.month-1)*31
                    + time_range_info.abs_time.start_time.day;
        end_day = (time_range_info.abs_time.end_time.year)*372
                    + (time_range_info.abs_time.end_time.month-1)*31
                    + time_range_info.abs_time.end_time.day;

        if(standard_day < start_day || standard_day > end_day)
        {
            *op_ret = OUT_TIME_RANGE;
            free(time_range_name);
            return MATCH_RETURN_CODE_SUCCESS;
        }

        {
            int abs_time_start = 0;
            int abs_time_end = 0;
            int current_time = 0;

            current_time = (timenow->tm_hour)*60 + timenow->tm_min;
            abs_time_start = (time_range_info.abs_time.start_time.hh)*60  
                    + time_range_info.abs_time.start_time.mm;
            abs_time_end = (time_range_info.abs_time.end_time.hh)*60
                  + time_range_info.abs_time.end_time.mm;
            
            if(standard_day == end_day)
            {
                if(current_time > abs_time_end)
                {   
                    *op_ret = OUT_TIME_RANGE;
                    free(time_range_name);
                    return MATCH_RETURN_CODE_SUCCESS;
                }
            }
            if(standard_day == start_day)
            {
                if(current_time < abs_time_start)
                {   
                    *op_ret = OUT_TIME_RANGE;
                    free(time_range_name);
                    return MATCH_RETURN_CODE_SUCCESS;
                }
            }
        }
        
        standard = (timenow->tm_hour)*60 + timenow->tm_min;
        start = (time_range_info.periodic_time.start_time.hh)*60  
                + time_range_info.periodic_time.start_time.mm;
        end = (time_range_info.periodic_time.end_time.hh)*60
              + time_range_info.periodic_time.end_time.mm;

        

        if(TIME_RANGE_EVERYDAY == time_range_info.periodic_time.flag)
        {
            if(standard>=start && standard<=end)
                *op_ret = IN_TIME_RANGE;
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(TIME_RANGE_WORKDAY == time_range_info.periodic_time.flag)
        {
            if(timenow->tm_wday <= 5 && timenow->tm_wday >= 1)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(TIME_RANGE_WEEKEND == time_range_info.periodic_time.flag) 
        {
            if(timenow->tm_wday == 6 || timenow->tm_wday == 0)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else
        {
            *op_ret = IN_TIME_RANGE;
        }
    }
    free(time_range_name);
    return MATCH_RETURN_CODE_SUCCESS;
}

int npd_no_time_range_name(char *time_range_name)
{
    unsigned int ret = 0;
    struct time_range_info_s time_range_info;
    memset(&time_range_info, 0, sizeof(struct time_range_info_s));
    strncpy(time_range_info.name, time_range_name, 16);
    ret = dbtable_hash_search(time_range_info_name,&time_range_info,NULL,&time_range_info);
    if(0 != ret)
    {        
        syslog_ax_acl_err("can't find the time-range %s \n",time_range_name);
        return TIME_RANGE_RETURN_NAME_NOEXIST;
    }
    if(time_range_info.acl_bind_count == 0)
    {
        ret = dbtable_hash_delete(time_range_info_name,&time_range_info,&time_range_info);
        if(ret == 0)
        {
            syslog_ax_acl_err("delete time-range %s success!\n",time_range_name);
            return MATCH_RETURN_CODE_SUCCESS;
        }
        else
        {
            syslog_ax_acl_err("delete time-range %s FAILED!!!\n",time_range_name);
            return ret;
        }
    }
    else
    {
        syslog_ax_acl_err("the time-range bind-with acl!");
        return TIME_RANGE_RETURN_BIND_WITH_ACL;
    }
    
}

DBusMessage * npd_dbus_no_time_range_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	time_range_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
					   DBUS_TYPE_STRING, &time_range_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	diffserv_check_golbal_start();

    ret = npd_no_time_range_name(time_range_name);
    
	diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}

int npd_acl_time_range_deploy(char *policy_name, int dir, unsigned int operation)
{
    int ret = 0;

    struct service_policy_s		service;
    struct policy_map_index_s	policy;
    struct class_map_index_s   class;

    memset(&service, 0, sizeof(struct service_policy_s));
	memset(&policy, 0, sizeof(struct policy_map_index_s));
    memset(&class, 0, sizeof(struct class_map_index_s));

    strncpy(policy.map_name, policy_name, 32);
    ret = dbtable_hash_search(policy_map_name, &policy, NULL, &policy);
	if(0 != ret)
	{
	    return POLICYMAP_RETURN_CODE_NOTEXIST;
	}

    if(policy.class_map_index != -1)
        ret = dbtable_array_get(class_map_master, policy.class_map_index, &class);
    else
        return POLICYMAP_RETURN_CODE_CLASSMAP_NOTEXIST;

    strncpy(service.policy_map_name, policy_name, 32);
    service.dir_type = dir;
    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
    if(0 != ret)
    {
        return DIFFSERV_RETURN_CODE_SERVICE_POLICY_NOT_EXIST;
    }
    if((OUT_TIME_RANGE == service.time_range_set && IN_TIME_RANGE == operation)
        || (IN_TIME_RANGE == service.time_range_set && OUT_TIME_RANGE == operation))
    {
        service.time_range_set = operation;
        ret = dbtable_array_update(service_policy_index, service.service_policy_index, NULL, &service);
        if(0 == ret)
        {
            return MATCH_RETURN_CODE_SUCCESS;
        }
        else
        {
            return TIME_RANGE_RETURN_DB_OPREATE_ERROR;
        }
    }
    return TIME_RANGE_RETURN_NEED_NOT_DEPLOY;   
}
int npd_no_acl_time_range_associate(char* map_name,char* time_range_name)
{
    struct service_policy_s		service;
    struct class_map_rule_s     rule;
	struct class_map_rule_s     del_rule;
    struct class_map_index_s    class_map;	
    unsigned int ret;

    memset(&service, 0, sizeof(struct service_policy_s));
    memset(&rule, 0, sizeof(struct class_map_rule_s));
    memset(&del_rule, 0, sizeof(struct class_map_rule_s));
    memset(&class_map, 0, sizeof(struct class_map_index_s));
 

    strncpy(service.policy_map_name, map_name, 32);


    if(0 == dbtable_hash_search(service_policy_name, &service, NULL, &service))
    {
        return DIFFSERV_RETURN_CODE_SERVICE_POLICY_EXIST;
    }

    if(TIME_RANGE_RETURN_BIND_WITH_TIME_RANGE != npd_is_acl_associate_time_range_info(map_name,rule.cmd_arg))
    {
        return TIME_RANGE_RETURN_NOT_BIND_WITH_TIME_RANGE;
    }

    if( 0 != strcmp(time_range_name,rule.cmd_arg) )
    {
        return TIME_RANGE_RETURN_ACL_BIND_OTHER_TIME_RANGE;
    }

    if(CLASSMAP_RETURN_CODE_EXIST != class_map_find_by_name(map_name, &class_map))
    {
        return CLASSMAP_RETURN_CODE_NOTEXIST;
    }

    rule.index = class_map.index;
    strncpy(rule.map_name, map_name, 32);
    strncpy(rule.cmd_name, "time-range", 32);

    ret = dbtable_hash_search(class_map_match_rule, &rule, (unsigned int(*)(void*, void*))class_map_rule_simple_cmp, &rule);    
    if(0 != ret)
	{
		return TIME_RANGE_RETURN_DB_OPREATE_ERROR;
	}
	ret = dbtable_hash_delete(class_map_match_rule, &rule, &del_rule);
	if(0 != ret)
	{
		return TIME_RANGE_RETURN_DB_OPREATE_ERROR;
	}

    {
        struct time_range_info_s time_range_info;
        strncpy(time_range_info.name, rule.cmd_arg, 16);
        dbtable_hash_search(time_range_info_name, &time_range_info, NULL, &time_range_info);
        time_range_info.acl_bind_count--;
        ret = dbtable_hash_insert(time_range_info_name,&time_range_info);
        if(0 != ret)
        {
            return TIME_RANGE_RETURN_DB_OPREATE_ERROR;
        }
    }
    
	return MATCH_RETURN_CODE_SUCCESS;
}

DBusMessage * npd_dbus_no_acl_time_range_associate(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	time_range_name = NULL;
    char*   map_name = NULL;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,
                       DBUS_TYPE_STRING, &map_name,
					   DBUS_TYPE_STRING, &time_range_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	diffserv_check_golbal_start();

    ret = npd_no_acl_time_range_associate(map_name,time_range_name);

    diffserv_check_golbal_end();
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;
}
DBusMessage * npd_dbus_acl_show_time_range_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	unsigned int	uni = 0;
	unsigned int 	ret = 0;
	const char*		time_range_name = NULL;
	char*			time_range_info_element[8];
    char            temp[33] ;

	struct time_range_info_s time_range_info;

	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
	
	memset(&time_range_info, 0, sizeof(struct time_range_info_s));
    memset(temp, 0, 33);

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,
					   DBUS_TYPE_STRING,&time_range_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	strncpy(time_range_info.name, time_range_name, 16);
	ret = dbtable_hash_search(time_range_info_name, &time_range_info, NULL, &time_range_info);

    if(0 == ret)
    {
        for(uni=0; uni < 8; uni++)
        {
            time_range_info_element[uni] = (char* )malloc(33);
            time_range_info_element[uni][32]=0;
        }
        strncpy(time_range_info_element[0], time_range_info.name, 16);

        sprintf(temp, "%d", time_range_info.acl_bind_count);
        strncpy(time_range_info_element[1], temp, 33);
        
        if(TIME_RANGE_ABS_TIME == time_range_info.abs_time.flag)
        {
            int mon1, mon2, day1, day2, hh1, hh2, mm1, mm2;
            mon1 = time_range_info.abs_time.start_time.month/10;
            mon2 = time_range_info.abs_time.start_time.month%10;
            day1 = time_range_info.abs_time.start_time.day/10;
            day2 = time_range_info.abs_time.start_time.day%10;
            hh1 = time_range_info.abs_time.start_time.hh/10;
            hh2 = time_range_info.abs_time.start_time.hh%10;
            mm1 = time_range_info.abs_time.start_time.mm/10;
            mm2 = time_range_info.abs_time.start_time.mm%10;
            strcpy(time_range_info_element[2], "YES");
            sprintf(temp, "%d-%d%d-%d%d %d%d:%d%d", time_range_info.abs_time.start_time.year,
                                                mon1, mon2,
                                                day1, day2,
                                                hh1, hh2,
                                                mm1, mm2);
            strncpy(time_range_info_element[4], temp, 33);

            mon1 = time_range_info.abs_time.end_time.month/10;
            mon2 = time_range_info.abs_time.end_time.month%10;
            day1 = time_range_info.abs_time.end_time.day/10;
            day2 = time_range_info.abs_time.end_time.day%10;
            hh1 = time_range_info.abs_time.end_time.hh/10;
            hh2 = time_range_info.abs_time.end_time.hh%10;
            mm1 = time_range_info.abs_time.end_time.mm/10;
            mm2 = time_range_info.abs_time.end_time.mm%10;
            sprintf(temp, "%d-%d%d-%d%d %d%d:%d%d", time_range_info.abs_time.end_time.year,
                                                mon1, mon2,
                                                day1, day2,
                                                hh1, hh2,
                                                mm1, mm2);
            strncpy(time_range_info_element[5], temp, 33);  
        }
        else
        {
            strcpy(time_range_info_element[2], "NO");
            sprintf(temp, "NOT SET");
            strncpy(time_range_info_element[4], temp, 33);
            strncpy(time_range_info_element[5], temp, 33);
        }

        switch(time_range_info.periodic_time.flag)
        {
            case TIME_RANGE_EVERYDAY:
                strcpy(time_range_info_element[3], "everyday");break;
            case TIME_RANGE_WORKDAY:
                strcpy(time_range_info_element[3], "workday");break;
            case TIME_RANGE_WEEKEND:
                strcpy(time_range_info_element[3], "weekend");break;
            default:
                strcpy(time_range_info_element[3], "NOT SET");
                    break;
        }
        {
            int hh1, hh2, mm1, mm2;
            hh1 = time_range_info.periodic_time.start_time.hh/10;
            hh2 = time_range_info.periodic_time.start_time.hh%10;
            mm1 = time_range_info.periodic_time.start_time.mm/10;
            mm2 = time_range_info.periodic_time.start_time.mm%10;
            sprintf(temp, "           %d%d:%d%d", hh1, hh2, mm1, mm2);
            strncpy(time_range_info_element[6], temp, 33);

            hh1 = time_range_info.periodic_time.end_time.hh/10;
            hh2 = time_range_info.periodic_time.end_time.hh%10;
            mm1 = time_range_info.periodic_time.end_time.mm/10;
            mm2 = time_range_info.periodic_time.end_time.mm%10;
            sprintf(temp, "           %d%d:%d%d", hh1, hh2, mm1, mm2); 
            strncpy(time_range_info_element[7], temp, 33);

        }
        if(0 == time_range_info.periodic_time.flag)
        {
            strcpy(time_range_info_element[6], "NOT SET");
            strcpy(time_range_info_element[7], "NOT SET");
        }
    }
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
    if(ret == 0)
    {
        for (uni = 0; uni < 8; uni++)
        {  
        	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &time_range_info_element[uni]);
        }

        for (uni = 0; uni < 8; uni++)
        {
        	free(time_range_info_element[uni]);
        }
    }

	return reply;
}
DBusMessage * npd_dbus_acl_show_time_range_bind(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;

    const char*		time_range_name = NULL;
    struct class_map_index_s class_index;
    struct class_map_rule_s class_rule;
    char* show_str = NULL;
    char tmp[32];
    int totalcount = 0;
    int uni = 0;
    int count = 0;
    int op_ret = 0;

    memset(&class_index, 0, sizeof(struct class_map_index_s));
    memset(&class_rule, 0, sizeof(struct class_map_rule_s));
    
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args (msg, &err,
					   DBUS_TYPE_STRING, &time_range_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    
    show_str = (char *)malloc(SHOW_SERVICE_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, SHOW_SERVICE_SIZE);
        totalcount = dbtable_array_totalcount(class_map_master);
        for(uni = 0; uni < totalcount; uni++)
        {
            op_ret = dbtable_array_get(class_map_master, uni, &class_index);        
            if(0 != op_ret)
                continue;
    	    if(!(strcmp(class_index.map_name, "VOICE VLAN"))
                || !(strncmp(class_index.map_name, "SERV_TC", strlen("SERV_TC")))
                || !(strncmp(class_index.map_name, "IPV6_", 5))			
                || !(strncmp(class_index.map_name, "SG_", strlen("SG_")))
#ifdef HAVE_PORTAL                 
                || !(strncmp(class_index.map_name, "PORTAL_", strlen("PORTAL_")))
#endif
              )
    	    {
                continue;
    	    }                       
            strcpy(class_rule.map_name, class_index.map_name);
            class_rule.index = class_index.index;
            op_ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
            while(op_ret == 0)
            {
                memset(tmp, 0, sizeof(tmp));
                if(0 == strcmp(time_range_name, class_rule.cmd_arg))
                {
                    sprintf(tmp, "%-12s", class_rule.map_name);
                    strcat(show_str, tmp);
                    count++;
                    if(0 == count % 5)
                    {
                        sprintf(tmp, "\n");
                        strcat(show_str, tmp);
                    }
                }
                op_ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
            }
        }
        if(0 != count % 5)
        {
            sprintf(tmp, "\n");
            strcat(show_str, tmp);  
        }
    }

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);

    if(show_str)
    {
        free(show_str);
        show_str = NULL;
    }
    return reply;
}
DBusMessage * npd_dbus_acl_show_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
 	unsigned int	name_count = 0;
	unsigned int	ret = MATCH_RETURN_CODE_SUCCESS;
 	char			tmp[256];
    char*           show_str = NULL;
    struct time_range_info_s time_range_info;
	DBusMessage*	reply = NULL;    
	DBusMessageIter	iter;
    DBusError		err;

    dbus_error_init(&err);

	memset(&time_range_info, 0, sizeof(struct time_range_info_s));

    show_str = (char *)malloc(SHOW_SERVICE_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, SHOW_SERVICE_SIZE);
    	ret = dbtable_hash_next(time_range_info_name, NULL, &time_range_info, NULL);
    	while (0 == ret)
    	{
    		memset(tmp, 0, sizeof(tmp));
            name_count++;
            sprintf(tmp, "%-15d%-40s\n", name_count, time_range_info.name);
    		strcat(show_str, tmp);
    		ret = dbtable_hash_next(time_range_info_name, &time_range_info, &time_range_info, NULL);
    	}
    }

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);     
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);
    if(show_str != NULL)
    {
        free(show_str);
        show_str = NULL;
    }
	return reply;
}

char* abs_time_range_info_show_running(char* showStr, int* safe_len)
{
    char*   cursor = NULL;
    int     totalLen = 0;
    int     ret = 0;
    char   namestr[32];
    char   typestr[32];
    char   start_time_str[32];
    char   end_time_str[32];

    int mon1 = 0, mon2 = 0;
    int day1 = 0, day2 = 0;
    int hh1 = 0, hh2 = 0;
    int mm1 = 0, mm2 = 0;
	int have_conf = 0;
    struct time_range_info_s time_range_info;

    cursor = showStr;

	
    memset(&time_range_info, 0, sizeof(struct time_range_info_s));
	
    ret = dbtable_hash_next(time_range_info_name, NULL, &time_range_info, NULL);

    while(0 == ret)
    {
        if(TIME_RANGE_ABS_TIME == time_range_info.abs_time.flag)
        {
            memset(namestr, 0, 32);
            memset(typestr, 0, 32);
            memset(start_time_str, 0, 32);
            memset(end_time_str, 0, 32);
			
            sprintf(namestr, "time-range %s ", time_range_info.name);
            sprintf(typestr, "absolute ");

            mon1 = time_range_info.abs_time.start_time.month/10;
            mon2 = time_range_info.abs_time.start_time.month%10;
            day1 = time_range_info.abs_time.start_time.day/10;
            day2 = time_range_info.abs_time.start_time.day%10;
            hh1 = time_range_info.abs_time.start_time.hh/10;
            hh2 = time_range_info.abs_time.start_time.hh%10;
            mm1 = time_range_info.abs_time.start_time.mm/10;
            mm2 = time_range_info.abs_time.start_time.mm%10;
            sprintf(start_time_str,"start %d/%d%d/%d%d %d%d:%d%d ", time_range_info.abs_time.start_time.year,
                                                                    mon1, mon2,
                                                                    day1, day2,
                                                                    hh1, hh2,
                                                                    mm1, mm2);
            mon1 = time_range_info.abs_time.end_time.month/10;
            mon2 = time_range_info.abs_time.end_time.month%10;
            day1 = time_range_info.abs_time.end_time.day/10;
            day2 = time_range_info.abs_time.end_time.day%10;
            hh1 = time_range_info.abs_time.end_time.hh/10;
            hh2 = time_range_info.abs_time.end_time.hh%10;
            mm1 = time_range_info.abs_time.end_time.mm/10;
            mm2 = time_range_info.abs_time.end_time.mm%10;
            sprintf(end_time_str,"end %d/%d%d/%d%d %d%d:%d%d ", time_range_info.abs_time.end_time.year,
                                                                    mon1, mon2,
                                                                    day1, day2,
                                                                    hh1, hh2,
                                                                    mm1, mm2);
            totalLen += sprintf(cursor, "%s%s%s%s\n", namestr, typestr, start_time_str, end_time_str);
    		cursor = showStr + totalLen;
            have_conf = 1;
        }
        
        ret = dbtable_hash_next(time_range_info_name, &time_range_info, &time_range_info, NULL);
    }
    if(have_conf)
    {
        totalLen += sprintf(cursor, "\n");
    }
	*safe_len = totalLen;
    return showStr;
}
char* periodic_time_range_info_show_running(char* showStr, int* safe_len)
{
    char*   cursor = NULL;
    int     totalLen = 0;
    int     ret = 0;
    char   namestr[32];
    char   typestr[32];
    char   start_time_str[32];
    char   end_time_str[32];

    int hh1 = 0, hh2 = 0;
    int mm1 = 0, mm2 = 0;
    int have_conf = 0;
    struct time_range_info_s time_range_info;
	
    cursor = showStr;

	memset(&time_range_info, 0, sizeof(struct time_range_info_s));
	
    ret = dbtable_hash_next(time_range_info_name, NULL, &time_range_info, NULL); 

    while(0 == ret)
    {
        if(0 != time_range_info.periodic_time.flag)
        {
            memset(namestr, 0, 32);
            memset(typestr, 0, 32);
            memset(start_time_str, 0, 32);
            memset(end_time_str, 0, 32);
			
            sprintf(namestr, "time-range %s ", time_range_info.name);

            if(TIME_RANGE_EVERYDAY == time_range_info.periodic_time.flag)
                sprintf(typestr, "periodic everyday ");
            else if(TIME_RANGE_WORKDAY == time_range_info.periodic_time.flag)
                sprintf(typestr, "periodic workday ");
            else if(TIME_RANGE_WEEKEND == time_range_info.periodic_time.flag)
                sprintf(typestr, "periodic weekend ");

            hh1 = time_range_info.periodic_time.start_time.hh/10;
            hh2 = time_range_info.periodic_time.start_time.hh%10;
            mm1 = time_range_info.periodic_time.start_time.mm/10;
            mm2 = time_range_info.periodic_time.start_time.mm%10;        
            sprintf(start_time_str, "start %d%d:%d%d ", hh1, hh2, mm1, mm2);

            hh1 = time_range_info.periodic_time.end_time.hh/10;
            hh2 = time_range_info.periodic_time.end_time.hh%10;
            mm1 = time_range_info.periodic_time.end_time.mm/10;
            mm2 = time_range_info.periodic_time.end_time.mm%10;
            sprintf(end_time_str, "end %d%d:%d%d", hh1, hh2, mm1, mm2); 

            totalLen += sprintf(cursor, "%s%s%s%s\n", namestr, typestr, start_time_str, end_time_str);
    		cursor = showStr + totalLen;
			have_conf = 1;
        }        
        ret = dbtable_hash_next(time_range_info_name, &time_range_info, &time_range_info, NULL);        
    }
    if(have_conf)
    {
        totalLen += sprintf(cursor, "\n");
    }
	*safe_len = totalLen;
    return showStr; 
}
char* time_range_info_associate_show_running(char* showStr, int* safe_len)
{
    char*   cursor = NULL;
    int     totalLen = 0;
    int     uni = 0;
    int     ret = 0;
    char   idstr[32];
    char   namestr[32];
    int     totalcount = 0;
	int have_conf = 0;
    struct class_map_index_s class_index;
	struct class_map_rule_s class_rule;

    cursor = showStr;

	memset(&class_index, 0, sizeof(struct class_map_index_s));
	memset(&class_rule, 0, sizeof(struct class_map_rule_s));

    totalcount = dbtable_array_totalcount(class_map_master);

    for (uni = 0; uni < totalcount; uni++)
	{
		if (*safe_len < (totalLen + 1000))
		{
			break;
		}
        ret = dbtable_array_get(class_map_master, uni, &class_index);        
        if(0 != ret)
            continue;
        if(0 != strncmp(class_index.map_name, "ACL_", 4))
        {
            continue;
        }
                
        strcpy(class_rule.map_name, class_index.map_name);
        class_rule.index = class_index.index;
        ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
        
        while (0 == ret)
		{
            if (0 == strcmp("time-range", class_rule.cmd_name))
            {
        		memset(idstr, 0, 32);
        		memset(namestr, 0, 32);
                sprintf(idstr, "acl %s ", &class_rule.map_name[4]);
                sprintf(namestr, "time-range %s", class_rule.cmd_arg);
                totalLen += sprintf(cursor, "%s%s\n", idstr, namestr);
        		cursor = showStr + totalLen;
				have_conf = 1;
				break;
            }
		  	ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
	    }
    }
    if(have_conf)
    {
        totalLen += sprintf(cursor, "\n");
    }
	*safe_len = totalLen;
    return showStr;
}
char *npd_acl_group_show_running_config(char *showStr, int *safe_len)
{
    char*   cursor = NULL;
    int     totalLen = 0;

    int     totalcount = 0;
    int     ret = 0;
    unsigned int uni = 0;
    unsigned int icount = 0;
    unsigned int id = 0;
    struct acl_group_stc acl_group;
    struct policy_map_index_s policy_map;

    cursor = showStr;
    totalcount = dbtable_array_totalcount(acl_group_index);
    for(uni = 0; uni < totalcount; uni++)
    {
  		if (*safe_len < (totalLen + 1000))
		{
			break;
		}
        
        memset(&acl_group, 0, sizeof(struct acl_group_stc));
        ret = dbtable_array_get(acl_group_index, uni, &acl_group);
        if(0 != ret)
        {
            continue;
        }
        
        totalLen += sprintf(cursor, "acl-group %s\n", acl_group.name);
        cursor = showStr + totalLen;
        for(icount = 0; icount < ACL_GROUP_MAX_RULES; icount++)
        {
            if(0 == acl_group.acl_index[icount])
            {
                continue;
            }
            memset(&policy_map, 0, sizeof(struct policy_map_index_s));
            ret = dbtable_array_get(policy_map_index, acl_group.acl_index[icount], &policy_map);
            if(0 != ret)
            {
                continue;
            }
            if(strncmp("ACL_", policy_map.map_name, 4) == 0)
            {
                id = strtoul(&policy_map.map_name[4], NULL, 0);
            }
            totalLen += sprintf(cursor, " add acl %d\n", id);
            cursor = showStr + totalLen;
        }
        if(strlen(acl_group.desp) > 0)
        {
            totalLen += sprintf(cursor, " description %s\n", acl_group.desp);
            cursor = showStr + totalLen;
        }
        totalLen += sprintf(cursor, " exit\n");
        cursor = showStr + totalLen;
    }
	*safe_len = totalLen;
    return showStr;
}
void npd_acl_rule_based_tm_timer(void)
{
    unsigned int ret = 0;
    unsigned int op_ret = 0;
    int i = 0, timer_counter = 0;
    int totalcount = 0;
    struct service_policy_s service_index;

    if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
        return ;
    npd_init_tell_whoami("SysAclBasedTime",0);
    
    while(1)
    {
        if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
        {
            totalcount = dbtable_array_totalcount(service_policy_index);
            for(i = 0; i < totalcount; i++)
            {
                ret = dbtable_array_get(service_policy_index, i, &service_index);
                if(0 != ret)
                    continue;
                ret = npd_is_time_in_time_range(service_index.policy_map_name,&op_ret);
                if(MATCH_RETURN_CODE_SUCCESS != ret)
                    continue;

                ret = npd_acl_time_range_deploy(service_index.policy_map_name,
                            service_index.dir_type ,op_ret);
                
            }
        }
        sleep(1);
    }
}
int npd_classmap_rule_del_by_vid(unsigned int vid)
{
    int i = 0;
    unsigned int ret = 0;
    unsigned int netif_index = 0;
    struct service_policy_s service_index;
    if(!NPD_ACL_BASED_VLAN_SUPPORT)
        return SERVICE_POLICY_RETURN_ACL_BASED_VLAN_NOT_SUPPORT;
    for(i = 0; i < dbtable_array_totalcount(service_policy_index); i++)
    {        
	    ret = dbtable_array_get(service_policy_index, i, &service_index);
        if(0 != ret)
        {
            continue;
        }
        if(!NPD_VBMP_MEMBER(service_index.vlanbmp, vid))
        {
            continue;
        }

        netif_index = npd_netif_vlan_get_index(vid);
        ret = service_policy_destroy(service_index.policy_map_name,
                               service_index.dir_type,
                               netif_index);  
    }
    return 0;
}

int npd_get_service_phase_by_index(int index, char* phase)
{
    struct service_policy_s     service;
    struct policy_map_index_s	policy_index;
    struct class_map_rule_s		class_rule;
    char	lookupPhase = 0;
    unsigned int ret = 0;
    
    memset(&service, 0, sizeof(struct service_policy_s));
    memset(&policy_index, 0, sizeof(struct policy_map_index_s));
    memset(&class_rule, 0, sizeof(struct class_map_rule_s));
    
    service.service_policy_index = index; 
    ret = dbtable_array_get(service_policy_index, service.service_policy_index, &service);
    if(0 != ret)
        return -1;
    if(1 == service.dir_type)
    {
        *phase = ACL_PHASE3_E;
        return 0;
    }
    strcpy(policy_index.map_name, service.policy_map_name);
	ret = dbtable_hash_search(policy_map_name, &policy_index, NULL, &policy_index);
	if(0 != ret)
		return -1;
    class_rule.index = policy_index.class_map_index;

	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
	while( 0 == ret )
	{
		rule_cmd_t*	cmd = NULL;
		char tempphase; 
		cmd = class_map_lookup_match(class_rule.cmd_name);
		if (NULL != cmd)
		{
            if (NULL != cmd->func_phase)
			{
				tempphase = (*cmd->func_phase)(service.service_policy_index, &class_rule);
			}
			else
				tempphase = class_rule.lk_phase;
		}

        if((0 == lookupPhase) && (tempphase != (ACL_PHASE2_E|ACL_PHASE1_E))
			&& (tempphase != ACL_PHASE12_E))
            lookupPhase = tempphase;
		else if(tempphase == ACL_PHASE12_E)
			lookupPhase = (ACL_PHASE2_E|ACL_PHASE1_E);
        else if(lookupPhase != (lookupPhase & tempphase))
		     lookupPhase |= tempphase;

		ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, class_map_rule_filter);
	}   
    *phase = lookupPhase;
    return 0;
}

int npd_serv_policy_is_vbmp_empty(int index)
{
    struct service_policy_s service;
    unsigned ret = 0;
    memset(&service, 0, sizeof(struct service_policy_s));
    service.service_policy_index = index; 
    ret = dbtable_array_get(service_policy_index, service.service_policy_index, &service);
    if(0 == ret)
    {
        if(NPD_VBMP_NOT_NULL(service.vlanbmp))
            return SERVICE_POLICY_RETURN_VBMP_NOT_NULL;
    }
    return SERVICE_POLICY_RETURN_VBMP_NULL;
}
DBusMessage * npd_dbus_acl_show_vmap_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	int 			ret = MATCH_RETURN_CODE_SUCCESS;
	unsigned short	vid = 0;
	const char*		map_name = NULL;
    char*           vlanStr = NULL;
	struct service_policy_s		service;
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;

    vlanStr = (char *)malloc(SHOW_SERVICE_SIZE);
    if(NULL != vlanStr)
    {
        memset(vlanStr, 0, SHOW_SERVICE_SIZE);
    }
    else
    {
        return NULL;
    }
	memset(&service, 0, sizeof(struct service_policy_s));

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,
					   DBUS_TYPE_STRING, &map_name,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	    if(vlanStr)
	    {
	        free(vlanStr);
	    }
		return NULL;
	}

	strcpy(service.policy_map_name, map_name);
	service.dir_type = 0;
	ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
    if (0 != ret)
    {
        service.dir_type = 1;
	    ret = dbtable_hash_search(service_policy_name, &service, NULL, &service);
    }
	if (0 == ret)
	{
		char	temp[32];
        unsigned int    count = 0;
        if(!NPD_VBMP_IS_NULL(service.vlanbmp))
        {
            memset(temp, 0, 32);
    		if(service.dir_type == ACL_DIRECTION_INGRESS_E)
    		    sprintf(temp, "Direction: Ingress\n");
			if(service.dir_type == ACL_DIRECTION_EGRESS_E)
                sprintf(temp, "Direction: Egress\n");
            strcat(vlanStr, temp);

            memset(temp, 0, 32);
            sprintf(temp, "Applied in vlans:\n\t\t");
    		strcat(vlanStr, temp);

    		NPD_VBMP_ITER(service.vlanbmp, vid)
    		{
                count++;
                memset(temp, 0, 32);
                sprintf(temp, "vlan %d ", vid);
                strcat(vlanStr, temp);
                if(0 == count % 5)
                {
                    strcat(vlanStr, "\n\t\t");
                }
    		}
        }
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &vlanStr);	

    if(NULL != vlanStr)
    {
        free(vlanStr);
        vlanStr = NULL;
    }
	return reply;
}
int npd_l4port_range_check_protocol_conflict(
    struct class_map_index_s* class_index, 
    char *match_name, 
    unsigned int protocol
    )
{
    struct class_map_rule_s rule;
    int ret = 0;
    int conflict_flag = 0; /*0 means no conflict*/
    char *endptr;
    
    if((NULL == class_index) || (NULL == match_name))
        return DIFFSERV_RETURN_CODE_ERROR;

    memset(&rule, 0, sizeof(struct class_map_rule_s));
    strcpy(rule.map_name, class_index->map_name);
    rule.index = class_index->index;

    if(0 == strcmp(match_name, "l4port_range_src"))
    {
        strcpy(rule.cmd_name, "l4port_range_dst");
        ret = dbtable_hash_search(class_map_match_rule, &rule,
                                  (unsigned int(*)(void*, void*))class_map_rule_simple_cmp,
                                  &rule);
        if(0 == ret)
        {
            if(protocol != strtol(&rule.cmd_arg[PROTOCOL_OFFSET], &endptr, 10))
                conflict_flag = 1;
        }
    }
    else if(0 == strcmp(match_name, "l4port_range_dst"))
    {
        strcpy(rule.cmd_name, "l4port_range_src");
        ret = dbtable_hash_search(class_map_match_rule, &rule,
                                  (unsigned int(*)(void*, void*))class_map_rule_simple_cmp,
                                  &rule);
        if(0 == ret)
        {
            if(protocol != strtol(&rule.cmd_arg[PROTOCOL_OFFSET], &endptr, 10))
                conflict_flag = 1;
        }
    }

    if(conflict_flag)
    {
        if(TCP_PROTOCOL == protocol)
            return DIFFSERV_RETURN_CODE_L4_PORT_UDP_EXIST;
        if(UDP_PROTOCOL == protocol)
            return DIFFSERV_RETURN_CODE_L4_PORT_TCP_EXIST;
    }

    return MATCH_RETURN_CODE_SUCCESS;
}

int npd_l4port_range_del_by_cmd_arg(const char *cmd_arg)
{
    int             port_range_index[L4_PORT_RANGE_OP_TOTOAL];
    char             index_string[L4_PORT_RANGE_OP_TOTOAL][4];
    struct l4port_range_s port_range[L4_PORT_RANGE_OP_TOTOAL];
    int uni = 0;
    int ret = 0;
    char *endptr = NULL;

    if(NULL == cmd_arg)
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }
    memset(port_range_index, -1, sizeof(port_range_index));
    memset(index_string, -1, sizeof(index_string));
    memset(port_range, 0, sizeof(port_range));

    for(uni = 0; uni < L4_PORT_RANGE_OP_TOTOAL; uni++)
    {
        memcpy(index_string[uni], &cmd_arg[uni * 4], 4);
        port_range_index[uni] = strtol(index_string[uni], &endptr, 10);
        if(-1 != port_range_index[uni])
        {
            ret = dbtable_array_get(l4port_range_master, port_range_index[uni], &port_range[uni]);
			if(0 != ret)
				continue;
            port_range[uni].bind_count--;
            if(0 == port_range[uni].bind_count)
            {
                ret = dbtable_array_delete(l4port_range_master, port_range_index[uni], &port_range[uni]);
            }
            else
            {
                ret = dbtable_array_update(l4port_range_master, port_range_index[uni], NULL, &port_range[uni]);
            }
        }
    }

    return MATCH_RETURN_CODE_SUCCESS;
}
int npd_match_l4port_range(
     struct class_map_index_s* class_index, 
     char *match_name, 
     unsigned int operation, 
     unsigned int protocol,
     unsigned int port0,
     unsigned int port1
     )
{
    struct l4port_range_s ps1;
    struct l4port_range_s ps2;
    struct class_map_rule_s rule;
    struct class_map_rule_s del_rule;
    struct class_map_index_s* db_index = (struct class_map_index_s*)class_index;
    unsigned int tmp_op = 0;
    unsigned int ret = 0;
    unsigned int id = 0;
    int totalcount = 0;
    unsigned int uni = 0;
    int flag = 0;
    int flag_tcp = -1;
    int flag_udp = -1;
    char temp_index[4] = {0};
    rule_cmd_t* cmd = NULL;

    memset(&ps1, 0, sizeof(struct l4port_range_s));
    memset(&ps2, 0, sizeof(struct l4port_range_s));
    memset(&rule, 0, sizeof(struct class_map_rule_s));
    memset(&del_rule, 0, sizeof(struct class_map_rule_s));

	cmd = class_map_lookup_match(match_name);
	if(NULL == cmd)
	{
		return CLASSMAP_RETURN_CODE_NOTFIND;
	}

    del_rule.index = db_index->index;
    strcpy(del_rule.map_name, db_index->map_name);
    strcpy(del_rule.cmd_name, match_name);
    ret = dbtable_hash_search(class_map_match_rule, &del_rule, (unsigned int(*)(void*, void*))class_map_rule_simple_cmp, &del_rule);    
    if(0 == ret)
    {
        if(0 == strcmp(match_name, "l4port_range_dst"))
        {
            return DIFFSERV_RETURN_CODE_L4PORT_DST_EXIST;
        }
        else if(0 == strcmp(match_name, "l4port_range_src"))
        {
            return DIFFSERV_RETURN_CODE_L4PORT_SRC_EXIST;
        }
    }

    ret = npd_l4port_range_check_protocol_conflict(class_index, match_name, protocol);
    if(0 != ret)
        return ret;
    
    /*
        * initialize rule.cmd_arg with -1,-1,-1, -1.
        * rule.cmd_arg[OPERATOR_GT_OFFSET] stored the index of the structure 
        * which presents greater than a port number.
        * rule.cmd_arg[OPERATOR_LT_OFFSET] stored the index of the structure 
        * which presents little than a port number.
        * rule.cmd_arg[OPERATOR_NEQ_OFFSET] stored the index of the structure 
        * which presents not equal with a port number.
        * rule.cmd_arg[PROTOCOL_OFFSET] presents the ip protocol,ether TCP_PROTOCOL or UDP_PROTOCOL.
        * writen by haost@autelan.com 2013/01/08
        */
    sprintf(temp_index, "-1");
    for(uni = 0; uni < 4; uni ++)
        memcpy(&rule.cmd_arg[uni * 4], temp_index, 4);
    
    if(operation == PORT_RANGE)
    {
        tmp_op = GREAT_THAN;
    }
    else
    {
        tmp_op = operation;
    }

    ps1.l4port = port0;
    ps1.operation = tmp_op;
    ps1.protocol = protocol;

    ret = dbtable_hash_search(l4port_range_name, &ps1, NULL, &ps1);
    if(0 == ret)
    {
        ps1.bind_count++;
        ret = dbtable_hash_update(l4port_range_name, NULL, &ps1);
    }
    else
    {
        if(TCP_PROTOCOL == protocol)
        {
            if(npd_tcp_cmp_used >= TCP_CMP_MAX)
            {
                flag = 0;
                flag_tcp = TCP_CMP_EXHAUST;
            }
            else
            {
                flag = 1;
            }
        }
        else
        {
            if(npd_udp_cmp_used >= TCP_CMP_MAX)
            {
                flag = 0;
                flag_udp = UDP_CMP_EXHAUST;
            }
            else 
            {
                flag = 1;
            }
        }
        if(1 == flag)
        {
            totalcount = dbtable_array_totalcount(l4port_range_master);
            for(uni = 0; uni < totalcount; uni++)
            {
                struct l4port_range_s tmp_port_range;
                ret = dbtable_array_get(l4port_range_master, uni, &tmp_port_range);
                if(0 != ret)
                    break;
            }
            ps1.bind_count = 1;
            ps1.index = (int)uni;
            ret = dbtable_array_insert_byid(l4port_range_master, uni, &ps1);
        }
        else if(TCP_CMP_EXHAUST == flag_tcp)
        {
            return CLASSMAP_RETURN_CODE_NOT_ENOUGH_TCP_CMP;
        }
        else if(UDP_CMP_EXHAUST == flag_udp)
        {
            return CLASSMAP_RETURN_CODE_NOT_ENOUGH_UDP_CMP;
        }
        else
        {
            return DIFFSERV_RETURN_CODE_ERROR;
        }
    }
    
    sprintf(temp_index, "%d", ps1.index);
    switch(tmp_op)
    {
        case GREAT_THAN:
            memcpy(&rule.cmd_arg[OPERATOR_GT_OFFSET], temp_index, 4);
            break;
        case LITTLE_THAN:
            memcpy(&rule.cmd_arg[OPERATOR_LT_OFFSET], temp_index, 4);
            break;
        case NOT_EQUAL:
            memcpy(&rule.cmd_arg[OPERATOR_NEQ_OFFSET], temp_index, 4);
            break;
        default:
            break;
    }
    if(PORT_RANGE == operation)
    {
        if(TCP_PROTOCOL == protocol)
        {
            if(npd_tcp_cmp_used >= TCP_CMP_MAX)
            {
                flag = 0;
                flag_tcp = TCP_CMP_EXHAUST;
            }
            else
                flag = 1;
        }
        else
        {
            if(npd_udp_cmp_used >= UDP_CMP_MAX)
            {
                flag = 0;
                flag_udp = UDP_CMP_EXHAUST;
            }
            else 
                flag = 1;
        }
        if(0 == flag)
        {
            id = ps1.index;
            ret = dbtable_array_get(l4port_range_master, id, &ps1);
            ps1.bind_count--;
            if(0 == ps1.bind_count)
            {
                ret = dbtable_array_delete(l4port_range_master, id, &ps1);
            }
            else
                ret = dbtable_array_update(l4port_range_master, id, NULL, &ps1);
            if(UDP_CMP_EXHAUST == flag_udp)
            {
                return CLASSMAP_RETURN_CODE_NOT_ENOUGH_UDP_CMP;
            }
            else if(TCP_CMP_EXHAUST == flag_tcp)
            {
                return CLASSMAP_RETURN_CODE_NOT_ENOUGH_TCP_CMP;
            }
            else
            {
                return DIFFSERV_RETURN_CODE_ERROR;
            }
        }
        
        ps2.l4port = port1;
        ps2.protocol = protocol;
        ps2.operation = LITTLE_THAN;
        ret = dbtable_hash_search(l4port_range_name, &ps2, NULL, &ps2);
        
        if(0 == ret)
        {
            ps2.bind_count++;
            id = ps2.index;
            ret = dbtable_array_update(l4port_range_master, id, NULL, &ps2);
            
        }
        else
        {
            totalcount = dbtable_array_totalcount(l4port_range_master);
            for(uni = 0; uni < totalcount; uni++)
            {
                struct l4port_range_s tmp_port_range;
                ret = dbtable_array_get(l4port_range_master, uni, &tmp_port_range);
                if(0 != ret)
                    break;
            }
            ps2.bind_count = 1;
            ps2.index = (int)uni;
            ret = dbtable_array_insert_byid(l4port_range_master, uni, &ps2);
        }

        sprintf(temp_index, "%d", ps2.index);
        memcpy(&rule.cmd_arg[OPERATOR_LT_OFFSET], temp_index, 4);
    }

	strcpy(rule.map_name, db_index->map_name);
	strcpy(rule.cmd_name, match_name);
	rule.index = db_index->index;
    rule.lk_phase = ACL_PHASE2_E;
	
    sprintf(&rule.cmd_arg[PROTOCOL_OFFSET], "%d", protocol);
    
    ret = dbtable_hash_insert(class_map_match_rule, &rule);

    return ret;
}
DBusMessage * npd_dbus_match_l4port_range(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply = NULL; 
	DBusMessageIter	iter;
	DBusError		err;
	
	unsigned int 	ret = MATCH_RETURN_CODE_SUCCESS;
	char*	map_name = NULL;
	char*	match_name = NULL;
    unsigned int operation;
	unsigned int protocol;
    unsigned int port0;
    unsigned int port1;
	struct class_map_index_s class_map;

	memset(&class_map, 0, sizeof(struct class_map_index_s));
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,  
								DBUS_TYPE_STRING, &map_name,
								DBUS_TYPE_STRING, &match_name,
								DBUS_TYPE_UINT32, &operation,
								DBUS_TYPE_UINT32, &protocol,
								DBUS_TYPE_UINT32, &port0,
								DBUS_TYPE_UINT32, &port1,
					            DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

	if(CLASSMAP_RETURN_CODE_EXIST != class_map_find_by_name(map_name, &class_map)) 
	{
		ret = CLASSMAP_RETURN_CODE_NOTEXIST;
	}
	else if(0 != class_map.is_deployed)
	{
		ret = CLASSMAP_RETURN_CODE_BINDED;
	}
	else
	{
		diffserv_check_golbal_start();
        ret = npd_match_l4port_range(&class_map, match_name, operation, protocol, port0, port1);
		diffserv_check_golbal_end();
	}

	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,&ret);
	return reply;

}

DBusMessage * npd_dbus_check_policy_route_support(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply = NULL; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int    policy_route_support = NPD_ACL_POLICY_ROUTE_SUPPORT;	
    dbus_error_init(&err); 

    reply = dbus_message_new_method_return(msg); 
    
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, &policy_route_support);
	return reply;

}
int npd_vlan_acl_vid_conflict_check(char* policy_name, unsigned int netif_index)
{
    int 			ret = 0;
    unsigned int    vid = 0;
    unsigned int    match_vid = 0;
    struct policy_map_index_s	policy;
    struct class_map_index_s    class_index;
    struct class_map_rule_s     class_rule;

    memset(&policy, 0, sizeof(struct policy_map_index_s));
    memset(&class_index, 0, sizeof(struct class_map_index_s));
    memset(&class_rule, 0, sizeof(struct class_map_rule_s));

    if(NPD_NETIF_VLAN_TYPE != npd_netif_type_get(netif_index))
    {
        return DIFFSERV_RETURN_CODE_ERROR;
    }
    else
    {
        vid = npd_netif_vlan_get_vid(netif_index);
    }
    
	strcpy(policy.map_name, policy_name);
    ret = dbtable_hash_search(policy_map_name, &policy, NULL, &policy);
    if(0 != ret)
    {
        return POLICYMAP_RETURN_CODE_NOTEXIST;
    }

    if(policy.class_map_index != -1)
        ret = dbtable_array_get(class_map_master, policy.class_map_index, &class_index);
    else
        return POLICYMAP_RETURN_CODE_CLASSMAP_NOTEXIST;

    if(0 == ret)
    {
        strncpy(class_rule.map_name, class_index.map_name, 32);
		class_rule.index = class_index.index;
	  	ret = dbtable_hash_head_key(class_map_match_rule, &class_rule, &class_rule, &class_map_rule_filter);
        
        while(0 == ret)
        {
            if (0 == strcmp("outer-vlan", class_rule.cmd_name))
            {
                memcpy(&match_vid, class_rule.cmd_arg, sizeof(unsigned int));
                if((0 != match_vid) && (vid != match_vid))
                    return SERVICE_POLICY_RETURN_ACL_VID_CONFLICT;
            }
            ret = dbtable_hash_next_key(class_map_match_rule, &class_rule, &class_rule, NULL);
        }
    }
    return MATCH_RETURN_CODE_SUCCESS;
}

int npd_class_map_have_dst_src_ip6(int index)
{
    struct class_map_index_s cmIndex;
    struct class_map_rule_s  cmRule;
    int ret = 0;
    int srcip6Flag = 0;
    int dstip6Flag = 0;

    memset(&cmIndex, 0, sizeof(struct class_map_index_s));
    memset(&cmRule, 0, sizeof(struct class_map_rule_s));

    ret = dbtable_array_get(class_map_master, (unsigned int)index, &cmIndex);
    if(0 != ret)
    {
        return FALSE;
    }

    strncpy(cmRule.map_name, cmIndex.map_name, sizeof(cmIndex.map_name));
    cmRule.index = index;
  	ret = dbtable_hash_head_key(class_map_match_rule, &cmRule, &cmRule, &class_map_rule_filter);
    while(0 == ret)
    {
        if(0 == strcmp("srcip6", cmRule.cmd_name))
        {
            srcip6Flag = 1;
        }
        if(0 == strcmp("dstip6", cmRule.cmd_name))
        {
            dstip6Flag = 1;
        }
        ret = dbtable_hash_next_key(class_map_match_rule, &cmRule, &cmRule, NULL);
    }
    return (srcip6Flag & dstip6Flag);    
}
#ifdef __cplusplus
}
#endif
#endif
