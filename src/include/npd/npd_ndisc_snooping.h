#ifndef _NPD_NDISC_SNOOPING_H
#define _NPD_NDISC_SNOOPINT_H


#define ND_OPT_RDNSS 25

struct nd_msg {
    struct icmp6_hdr icmph;
    ip6_addr	target;
	unsigned char opt[0];
};

struct rs_msg {
	struct icmp6_hdr icmph;
	unsigned char opt[0];
};

struct ra_msg {
    struct icmp6_hdr icmph;
    unsigned int reachable_time;
    unsigned int retrans_timer;
};

/* ND options */
struct ndisc_options {
	struct nd_opt_hdr *nd_opt_array[16];
	struct nd_opt_hdr *nd_opts_ri;
	struct nd_opt_hdr *nd_opts_ri_end;
	struct nd_opt_hdr *nd_useropts;
	struct nd_opt_hdr *nd_useropts_end;
};

#define nd_opts_src_lladdr	nd_opt_array[ND_OPT_SOURCE_LINKADDR]
#define nd_opts_tgt_lladdr	nd_opt_array[ND_OPT_TARGET_LINKADDR]
#define nd_opts_pi		nd_opt_array[ND_OPT_PREFIX_INFORMATION]
#define nd_opts_pi_end		nd_opt_array[0]
#define nd_opts_rh		nd_opt_array[ND_OPT_REDIRECTED_HEADER]
#define nd_opts_mtu		nd_opt_array[ND_OPT_MTU]

/*neighbor discovery database definition*/
#define NPD_NDISCSNP_HASHTBL_NAME   "npdNdiscSnpHashTbl"
#define NPD_NDISCSNP_CFGTBL_NAME    "npdndiscSnpCfgTbl"

#define NPD_NDISCSNP_HASH_IP_SIZE (NPD_NDISCSNP_TABLE_SIZE)
#define NPD_NDISCSNP_HASH_PORT_SIZE  (1024)
#define NPD_NDISCSNP_HASH_MAC_SIZE (1024)


#define syslog_ax_ndiscsnooping_dbg syslog_ax_arpsnooping_dbg
#define syslog_ax_ndiscsnooping_err syslog_ax_arpsnooping_err


#endif  //_NPD_NDISC_SNOOPINT_H


