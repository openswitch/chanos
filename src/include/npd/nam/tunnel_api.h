#ifndef __NPD_TUNNEL_API_H__
#define __NPD_TUNNEL_API_H__


int nam_tunnel_start_set_entry
(
	unsigned char devNum,
	unsigned int routerArpTunnelStartLineIndex,
	unsigned int tunnelType,
	struct tunnel_item_s	*configPtr
);

int nam_tunnel_start_del_entry
(
	unsigned char devNum,
	unsigned int routerArpTunnelStartLineIndex,
	unsigned int tunnelType
);

int nam_tunnel_nh_set
(
	unsigned char devNum,
	struct tunnel_item_s	*configPtr
);

int nam_tunnel_nh_del
(
	unsigned char devNum,
	unsigned int  nexthopindex
);

int nam_tunnel_set_tcam_ipltt
(
	unsigned char devnum,
	unsigned int hostdip,
	unsigned int hdiplen,
	struct tunnel_item_s	*configPtr
);

int  nam_tunnel_del_tcam_ipltt
(
	unsigned int dstip,
	unsigned int masklen
);

int nam_tunnel_start_set_check
(
	unsigned char devNum,
	unsigned int routerArpTunnelStartLineIndex
);

int nam_tunnel_ipv4_tunnel_term_port_set
(
	unsigned char       devNum,
	unsigned char       port,
	unsigned long		enable
);

int nam_tunnel_term_entry_set
(
	unsigned char devNum,
	unsigned int routerTunnelTermTcamIndex,
	unsigned int  tunnelType,
	struct tunnel_item_s    *configPtr
);

int nam_tunnel_term_entry_del
(
	unsigned char devNum,
	unsigned int routerTunnelTermTcamIndex,
	unsigned int  tunnelType
);

int nam_tunnel_term_entry_invalidate
(
	unsigned char devNum,
	unsigned int routerTunnelTermTcamIndex
);

#endif

