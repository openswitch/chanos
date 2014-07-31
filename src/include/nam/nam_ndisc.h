#ifndef __NAM_NDISC_H__
#define __NAM_NDISC_H__


#define syslog_ax_nam_ndisc_err syslog_ax_nam_arp_err
#define syslog_ax_nam_ndisc_dbg syslog_ax_nam_arp_dbg

enum nam_ndisc_snoop_op_ent {
	NAM_NDISC_SNOOP_ADD_ITEM = 0,
	NAM_NDISC_SNOOP_DEL_ITEM,
	NAM_NDISC_SNOOP_UPDATE_ITEM,
	NAM_NDISC_SNOOP_ACTION_MAX
};



#endif
