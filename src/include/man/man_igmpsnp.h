#ifndef __MAN_IGMPSNP_H__
#define __MAN_IGMPSNP_H__

typedef struct igmp_snp_cfg_info
{
	unsigned int igmp_snoop_enable;
	unsigned int igmpsnp_router_timeout;
	unsigned int igmpsnp_vlan_lifetime;    /*obselate timeout*/
	unsigned int igmpsnp_grp_lifetime;     /*obselate timeout*/
	unsigned int igmpsnp_robust_val;
	unsigned int igmpsnp_query_int;
	unsigned int igmpsnp_resp_int;
	unsigned int igmpsnp_last_member_int;
	unsigned int igmpsnp_last_member_count;
	unsigned int igmpsnp_fastleave_enable;
	unsigned int igmpsnp_port_filter_enable;
	unsigned int igmpsnp_max_join_group;
	unsigned int igmpsnp_querier_source;
}IGMP_SNP_CFG_STC;

typedef struct igmp_snp_port_cfg_info
{
	unsigned int ifindex;
	unsigned int max_join_group;
#ifdef HAVE_MVLAN
	unsigned short mvlanId;
#endif
    int    enable;
}IGMP_SNP_PORT_CFG_STC;

typedef struct igmp_snp_group_info
{
	unsigned char valid;
	unsigned char   state;
	unsigned short	vid;				/*VLAN ID*/
	unsigned short	ver_flag;			/*IGMP version 1-v1, 0-v2*/
	unsigned int mgroup_id;			/*igmp_group_count;vidx*/ //added by wujh
	unsigned int	MC_ipadd;			/*mc group ip*/	
    unsigned int    MC_sadd;            /*mc source ip*/
	unsigned int	report_ipadd;		/*latest report ip*/
	unsigned int	saddr;			    /*last source ip address which send general query packet*/
	npd_pbmp_t  portmbr;
} IGMP_SNP_GRP_STC;

typedef struct igmp_snp_port_info
{	
	unsigned char version;
	unsigned char state;	
	unsigned char type;
	unsigned short vid;
	unsigned int ifindex;			/*index*/
	unsigned int group_addr;
	unsigned int saddr;	
} IGMP_SNP_PORT_STC;

typedef struct igmp_snp_vlan_info
{
	unsigned char state;
	unsigned char enDis;
	unsigned short vid;
#ifdef HAVE_MVLAN
	unsigned char ismvlan;
#endif
	unsigned int saddr;
	npd_pbmp_t member;	
} IGMP_SNP_VLAN_STC;

int dcli_igmp_snp_check_status(unsigned char* stats);

int dcli_enable_disable_igmp_one_vlan
(
	unsigned short vlanId,
	unsigned int enable
);

int dcli_enable_disable_igmp_one_port
(
	unsigned short vlanId,
	unsigned int eth_g_index,
	unsigned char enable
);


int dcli_igmp_snp_show_running_routeport
(
	unsigned short	vlanId,
	IGMP_SNP_PORT_STC *portArray
);

#endif
