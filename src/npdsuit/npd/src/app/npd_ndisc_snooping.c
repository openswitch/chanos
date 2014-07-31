/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_ndisc_snooping.c
*******************************************************************************/

/*
 *	Neighbour  snooping for IPv6
 *	Linux INET6 implementation
 *
 *Authors:
 *
 * CREATOR:
 *		hujc@autelan.com
 *
 * DESCRIPTION:
 *		APIs used in NPD for ipv6 ndp snooping process.
 *
 * DATE:
 *		02/10/2012	
 *
 *  FILE REVISION NUMBER:
 *  		$Revision: 1.0	
 */
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_ndisc_snooping.h"

#ifdef HAVE_NPD_IPV6
hash_table_index_t *npd_ndiscsnp_haship_index = NULL;
hash_table_index_t *npd_ndiscsnp_hashport_index = NULL;
hash_table_index_t *npd_ndiscsnp_hashmac_index = NULL;
hash_table_index_t *npd_ipv6_nexthop_hash_index = NULL;
array_table_index_t *npd_ndiscsnp_cfg_index = NULL;

db_table_t         *npd_ndiscsnp_dbtbl = NULL;
db_table_t         *npd_ipv6_nexthop_dbtbl = NULL;
db_table_t		   *npd_ndiscsnp_cfgtbl = NULL;
unsigned int ndiscsnp_aging_continue = 0;
extern pthread_mutex_t namItemOpMutex ;
extern pthread_mutex_t nexthopHashMutex ;

int sysKernNeighbourSock = 0;  /*user neighbour synchronization kernel */
unsigned int ndisc_aging_bcast = NPD_FALSE;

#define NDISC_OPT_SPACE(len) (((len)+2+7)&~7)

#define	ARPHRD_IEEE1394	24		/* IEEE 1394 IPv4 - RFC 2734	*/

#define ARPHRD_INFINIBAND 32		/* InfiniBand			*/

#define ETH_ALEN 6

int npd_ndisc_solicit_send(struct ndisc_snooping_item_s *item, ip6_addr *gateway);

netif_event_notifier_t ndisc_snp_netif_notifier =
{
    .netif_event_handle_f = &npd_ndisc_snp_notify_event,
    .netif_relate_handle_f = &npd_ndisc_snp_relate_event
};


/*
 * Return the padding between the option length and the start of the
 * link addr.  Currently only IP-over-InfiniBand needs this, although
 * if RFC 3831 IPv6-over-Fibre Channel is ever implemented it may
 * also need a pad of 2.
 */
static int ndisc_addr_option_pad(unsigned short type)
{
	switch (type) {
	case ARPHRD_INFINIBAND: return 2;
	default:                return 0;
	}
}

static inline int ndisc_is_useropt(struct nd_opt_hdr *opt)
{
	return (opt->nd_opt_type == ND_OPT_RDNSS);
}

static struct ndisc_options *ndisc_parse_options(unsigned char *opt, int opt_len,
						 struct ndisc_options *ndopts)
{
	struct nd_opt_hdr *nd_opt = (struct nd_opt_hdr *)opt;

	if (!nd_opt || opt_len < 0 || !ndopts)
		return NULL;
	memset(ndopts, 0, sizeof(*ndopts));
	while (opt_len) {
		int l;
		if (opt_len < sizeof(struct nd_opt_hdr))
			return NULL;
		l = nd_opt->nd_opt_len << 3;
		if (opt_len < l || l == 0)
			return NULL;
		switch (nd_opt->nd_opt_type) {
		case ND_OPT_SOURCE_LINKADDR:
		case ND_OPT_TARGET_LINKADDR:
		case ND_OPT_MTU:
		case ND_OPT_REDIRECTED_HEADER:
			if (ndopts->nd_opt_array[nd_opt->nd_opt_type]) {
				syslog_ax_ndiscsnooping_dbg( "%s(): duplicated ND6 option found: type=%d\n",
					   __func__,
					   nd_opt->nd_opt_type);
			} else {
				ndopts->nd_opt_array[nd_opt->nd_opt_type] = nd_opt;
			}
			break;
		case ND_OPT_PREFIX_INFORMATION:
			ndopts->nd_opts_pi_end = nd_opt;
			if (!ndopts->nd_opt_array[nd_opt->nd_opt_type])
				ndopts->nd_opt_array[nd_opt->nd_opt_type] = nd_opt;
			break;
#ifdef CONFIG_IPV6_ROUTE_INFO
		case ND_OPT_ROUTE_INFO:
			ndopts->nd_opts_ri_end = nd_opt;
			if (!ndopts->nd_opts_ri)
				ndopts->nd_opts_ri = nd_opt;
			break;
#endif
		default:
			if (ndisc_is_useropt(nd_opt)) {
				ndopts->nd_useropts_end = nd_opt;
				if (!ndopts->nd_useropts)
					ndopts->nd_useropts = nd_opt;
			} else {
				/*
				 * Unknown options must be silently ignored,
				 * to accommodate future extension to the
				 * protocol.
				 */
				syslog_ax_ndiscsnooping_dbg("%s(): ignored unsupported option; type=%d, len=%d\n",
					   __func__,
					   nd_opt->nd_opt_type, nd_opt->nd_opt_len);
			}
		}
		opt_len -= l;
		nd_opt = ((void *)nd_opt) + l;
	}
	return ndopts;
}

static inline unsigned char *ndisc_opt_addr_data(struct nd_opt_hdr *p)
{
	unsigned char *lladdr = (unsigned char *)(p + 1);
	int lladdrlen = p->nd_opt_len << 3;
	int prepad = ndisc_addr_option_pad(ARPHRD_IEEE1394);
	if (lladdrlen != NDISC_OPT_SPACE(ETH_ALEN + prepad))
		return NULL;
	return (lladdr + prepad);
}


/**********************************************************************************
 * npd_ndisc_snooping_find_item_byip
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		ipAddr	- neighbour ip address
 *
 *	OUTPUT:
 *		dbItem	- neighbour information
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_snooping_find_item_byip
(
	ip6_addr  ipAddr,
	struct ndisc_snooping_item_s *dbItem
)
{
	struct ndisc_snooping_item_s data;
	int status;

	memset(&data,0,sizeof(struct ndisc_snooping_item_s));
	memcpy(&data.ipAddr, &ipAddr, sizeof(data.ipAddr));

	status = dbtable_hash_search(npd_ndiscsnp_haship_index,&data,npd_ndisc_snooping_compare_byip,dbItem);
	if(0 != status ) {
		syslog_ax_ndiscsnooping_dbg("not found neighbour item of "IPV6STR"\n", IPV6_2_STR(dbItem->ipAddr));
		return NDISC_RETURN_CODE_NULL_PTR; 
	}

	syslog_ax_ndiscsnooping_dbg("found neighbour item "IPV6STR" on port_index(0x%x) vlan %d if %d\r\n",	\
			IPV6_2_STR(dbItem->ipAddr), dbItem->ifIndex,dbItem->vid,dbItem->l3Index);
	return NDISC_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_ndisc_snooping_learning.l
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_snooping_learning
(
	struct ndisc_snooping_item_s *item
)
{
	unsigned int ret = NDISC_RETURN_CODE_SUCCESS;
	struct ndisc_snooping_item_s dupItem;	
	int status;
	int replace = FALSE;
	unsigned int ndisc_count = 0;
	unsigned int ctrl_state = 0;

	
	memset(&dupItem,0,sizeof(struct ndisc_snooping_item_s));

	dbtable_hash_lock(npd_ndiscsnp_haship_index);

	status = npd_ndisc_snooping_find_item_byip( item->ipAddr, &dupItem);
	if( 0 == status && dupItem.isStatic != TRUE )
	{
        if(!npd_ndisc_snooping_compare(item, &dupItem))
        {
			npd_route_v6_update_by_nhp(item->ipAddr, FALSE);
        	status = dbtable_hash_delete( npd_ndiscsnp_haship_index, &dupItem, &dupItem);
            replace = TRUE;
        }
        else if(dupItem.flag & NDISCSNP_FLAG_DROP)
        {
	        npd_route_v6_update_by_nhp(item->ipAddr, FALSE);
        	status = dbtable_hash_delete( npd_ndiscsnp_haship_index, &dupItem, &dupItem);
            replace = TRUE;
        }
		else if(!(dupItem.flag & NDISCSNP_FLAG_HIT))
		{
			dupItem.flag |= NDISCSNP_FLAG_HIT;
			syslog_ax_ndiscsnooping_dbg("ndisc learning: update ndisc "IPV6STR" by hit\n", \
					IPV6_2_STR(dupItem.ipAddr));
			status = dbtable_hash_update(npd_ndiscsnp_haship_index, &dupItem, &dupItem);
		}
	}

	npd_intf_get_l3intf_ctrl_status(item->l3Index,&ctrl_state);
    if( ((TRUE == replace) || (0 != status)) && (INTF_CTRL_STATE_UP == ctrl_state) )	
    {
		ndisc_count = npd_ndisc_snooping_count_all(); /*To avoid neighbour entries are occupied by Drop entries*/
		if(NPD_NDISCSNP_TABLE_SIZE <= ndisc_count){
			npd_ndisc_snooping_drop_handle();		
		}
		ndisc_count = npd_ndisc_snooping_count_all();
		if(NPD_NDISCSNP_TABLE_SIZE > ndisc_count)
		{
	    	 syslog_ax_ndiscsnooping_dbg("npd_ndisc_snooping_learning: vid %d, vidx %d, s tatic %d, valid %d\r\n", \
	    	                       item->vid, item->vidx, item->isStatic, item->isValid);
	    	 status = dbtable_hash_insert( npd_ndiscsnp_haship_index, item);
	    	 npd_route_v6_update_by_nhp(item->ipAddr, TRUE);
		}
    }
	
	if( 0 != status )
		ret = NDISC_RETURN_CODE_ERR_GENERAL;

	dbtable_hash_unlock(npd_ndiscsnp_haship_index);
	
	return ret;	
}


/**********************************************************************************
 * npd_ipv6_route_nexthop_op_item
 *
 * ndisc snooping database add/delete or other operations.
 *
 *	INPUT:
 *		item - ndisc snooping DB items
 *		action - add or delete operation
 *           dupIfIndex - old ifindex
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ROUTE_RETURN_CODE_SUCCESS - if no error occurred
 *		COMMON_RETURN_CODE_NULL_PTR - if input parameters have null pointer
 *		COMMON_RETURN_CODE_NO_RESOURCE - if no memory allocatable
 *		ROUTE_RETURN_CODE_NOT_FOUND - if item not exists in DB
 *		ROUTE_RETURN_CODE_HASH_OP_FAILED - hash table operation failed 
 *		
 *	NOTE:
 *		ndisc snooping database can only be modified by this API.
 *		Input arguments is viewed as temporary memory,when 
 *		add		- all hash backet data is newly allocated here.
 *		delete	- memory is returned back to system(or say FREE) here.
 *
 **********************************************************************************/
int npd_ipv6_route_nexthop_op_item
(
	struct route_ipv6_nexthop_brief_s *item,
	enum NPD_NEXTHOP_DB_ACTION action,
	unsigned int dupIfIndex
)
{
	int ret = 0;
	struct route_ipv6_nexthop_brief_s data;

	if(NULL == item) {
		syslog_ax_ndiscsnooping_err("npd %s nexthop brief item null pointer error.\r\n",(NEXTHOP_ADD_ITEM==action) ? "add":"del");
		return NDISC_RETURN_CODE_NULL_PTR;
	}
	syslog_ax_ndiscsnooping_dbg("npd nexthop op %d for ip "IPV6STR" l3index 0x%x tblIndex %d\n",\
		action, IPV6_2_STR(item->ipAddr), item->l3Index, item->tblIndex);

	ret = dbtable_hash_search(npd_ipv6_nexthop_hash_index, item, NULL, &data);

	if(NEXTHOP_ADD_ITEM == action) {
		if(0 == ret) {
			ret = dbtable_hash_update(npd_ipv6_nexthop_hash_index, &data, item);
			syslog_ax_ndiscsnooping_dbg("npd nexthop brief dup item ip "IPV6STR" found when add.\r\n", \
				IPV6_2_STR(data.ipAddr));
			return NDISC_RETURN_CODE_SUCCESS;
		}

		ret = dbtable_hash_insert(npd_ipv6_nexthop_hash_index, item);		
	}
	else if(NEXTHOP_DEL_ITEM == action)
	{
		if(0 != ret) {
			syslog_ax_ndiscsnooping_err("npd nexthop brief no item found when delete.\r\n");
			return NDISC_RETURN_CODE_SUCCESS;
		}

		ret = dbtable_hash_delete(npd_ipv6_nexthop_hash_index, &data, &data);
	}
	
	return NDISC_RETURN_CODE_SUCCESS;	
}




/**********************************************************************************
 * npd_ndisc_recv_ns
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_recv_ns(
	npd_ndisc_ctrl_stc *ndisc_ctrl_info,
	struct nd_msg *hdr, int len)
{
	struct nd_neighbor_solicit *msg = (struct nd_neighbor_solicit *)hdr;
	ip6_addr *saddr = &ndisc_ctrl_info->sip;
	unsigned char *lladdr = NULL;
	unsigned int ndoptlen = len - offsetof(struct nd_msg, opt);
	struct ndisc_options ndopts;
	int dad = ipv6_addr_any(saddr);
	struct ndisc_snooping_item_s neigh;
	unsigned int retVal = NDISC_RETURN_CODE_SUCCESS;


	if(dad){
		syslog_ax_ndiscsnooping_dbg("DAD packet doesn't need snooping.\r\n");
		return NDISC_RETURN_CODE_SUCCESS;
	}
	
	if (IN6_IS_ADDR_MULTICAST(&msg->nd_ns_target)) {
		syslog_ax_ndiscsnooping_dbg("ICMPv6 NS: multicast target address");
		return NDISC_RETURN_CODE_ERROR;
	}

	if (!ndisc_parse_options((unsigned char *)(msg+1), ndoptlen, &ndopts)) {
		syslog_ax_ndiscsnooping_dbg("ICMPv6 NS: invalid ND options\n");
		return NDISC_RETURN_CODE_ERROR;
	}

	if (ndopts.nd_opts_src_lladdr) {
		lladdr = ndisc_opt_addr_data(ndopts.nd_opts_src_lladdr);
		if (!lladdr) {
			syslog_ax_ndiscsnooping_dbg("ICMPv6 NS: invalid link-layer address length\n");
			return NDISC_RETURN_CODE_ERROR;
		}
	}
	else{
		return NDISC_RETURN_CODE_SUCCESS;
	}

	/* TODO:Build up ndisc snooping DB item*/
	memset(&neigh, 0, sizeof(struct ndisc_snooping_item_s));
	npd_intf_get_global_l3index(ndisc_ctrl_info->ifIndex, &(neigh.l3Index));
	neigh.isStatic = FALSE;
	neigh.isValid = TRUE;
	memcpy(&neigh.ipAddr, saddr, sizeof(neigh.ipAddr));
	neigh.isTagged = ndisc_ctrl_info->isTagged;
	neigh.vid		= ndisc_ctrl_info->vid;
	neigh.ifIndex  = ndisc_ctrl_info->netif_index;
	memcpy(neigh.mac, lladdr, MAC_ADDRESS_LEN);
	neigh.flag |= NDISCSNP_FLAG_HIT;

	retVal = npd_ndisc_snooping_learning( &neigh );
	if(NDISC_RETURN_CODE_SUCCESS != retVal) 
	{
		syslog_ax_ndiscsnooping_err("npd neighbour snooping learning error %d\n",retVal);
	}
	
	return retVal;
}

/**********************************************************************************
 * npd_ndisc_recv_na
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_recv_na(
	npd_ndisc_ctrl_stc *ndisc_ctrl_info,
	struct nd_msg *hdr, int len)
{
	struct nd_neighbor_advert *msg = (struct nd_neighbor_advert *)hdr;
	ip6_addr *daddr = &ndisc_ctrl_info->dip;
	unsigned char *lladdr = NULL;
	unsigned int ndoptlen = len - offsetof(struct nd_msg, opt);
	struct ndisc_options ndopts;

	struct ndisc_snooping_item_s dbItem;
	unsigned int retVal = NDISC_RETURN_CODE_SUCCESS;

	if (len < sizeof(struct nd_msg)) {
		syslog_ax_ndiscsnooping_err("ICMPv6 NA: packet too short\n");
		return NDISC_RETURN_CODE_ERROR;
	}

	if (IN6_IS_ADDR_MULTICAST(&msg->nd_na_target)) {
		syslog_ax_ndiscsnooping_err("ICMPv6 NA: target address is multicast.\n");
		return NDISC_RETURN_CODE_ERROR;
	}

	if (IN6_IS_ADDR_MULTICAST(daddr) &&
	    msg->nd_na_flags_reserved&ND_NA_FLAG_SOLICITED) 
	{
		syslog_ax_ndiscsnooping_err("ICMPv6 NA: solicited NA is multicasted.\n");
		return NDISC_RETURN_CODE_ERROR;
	}
	

	if (!ndisc_parse_options((unsigned char *)(msg+1), ndoptlen, &ndopts)) {
		syslog_ax_ndiscsnooping_err("ICMPv6 NS: invalid ND option\n");
		return NDISC_RETURN_CODE_ERROR;
	}
	if (ndopts.nd_opts_tgt_lladdr) {
		lladdr = ndisc_opt_addr_data(ndopts.nd_opts_tgt_lladdr);
		if (!lladdr) {
			syslog_ax_ndiscsnooping_err("ICMPv6 NA: invalid link-layer address length\n");
			return NDISC_RETURN_CODE_ERROR;
		}
	}
	else{
		syslog_ax_ndiscsnooping_dbg("ICMPv6 NA: packet have no link-layer address to snoop\r\n");
		return NDISC_RETURN_CODE_ERROR;
	}
#if 0	
	neigh = neigh_lookup(&nd_tbl, &msg->target, dev);

	if (neigh) {
		u8 old_flags = neigh->flags;

		if (neigh->nud_state & NUD_FAILED)
			goto out;

		/*
		 * Don't update the neighbor cache entry on a proxy NA from
		 * ourselves because either the proxied node is off link or it
		 * has already sent a NA to us.
		 */
		if (lladdr && !memcmp(lladdr, dev->dev_addr, dev->addr_len) &&
		    ipv6_devconf.forwarding && ipv6_devconf.proxy_ndp &&
		    pneigh_lookup(&nd_tbl, dev_net(dev), &msg->target, dev, 0)) {
			/* XXX: idev->cnf.prixy_ndp */
			goto out;
		}

		neigh_update(neigh, lladdr,
			     msg->icmph.icmp6_solicited ? NUD_REACHABLE : NUD_STALE,
			     NEIGH_UPDATE_F_WEAK_OVERRIDE|
			     (msg->icmph.icmp6_override ? NEIGH_UPDATE_F_OVERRIDE : 0)|
			     NEIGH_UPDATE_F_OVERRIDE_ISROUTER|
			     (msg->icmph.icmp6_router ? NEIGH_UPDATE_F_ISROUTER : 0));

		if ((old_flags & ~neigh->flags) & NTF_ROUTER) {
			/*
			 * Change: router to host
			 */
			struct rt6_info *rt;
			rt = rt6_get_dflt_router(saddr, dev);
			if (rt)
				ip6_del_rt(rt);
		}

out:
		neigh_release(neigh);
	}
#endif //if 0	
	/* TODO:Build up ndisc snooping DB item*/
	memset(&dbItem, 0, sizeof(struct ndisc_snooping_item_s));
	npd_intf_get_global_l3index(ndisc_ctrl_info->ifIndex, &(dbItem.l3Index));
	dbItem.isStatic = FALSE;
	dbItem.isValid = TRUE;
	memcpy(&dbItem.ipAddr, &ndisc_ctrl_info->sip, sizeof(dbItem.ipAddr));;	/* usually we learning source ip address*/
	dbItem.isTagged = ndisc_ctrl_info->isTagged;
	dbItem.vid		= ndisc_ctrl_info->vid;
	dbItem.ifIndex  = ndisc_ctrl_info->netif_index;
	dbItem.flag     |= NDISCSNP_FLAG_HIT;
	memcpy(dbItem.mac, lladdr, MAC_ADDRESS_LEN);
	retVal = npd_ndisc_snooping_learning( &dbItem );
	if(NDISC_RETURN_CODE_SUCCESS != retVal) 
	{
		syslog_ax_ndiscsnooping_err("npd neighbour snooping learning error %d\n",retVal);
		return retVal;
	}
	return NDISC_RETURN_CODE_SUCCESS;
}



/**********************************************************************************
 * npd_ndisc_recv_rs
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_recv_rs(
	npd_ndisc_ctrl_stc *ndisc_ctrl_info,
	struct nd_msg *hdr, int len)
{
	struct ndisc_snooping_item_s dbItem;
	unsigned int retVal = NDISC_RETURN_CODE_SUCCESS;
	struct nd_router_solicit *rs_msg = (struct nd_router_solicit *)hdr;
	unsigned long ndoptlen = len - sizeof(*rs_msg);
	ip6_addr *saddr = &ndisc_ctrl_info->sip;
	struct ndisc_options ndopts;
	unsigned char *lladdr = NULL;

	if (len < sizeof(*rs_msg))
		return -1;

	/*
	 * Don't update NCE if src = ::;
	 * this implies that the source node has no ip address assigned yet.
	 */
	if (IN6_IS_ADDR_UNSPECIFIED(saddr))
		return -1;

	
	/* Parse ND options */
	if (!ndisc_parse_options((unsigned char *)(rs_msg+1), ndoptlen, &ndopts)) {
		return -1;
	}

	if (ndopts.nd_opts_src_lladdr) {
		lladdr = ndisc_opt_addr_data(ndopts.nd_opts_src_lladdr);
		if (!lladdr)
			return -1;
	}
	else{
		return -1;
	}

	/* TODO:Build up neighbour snooping DB item*/
	memset(&dbItem, 0, sizeof(struct ndisc_snooping_item_s));
	npd_intf_get_global_l3index(ndisc_ctrl_info->ifIndex, &(dbItem.l3Index));
	dbItem.isStatic = FALSE;
	dbItem.isValid = TRUE;
	memcpy(&dbItem.ipAddr, &ndisc_ctrl_info->sip, sizeof(dbItem.ipAddr));	/* usually we learning source ip address*/
	dbItem.isTagged = ndisc_ctrl_info->isTagged;
	dbItem.vid		= ndisc_ctrl_info->vid;
	dbItem.ifIndex  = ndisc_ctrl_info->netif_index;
	dbItem.flag     |= NDISCSNP_FLAG_HIT;
	memcpy(dbItem.mac, lladdr, MAC_ADDRESS_LEN);

	retVal = npd_ndisc_snooping_learning( &dbItem );
	if(NDISC_RETURN_CODE_SUCCESS != retVal) 
	{
		syslog_ax_ndiscsnooping_err("npd neighbour snooping learning error %d\n",retVal);
		return retVal;
	}
	return 0;
	
}



/**********************************************************************************
 * npd_ndisc_router_discovery
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_router_discovery(
	npd_ndisc_ctrl_stc *ndisc_ctrl_info,
	struct nd_msg *hdr, int len)
{
	return 0;
}


/**********************************************************************************
 * npd_ndisc_redirect_rcv
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_redirect_rcv(
	npd_ndisc_ctrl_stc *ndisc_ctrl_info,
	struct nd_msg *hdr, int len)
{
	return 0;
	
}

#define NAM_PACKET_RX_LOCAL_OP_ERR -3
#define NAM_PACKET_RX_HANDLER_NULL  -2
#define NAM_PACKET_RX_TYPE_MISMATCH -1
#define NAM_PACKET_RX_COMPLETE 0
#define NAM_PACKET_RX_DO_MORE 1
/**********************************************************************************
 * npd_ndisc_packet_rx_process
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
int npd_ndisc_packet_rx_process
(
	unsigned int   packet_type,
	unsigned int   netif_index,
	unsigned short vid,
	unsigned char  isTagged,
	unsigned char  *packet,
	unsigned long  length
)
{
	struct ether_header_t *layer2 = NULL;
	struct ip6_hdr 	*layer3 = NULL;
	struct icmp6_hdr *hdr;
	npd_ndisc_ctrl_stc ndisc_ctrl_info;
    unsigned int l3Intf = 0;
	unsigned char *ptr = NULL;
	unsigned char nexthdr;
	unsigned char type = 0;
	int len;
	int learn_mode = 0;

	npd_module_learning_mode_get(netif_index, 0, &learn_mode);
	if (learn_mode == 0)
	{
		return NDISC_RETURN_CODE_SUCCESS;
	}

    //get dev_type and l3_if_index
	npd_syslog_dbg(" %s ", nam_get_packet_type_str(packet_type));
    if((NPD_FALSE == npd_vlan_interface_check(vid,&l3Intf))|| (~0UI == l3Intf))
    {
		if((NPD_FALSE == npd_intf_exist_check(netif_index,&l3Intf)) || (~0UI == l3Intf))
		{
			npd_syslog_dbg(" NO l3 interface found\r\n");
			return NDISC_RETURN_CODE_ERR_GENERAL;
		}
    }


	layer2 = (struct ether_header_t*)(packet);
	if(htons(0x86dd) != layer2->etherType){
		return NDISC_RETURN_CODE_INVALID_PACKET_TYPE;
	}
	
	layer3 = (struct ip6_hdr *)(layer2 + 1);
	
	/* pointer to the 1st exthdr */
	nexthdr = layer3->ip6_nxt;
	/* available length */
	len = layer3->ip6_plen;

	//initiate ndisc ctrl information
	memset(&ndisc_ctrl_info, 0, sizeof(ndisc_ctrl_info));
	memcpy((char*)&ndisc_ctrl_info.sip, (char*)&layer3->ip6_src, sizeof(ndisc_ctrl_info.sip));
	memcpy((char*)&ndisc_ctrl_info.dip, (char*)&layer3->ip6_dst, sizeof(ndisc_ctrl_info.dip));
	ndisc_ctrl_info.netif_index = netif_index;
	ndisc_ctrl_info.ifIndex = l3Intf;
	ndisc_ctrl_info.vid = vid;
	ndisc_ctrl_info.isTagged = isTagged;
	
	/*pointer to current exthdr*/
	ptr = (unsigned char*)(layer3 + 1);

	while (ip6t_ext_hdr(nexthdr)) {
		const struct ip6_ext *hp;
		unsigned int hdrlen;

		/* Is there enough space for the next ext header? */
		if (len < (int)sizeof(struct ip6_ext))
			return NDISC_RETURN_CODE_ERR_GENERAL;
		/* No more exthdr -> evaluate */
		if (nexthdr == NEXTHDR_NONE) {
			return NDISC_RETURN_CODE_ERR_GENERAL;
		}
		 /*ESP -> evaluate */
		if (nexthdr == NEXTHDR_ESP) {
			return NDISC_RETURN_CODE_ERR_GENERAL;
		}

		hp = (struct ip6_ext *)ptr;
		
		/* Calculate the header length */
		if (nexthdr == NEXTHDR_FRAGMENT)
			hdrlen = 8;
		else if (nexthdr == NEXTHDR_AUTH)
			hdrlen = (hp->ip6e_len + 2) << 2;
		else
			hdrlen = (hp->ip6e_len + 1) << 3;
		len -= hdrlen;

		nexthdr = hp->ip6e_nxt;
		ptr = (unsigned char*)((unsigned char *)ptr+hdrlen);
		if(NULL == ptr){
			return NDISC_RETURN_CODE_ERR_GENERAL;
		}
	}

	//get icmp6 header
	if(nexthdr != NEXTHDR_ICMP){
		return NDISC_RETURN_CODE_ERR_GENERAL;
	}
	hdr = (struct icmp6_hdr *)ptr;
	if((NULL == hdr) || len < sizeof(*hdr)){
		return NDISC_RETURN_CODE_ERR_GENERAL;
	}
	
	type = hdr->icmp6_type;
	switch (type) {
	case ND_ROUTER_SOLICIT:
		npd_ndisc_recv_rs(&ndisc_ctrl_info, (struct nd_msg *)hdr, len);
		break;
	case ND_ROUTER_ADVERT:
		npd_ndisc_router_discovery(&ndisc_ctrl_info, (struct nd_msg *)hdr, len);
		break;
	case ND_NEIGHBOR_SOLICIT:
		npd_ndisc_recv_ns(&ndisc_ctrl_info, (struct nd_msg *)hdr, len);
		break;
	case ND_NEIGHBOR_ADVERT:
		npd_ndisc_recv_na(&ndisc_ctrl_info, (struct nd_msg *)hdr, len);
		break;
	case ND_REDIRECT:
		npd_ndisc_redirect_rcv(&ndisc_ctrl_info, (struct nd_msg *)hdr, len);
		break;
	default:
		return NDISC_RETURN_CODE_INVALID_PACKET_TYPE;
	}
	
	return NDISC_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 * npd_ipv6_route_nexthop_find
 *
 * find route next-hop brief item according to L3 index and ip address
 *
 *	INPUT:
 *		ifIndex - L3 interface index
 *		ipAddr - ip address
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		NULL - if no item found
 *		item  - if route nexthop brief item found
 *		
 *	NOTE:
 *
 **********************************************************************************/
int npd_ipv6_route_nexthop_find
(	
	ip6_addr ipAddr,
	struct route_ipv6_nexthop_brief_s *nexthopEntry
)
{
	int ret = 0;
	if( NULL == nexthopEntry )
		return NPD_FALSE;
		
	memcpy((char*)&nexthopEntry->ipAddr, (char*)&ipAddr, sizeof(nexthopEntry->ipAddr));

	ret = dbtable_hash_search(npd_ipv6_nexthop_hash_index, nexthopEntry, NULL, nexthopEntry);
	if( 0 == ret )
	{
		syslog_ax_ndiscsnooping_dbg("npd get nexthop tbl index %#0x for if %#0x ip "IPV6STR" ref %d\r\n",	\
						nexthopEntry->tblIndex, nexthopEntry->l3Index, IPV6_2_STR(nexthopEntry->ipAddr), \
						nexthopEntry->rtUsedCnt);
		return NPD_TRUE;
	}

	syslog_ax_ndiscsnooping_dbg("npd not found nexthop tbl index for ip "IPV6STR"\r\n", IPV6_2_STR(ipAddr));

	return NPD_FALSE;	
}


/**********************************************************************************
 * npd_ipv6_route_nexthop_tblindex_get
 *
 * find route next-hop brief item according to L3 index and ip address
 *
 *	INPUT:
 *		ifIndex - L3 interface index
 *		ipAddr - ip address
 *	
 *	OUTPUT:
 *		tblIndex - next-hop table index which hold the next-hop item info
 *
 * 	RETURN:
 *		ROUTE_RETURN_CODE_SUCCESS - if no error occurred.
 *		COMMON_RETURN_CODE_NULL_PTR - if input parameters have null pointer
 *		COMMON_RETURN_CODE_NO_RESOURCE - if no memory allocatable
 *		ROUTE_RETURN_CODE_NOT_FOUND - if item not exists in DB
 *		ROUTE_RETURN_CODE_HASH_OP_FAILED - hash table operation failed 
 *		ROUTE_RETURN_CODE_ERROR - neighbour occupy failed
 *		
 *	NOTE:
 *		if ip address and ifIndex combination is not exists in the hash table, which 
 *		means ip address has not learned before. Occupy a HW table item first, next
 *		time we learned this will re-fill Next-Hop table and neighbour mac table
 *
 **********************************************************************************/
unsigned int npd_ipv6_route_nexthop_tblindex_get
(
	unsigned int l3Index,
	ip6_addr ipAddr,
	unsigned int *tblIndex
)
{
	int ret = 0;
	struct route_ipv6_nexthop_brief_s data;

	ret = npd_ipv6_route_nexthop_find(ipAddr, &data);
	if(NPD_TRUE != ret) {
		return ROUTE_RETURN_CODE_NO_RESOURCE;
	}
	else {
		*tblIndex = data.tblIndex;
		syslog_ax_ndiscsnooping_dbg("npd got nexthop tbl index %#0x for index %#0x ip "IPV6STR" ref %d\r\n",	\
										*tblIndex, l3Index, IPV6_2_STR(ipAddr), data.rtUsedCnt);
	}

	return ROUTE_RETURN_CODE_SUCCESS;
}



/**********************************************************************************
 * npd_ndiscsnp_dbtbl_app_handle_update
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero.
 *
 **********************************************************************************/
long npd_ndiscsnp_dbtbl_app_handle_update(  void *newItem, void *oldItem )
{
	return 0;
}


/**********************************************************************************
 * npd_ndiscsnp_dbtbl_handle_update
 *
 * function: neighbour snooping database(Hash table) item update callback method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
long npd_ndiscsnp_dbtbl_handle_update(  void *newItem, void *oldItem )
{
	int ret = NDISC_RETURN_CODE_SUCCESS;
	struct ndisc_snooping_item_s *origItem = NULL, *updateItem = NULL;
	struct route_ipv6_nexthop_brief_s nextHopItem;
	unsigned int tblIndex = 0;

	syslog_ax_ndiscsnooping_dbg("npd ndisc handle update\n");
	
	if( ( newItem == NULL ) || ( oldItem == NULL ) )
		return NDISC_RETURN_CODE_ERROR;

	origItem = (struct ndisc_snooping_item_s *)oldItem;
	updateItem = (struct ndisc_snooping_item_s *)newItem;

	if( origItem->isValid != updateItem->isValid )
	{
		if( updateItem->isValid == TRUE )
		{
			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_ndisc_snooping_op_item(updateItem,NDISC_SNOOPING_ADD_ITEM,&tblIndex);	
			pthread_mutex_unlock(&namItemOpMutex);
			if(NDISC_RETURN_CODE_SUCCESS != ret ) {
				syslog_ax_ndiscsnooping_err("nam add item error %d\r\n",ret); 	
				return ret;
			}			

			syslog_ax_ndiscsnooping_dbg("npd set nexthop hash table\r\n");
			memset(&nextHopItem, 0, sizeof(struct route_ipv6_nexthop_brief_s));
			npd_intf_port_check(updateItem->vid, updateItem->ifIndex, &(nextHopItem.l3Index));
			memcpy((char*)&nextHopItem.ipAddr, (char*)&updateItem->ipAddr,sizeof(nextHopItem.ipAddr));
			nextHopItem.tblIndex= tblIndex;
			pthread_mutex_lock(&nexthopHashMutex);
			ret = npd_ipv6_route_nexthop_op_item(&nextHopItem,NEXTHOP_ADD_ITEM,nextHopItem.l3Index); 
			pthread_mutex_unlock(&nexthopHashMutex);
			if(ROUTE_RETURN_CODE_SUCCESS != ret) {
				syslog_ax_ndiscsnooping_err("npd nexthop brief add to db error %d\r\n",ret);
				return NDISC_RETURN_CODE_ERROR;
			}

			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_set_ipv6_host_route(updateItem->ipAddr, updateItem->ifIndex);
			pthread_mutex_unlock(&namItemOpMutex);
			if(NDISC_RETURN_CODE_SUCCESS != ret ) {
				syslog_ax_ndiscsnooping_err("nam add host route error %d\r\n",ret);		
				return ret;
			}						
#if 0			
			syslog_ax_arpsnooping_dbg("npd add route table");
			ret = npd_route_table_op_host_item(&nextHopItem,ROUTE_ADD_ITEM);
			if(ROUTE_RETURN_CODE_SUCCESS!= ret) {
				syslog_ax_ndiscsnooping_err("npd route host add to HW error %d",ret);
				return ARP_RETURN_CODE_ERROR;
			}
#endif			
		}
		else 
		{			
			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_del_ipv6_host_route(updateItem->ipAddr, updateItem->ifIndex);
			pthread_mutex_unlock(&namItemOpMutex);
			if(NDISC_RETURN_CODE_SUCCESS != ret ) {
				syslog_ax_ndiscsnooping_err("nam delete host route error %d\r\n",ret);		
				return ret;
			}
			
			pthread_mutex_lock(&nexthopHashMutex);			
			ret = npd_ipv6_route_nexthop_find(updateItem->ipAddr, &nextHopItem);
			if(NPD_FALSE == ret) {
				syslog_ax_ndiscsnooping_err("npd neighbour set invalid ifindex %#x ip "IPV6STR" not found in sw hash table\r\n", \
						updateItem->l3Index, IPV6_2_STR(updateItem->ipAddr));
				pthread_mutex_unlock(&nexthopHashMutex);
				return NDISC_RETURN_CODE_SUCCESS;
			}

			pthread_mutex_lock(&namItemOpMutex);
			ret = nam_ndisc_snooping_op_item(updateItem,NDISC_SNOOPING_DEL_ITEM,&(nextHopItem.tblIndex));
			pthread_mutex_unlock(&namItemOpMutex);
			if(NDISC_RETURN_CODE_SUCCESS != ret) {
				syslog_ax_ndiscsnooping_err("nam del neighbour snooping item at %d error %d\r\n", nextHopItem.tblIndex,ret);
			}
#if 0
			/* TODO: delete host route from RT in hw*/
			npd_route_table_op_host_item(routeNextHop,ROUTE_DEL_ITEM);
#endif 
			/* delete route_nexthop item only if not used by any route entry*/
			ret = npd_ipv6_route_nexthop_op_item(&nextHopItem,NEXTHOP_DEL_ITEM,0); 
			
			pthread_mutex_unlock(&nexthopHashMutex);			
		}		
	}

	return NDISC_RETURN_CODE_SUCCESS;  
}


/**********************************************************************************
 * npd_ndiscsnp_dbtbl_handle_insert
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
long npd_ndiscsnp_dbtbl_handle_insert( void *newItem )
{
	int ret = NDISC_RETURN_CODE_SUCCESS;
	struct ndisc_snooping_item_s *opItem = NULL;
	struct route_ipv6_nexthop_brief_s nextHopItem;
	unsigned int tblIndex = 0;


	syslog_ax_ndiscsnooping_dbg("npd ndisc handle insert\n");
	
	if( newItem == NULL )
		return NDISC_RETURN_CODE_SUCCESS;

	opItem = (struct ndisc_snooping_item_s *)newItem;

	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_ndisc_snooping_op_item(opItem,NDISC_SNOOPING_ADD_ITEM,&tblIndex);
	pthread_mutex_unlock(&namItemOpMutex);
	if(NDISC_RETURN_CODE_SUCCESS != ret ) {
		syslog_ax_ndiscsnooping_err("nam add item error %d\r\n",ret);		
		return ret;
	}
	memset(&nextHopItem, 0, sizeof(struct route_ipv6_nexthop_brief_s));
	npd_intf_port_check(opItem->vid, opItem->ifIndex, &(nextHopItem.l3Index));
	memcpy((char*)&nextHopItem.ipAddr, (char*)&opItem->ipAddr, sizeof(nextHopItem.ipAddr));
	nextHopItem.tblIndex= tblIndex;

	pthread_mutex_lock(&nexthopHashMutex);
	ret = npd_ipv6_route_nexthop_op_item(&nextHopItem,NEXTHOP_ADD_ITEM,nextHopItem.l3Index); 
	pthread_mutex_unlock(&nexthopHashMutex);
	if(ROUTE_RETURN_CODE_SUCCESS != ret) {
		syslog_ax_ndiscsnooping_err("npd nexthop brief add to db error %d\r\n",ret);
		return ret;
	}
	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_set_ipv6_host_route(opItem->ipAddr, opItem->ifIndex);
	pthread_mutex_unlock(&namItemOpMutex);
	if(NDISC_RETURN_CODE_SUCCESS != ret ) {
		syslog_ax_ndiscsnooping_err("nam add host route error %d\r\n",ret);		
		return ret;
	}
	syslog_ax_ndiscsnooping_dbg("npd add item at hw nexthop table %d ok\r\n",tblIndex);
	return NDISC_RETURN_CODE_SUCCESS;	
}


/**********************************************************************************
 * npd_ndiscsnp_dbtbl_handle_delete
 *
 *  function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		delItem	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		zero if no error occur,otherwise corresponding error code.
 *
 **********************************************************************************/
long npd_ndiscsnp_dbtbl_handle_delete( void *delItem )
{
	int ret = NDISC_RETURN_CODE_SUCCESS;
	struct ndisc_snooping_item_s *opItem = NULL;
	struct route_ipv6_nexthop_brief_s routeNextHop;

	syslog_ax_ndiscsnooping_dbg("npd ndisc handle delete\n");

	if( delItem == NULL )
		return NDISC_RETURN_CODE_SUCCESS;

	opItem = (struct ndisc_snooping_item_s *)delItem;

	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_del_ipv6_host_route(opItem->ipAddr, opItem->ifIndex);
	pthread_mutex_unlock(&namItemOpMutex);
	if(NDISC_RETURN_CODE_SUCCESS != ret ) {
		syslog_ax_ndiscsnooping_err("nam delete host route error %d\r\n",ret);		
		return ret;
	}
	
	pthread_mutex_lock(&nexthopHashMutex);
	ret = npd_ipv6_route_nexthop_find(opItem->ipAddr, &routeNextHop);
	pthread_mutex_unlock(&nexthopHashMutex);
	
	if(NPD_FALSE == ret) {
		syslog_ax_ndiscsnooping_err("npd delete ifindex %#x ip "IPV6STR" not found in sw hash table\r\n", \
				opItem->l3Index, IPV6_2_STR(opItem->ipAddr));		
		return NDISC_RETURN_CODE_SUCCESS;
	}
	
	syslog_ax_ndiscsnooping_dbg("npd delete ifindex %#x ip "IPV6STR" found at %d ref %d of HW table\r\n", \
			opItem->l3Index, IPV6_2_STR(opItem->ipAddr), routeNextHop.tblIndex, \
			routeNextHop.rtUsedCnt);

	
	pthread_mutex_lock(&namItemOpMutex);
	ret = nam_ndisc_snooping_op_item(opItem,NDISC_SNOOPING_DEL_ITEM,&(routeNextHop.tblIndex));
	pthread_mutex_unlock(&namItemOpMutex);
	if(NDISC_RETURN_CODE_SUCCESS != ret) {
		syslog_ax_ndiscsnooping_err("nam del ndisc snooping item at %d error %d\r\n",routeNextHop.tblIndex,ret);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	else {
		syslog_ax_ndiscsnooping_dbg("nam del ndisc snooping item at %d of HW table ok\r\n",	routeNextHop.tblIndex);
	}

#if 0
	/* TODO: delete host route from RT in hw*/
	npd_route_table_op_host_item(routeNextHop,ROUTE_DEL_ITEM);
#endif

	/* delete route_nexthop item only if not used by any route entry*/
	ret = npd_ipv6_route_nexthop_op_item(&routeNextHop,NEXTHOP_DEL_ITEM,0); 
	
	syslog_ax_ndiscsnooping_dbg("npd del item at ndisc table ok\r\n");
	return ret;
}


/**********************************************************************************
 * npd_ndiscsnp_dbtbl_app_handle_delete
 *
 * function: do nothing.
 *
 *	INPUT:
 *		delItem	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0.
 *
 **********************************************************************************/
long npd_ndiscsnp_dbtbl_app_handle_delete(  void *delItem )
{
	return 0;
}


#define NDM_BUFLENGTH  (4096)
void form_Request_for_set_kneighs(unsigned char* buf, struct ndisc_snooping_item_s *item)
{
	struct nlmsghdr* nl;
	struct ndmsg*   ndm;
	struct nlattr *nl_attr = NULL;
	unsigned char *attr_data = NULL;
	struct nda_cacheinfo ci;

	
  	bzero(buf, NDM_BUFLENGTH);
	nl = (struct nlmsghdr *)buf;
  	nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
  	nl->nlmsg_flags = 0;
  	nl->nlmsg_type = RTM_NEWNEIGH;
  	nl->nlmsg_pid = 0;
  	nl->nlmsg_seq = 0;

	ndm = (struct ndmsg*)(nl+1);
  	ndm->ndm_family = AF_INET6;
  	ndm->ndm_type = RTN_UNICAST;
  	ndm->ndm_flags = NTF_ROUTER;
  	ndm->ndm_state = NUD_REACHABLE | NUD_PERMANENT;
  	ndm->ndm_ifindex= item->l3Index;
  	ndm->ndm_pad1= 0;
  	ndm->ndm_pad2= 0;
  	
  	nl_attr = (struct nlattr *)(ndm+1);
  	nl_attr->nla_len = IPV6_MAX_BYTELEN+ NLA_HDRLEN;
  	nl_attr->nla_type = NDA_DST;
  	attr_data = (unsigned char *)(nl_attr + 1);
  	memcpy(attr_data, &item->ipAddr, IPV6_MAX_BYTELEN);

  	nl_attr = (struct nlattr*)((unsigned char*)(nl_attr) + nl_attr->nla_len);
  	nl_attr->nla_len = MAC_ADDR_LEN+ NLA_HDRLEN;
  	nl_attr->nla_type = NDA_LLADDR;
  	attr_data = (unsigned char *)(nl_attr + 1);
  	memcpy(attr_data, item->mac, MAC_ADDR_LEN);



	ci.ndm_used	 = 0;//jiffies_to_clock_t(now - neigh->used);
	ci.ndm_confirmed = 0;//jiffies_to_clock_t(now - neigh->confirmed);
	ci.ndm_updated	 = 0;//jiffies_to_clock_t(now - neigh->updated);
	ci.ndm_refcnt	 = 1;//atomic_read(&neigh->refcnt) - 1;
  	
  	nl_attr = (struct nlattr*)((unsigned char*)(nl_attr) + nl_attr->nla_len);
  	nl_attr->nla_len = sizeof(struct nda_cacheinfo) + NLA_HDRLEN;
  	nl_attr->nla_type = NDA_CACHEINFO;
  	attr_data = (unsigned char *)(nl_attr + 1);
  	memcpy(attr_data, &ci, sizeof(struct nda_cacheinfo));
}

int send_Request_for_set_kneighs(int sockfd,unsigned char *buf)
{
	struct msghdr msg;
	struct sockaddr_nl pa;
	struct nlmsghdr* nl;
	struct iovec iov;

  	bzero(&pa, sizeof(pa));
  	pa.nl_family = AF_NETLINK;
  	bzero(&msg, sizeof(msg));
  	msg.msg_name = (void *) &pa;
  	msg.msg_namelen = sizeof(pa);

	nl = (struct nlmsghdr* )buf;

  	iov.iov_base = (void *)nl;
  	iov.iov_len = nl->nlmsg_len;
  	msg.msg_iov = &iov;
  	msg.msg_iovlen = 1;
  	return sendmsg(sockfd, &msg, 0);
}



/**********************************************************************************
 * npd_ndisc_snooping_create_kern_neighbour
 *	This routine is used to create kern static ndisc entry.
 *
 *	INPUT:
 *		ipAddr		-- 	ip address
 *		mac			--	MAC address
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS 	- 	if no error occurred
 *		NDISC_RETURN_CODE_ERROR		-	if error occurs
 *		
 *	NOTE:
 *
 **********************************************************************************/
int npd_ndisc_snooping_create_kern_neighbour
(
	struct ndisc_snooping_item_s *item
)
{
	int fd;
	struct sockaddr_nl la;
	unsigned char buf[NDM_BUFLENGTH] = {0};

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(fd<0)
	{
		 syslog_ax_route_err("create socket error when copy fib to asic\n");
		return NPD_FAIL;
	}
	
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
#ifdef CPU_ARM_XCAT
	la.nl_pid = syscall(SYS_gettid);
#endif
	if(bind(fd, (struct sockaddr*) &la, sizeof(la)))
	{
		syslog_ax_route_err("bind socket error when copy fib to asic\n");
		close(fd);
		return NPD_FAIL;
	}

	//form_Request_for_set_kneighs(buf, item);
	{
		struct nlmsghdr* nl;
		struct ndmsg*   ndm;
		struct nlattr *nl_attr = NULL;
		unsigned char *attr_data = NULL;
		//struct nda_cacheinfo ci;

		
	  	bzero(buf, NDM_BUFLENGTH);
		nl = (struct nlmsghdr *)buf;
	  	nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg))+NLA_HDRLEN*2 + IPV6_MAX_BYTELEN+ MAC_ADDR_LEN;
	  	nl->nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_REPLACE;
	  	nl->nlmsg_type = RTM_NEWNEIGH;
	  	nl->nlmsg_pid = 0;
	  	nl->nlmsg_seq = 0;

		ndm = (struct ndmsg*)(nl+1);
	  	ndm->ndm_family = AF_INET6;
	  	//ndm->ndm_type = RTN_UNICAST;
	  	//ndm->ndm_flags = NTF_ROUTER;
	  	ndm->ndm_state = NUD_PERMANENT;
	  	ndm->ndm_ifindex= item->l3Index;
	  	ndm->ndm_pad1= 0;
	  	ndm->ndm_pad2= 0;
	  	
	  	nl_attr = (struct nlattr *)(ndm+1);
	  	nl_attr->nla_len = IPV6_MAX_BYTELEN+ NLA_HDRLEN;
	  	nl_attr->nla_type = NDA_DST;
	  	attr_data = (unsigned char *)(nl_attr + 1);
	  	memcpy(attr_data, &item->ipAddr, IPV6_MAX_BYTELEN);

	  	nl_attr = (struct nlattr*)((unsigned char*)(nl_attr) + nl_attr->nla_len);
	  	nl_attr->nla_len = MAC_ADDR_LEN+ NLA_HDRLEN;
	  	nl_attr->nla_type = NDA_LLADDR;
	  	attr_data = (unsigned char *)(nl_attr + 1);
	  	memcpy(attr_data, item->mac, MAC_ADDR_LEN);
	}
	send_Request_for_set_kneighs(fd,buf);
	
	close(fd);
    syslog_ax_ndiscsnooping_dbg("npd create kern static ndisc success \n");
	return NDISC_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_ndisc_snooping_del_kern_neighbour
 *
 *  	del kern ndisc info learned 
 *
 *	INPUT:
 *		item  - the item which we want to delete from the kernal
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS  - delete success
 *		NDISC_RETURN_CODE_ERROR  -  delete failed
 *		
 *	NOTE:
 *
 **********************************************************************************/
int npd_ndisc_snooping_del_kern_neighbour
(
	struct ndisc_snooping_item_s *item
)
{
	int fd;
	struct sockaddr_nl la;
	unsigned char buf[NDM_BUFLENGTH] = {0};

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(fd<0)
	{
		 syslog_ax_route_err("create socket error when copy fib to asic\n");
		return NPD_FAIL;
	}
	
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
#ifdef CPU_ARM_XCAT
	la.nl_pid = syscall(SYS_gettid);
#endif
	if(bind(fd, (struct sockaddr*) &la, sizeof(la)))
	{
		syslog_ax_route_err("bind socket error when copy fib to asic\n");
		close(fd);
		return NPD_FAIL;
	}

	{
		struct nlmsghdr* nl;
		struct ndmsg*	ndm;
		struct nlattr *nl_attr = NULL;
		unsigned char *attr_data = NULL;
		
		bzero(buf, NDM_BUFLENGTH);
		nl = (struct nlmsghdr *)buf;
		nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg))+NLA_HDRLEN*2 + IPV6_MAX_BYTELEN+ MAC_ADDR_LEN;
		nl->nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_REPLACE;
		nl->nlmsg_type = RTM_DELNEIGH;
		nl->nlmsg_pid = 0;
		nl->nlmsg_seq = 0;

		ndm = (struct ndmsg*)(nl+1);
		ndm->ndm_family = AF_INET6;
		ndm->ndm_state = NUD_PERMANENT;
		ndm->ndm_ifindex= item->l3Index;
		ndm->ndm_pad1= 0;
		ndm->ndm_pad2= 0;
		
		nl_attr = (struct nlattr *)(ndm+1);
		nl_attr->nla_len = IPV6_MAX_BYTELEN+ NLA_HDRLEN;
		nl_attr->nla_type = NDA_DST;
		attr_data = (unsigned char *)(nl_attr + 1);
		memcpy(attr_data, &item->ipAddr, IPV6_MAX_BYTELEN);

		nl_attr = (struct nlattr*)((unsigned char*)(nl_attr) + nl_attr->nla_len);
		nl_attr->nla_len = MAC_ADDR_LEN+ NLA_HDRLEN;
		nl_attr->nla_type = NDA_LLADDR;
		attr_data = (unsigned char *)(nl_attr + 1);
		memcpy(attr_data, item->mac, MAC_ADDR_LEN);
		
		send_Request_for_set_kneighs(fd,buf);
	}

	close(fd);
	syslog_ax_ndiscsnooping_dbg("npd delete kern static ndisc success \n");
	return NDISC_RETURN_CODE_SUCCESS;
}



/************************************************************************************
 *		NPD ipv6  neighbour and nexthop hash operation
 *
 ************************************************************************************/
/**********************************************************************************
 * npd_ndisc_snooping_key_generate
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_key_generate
(
	void *data
)
{
	unsigned int tmp;
	unsigned int key = 0;
	struct ndisc_snooping_item_s *item = NULL;


	item = (struct ndisc_snooping_item_s *)data;
	if(NULL == item) {
		syslog_ax_ndiscsnooping_err("npd ndisc snooping items make key null pointers error.\r\n");
		return ~0UI;
	}

	tmp = item->ipAddr.u6_addr32[0] ^ item->ipAddr.u6_addr32[1]\
		^ item->ipAddr.u6_addr32[2] ^ item->ipAddr.u6_addr32[3];
    key = jhash_1word(tmp, 0x35798642);
    key %= NPD_NDISCSNP_HASH_IP_SIZE;

	return key;
}


/**********************************************************************************
 * npd_ndisc_snooping_key_port_generate
 *
 * function: neighbour snooping database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_key_port_generate
(
	void *data	
)
{
	unsigned int key = 0; /*for hash key calculate*/
	struct ndisc_snooping_item_s *item = (struct ndisc_snooping_item_s *)data;

	if(NULL == item) {
		syslog_ax_ndiscsnooping_err("npd ndisc snooping items make key null pointers error.\r\n");
		return ~0UI;
	}
	
	key = (item->ifIndex >> 14)& 0x3FF ;
	
	key %= (NPD_NDISCSNP_HASH_PORT_SIZE);
	
	return key;
}



/**********************************************************************************
 * npd_ndisc_snooping_key_mac_generate
 *
 * function: neighbour database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- neighbour database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_key_mac_generate
(
	void *data	
)
{
	unsigned int key = 0; /*for hash key calculate*/
	unsigned char mac[MAC_ADDR_LEN] = {0,0,0,0,0,0};
	unsigned int tmpData = 0;
	unsigned int tmpData1 = 0;
	unsigned int tmpData2 = 0;
	int i = 0;
	struct ndisc_snooping_item_s *item = (struct ndisc_snooping_item_s *)data;

	if(NULL == item) {
		syslog_ax_ndiscsnooping_err("npd ndisc snooping items make key null pointers error.\r\n");
		return ~0UL;
	}
	memcpy(mac,item->mac,MAC_ADDR_LEN);
	for(i = 0; i < 3; i++){
		tmpData1 = (tmpData1<<8) + mac[i];
	}
	for(i = 3; i < 6; i++){
		tmpData2 = (tmpData2<<8) + mac[i];
	}

	tmpData = tmpData1 + tmpData2;
	key = tmpData%(NPD_NDISCSNP_HASH_MAC_SIZE);
	
	return key;
}

/**********************************************************************************
 * npd_ndisc_snooping_compare
 *
 * function: compare two of neighbour snooping database(Hash table) items
 *
 *	INPUT:
 *		itemA	- neighbour snooping database item
 *		itemB	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_compare
(
	void * data1,
	void * data2	
)
{
	struct ndisc_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 )
	{
		return NPD_FALSE;
	}

	itemA = (struct ndisc_snooping_item_s *)data1;
	itemB = (struct ndisc_snooping_item_s *)data2;

	if(itemA->ifIndex!=itemB->ifIndex) {/* L3 intf index*/
		return NPD_FALSE;
	}

	if(0 != memcmp((char*)itemA->mac,(char*)itemB->mac,MAC_ADDRESS_LEN)) { /* MAC*/
		return NPD_FALSE;
	}

	if(0 != memcmp((char*)&itemA->ipAddr, (char*)&itemB->ipAddr, sizeof(itemA->ipAddr))) { /* IP Address*/
		return NPD_FALSE;
	}
	
	return TRUE;
}


/**********************************************************************************
 * npd_ndisc_snooping_compare_byip
 *
 * function: compare two of neighbour snooping database(Hash table) items
 *
 *	INPUT:
 *		itemA	- neighbour snooping database item
 *		itemB	- neighbour snooping database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_compare_byip
(
	void * data1,
	void * data2	
)
{
	struct ndisc_snooping_item_s *itemA, *itemB;

	
	if(NULL == data1 || NULL == data2 ) {
		return NPD_FALSE;
	}

	itemA = (struct ndisc_snooping_item_s *)data1;
	itemB = (struct ndisc_snooping_item_s *)data2;

	if(0 != memcmp(&itemA->ipAddr, &itemB->ipAddr, sizeof(itemA->ipAddr))) { /* IP Address*/
		return NPD_FALSE;
	}

	return NPD_TRUE;

}

/**********************************************************************************
 * npd_ndisc_snooping_filter_by_ifindex
 *
 *  	filter neighbour snooping by eth port
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *		data - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE - if neighbour item has different port info from data.
 *		TRUE - if neighbour item has the same port info as data.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_filter_by_ifindex
(
	void * data1,
	void * data2	
)
{
	struct ndisc_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return NPD_FALSE;
	}

	itemA = (struct ndisc_snooping_item_s *)data1;
	itemB = (struct ndisc_snooping_item_s *)data2;
	

	if( itemA->ifIndex != itemB->ifIndex ) {
		return NPD_FALSE;
	}

	return NPD_TRUE;
}

/**********************************************************************************
 * npd_ndisc_snooping_filter_by_l3index
 *
 *  	filter neighbour snooping by vlanId
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *		data - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE - if neighbour item has different port info from data.
 *		TRUE - if neighbour item has the same port info as data.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_filter_by_l3index
(
	void * data1,
	void * data2	
)
{
	struct ndisc_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return NPD_FALSE;
	}

	itemA = (struct ndisc_snooping_item_s *)data1;
	itemB = (struct ndisc_snooping_item_s *)data2;
	
	if( itemA->l3Index != itemB->l3Index ) {
		return NPD_FALSE;
	}

	return NPD_TRUE;
}

/**********************************************************************************
 * npd_ndisc_snooping_filter_by_vid
 *
 *  	filter neighbour snooping by vid
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *		data - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE - if neighbour item has different port info from data.
 *		TRUE - if neighbour item has the same port info as data.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_filter_by_vid
(
	void * data1,
	void * data2	
)
{
	struct ndisc_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return NPD_FALSE;
	}

	itemA = (struct ndisc_snooping_item_s *)data1;
	itemB = (struct ndisc_snooping_item_s *)data2;

	if( itemA->vid != itemB->vid )
		return NPD_FALSE;

	return NPD_TRUE;
}


/**********************************************************************************
 * npd_ndisc_snooping_filter_by_mac
 *
 *  	filter neighbour snooping by mac
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *		data - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE - if neighbour item has different port info from data.
 *		TRUE - if neighbour item has the same port info as data.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_filter_by_mac
(
	void * data1,
	void * data2	
)
{
	struct ndisc_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return NPD_FALSE;
	}

	itemA = (struct ndisc_snooping_item_s *)data1;
	itemB = (struct ndisc_snooping_item_s *)data2;
	
	if( memcmp((char*)itemA->mac,(char*)itemB->mac,MAC_ADDRESS_LEN) ){
		return NPD_FALSE;
	}

	return NPD_TRUE;
}

/**********************************************************************************
 * npd_ndisc_snooping_filter_by_macvid
 *
 *  	filter neighbour snooping by mac
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *		data - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE - if neighbour item has different port info from data.
 *		TRUE - if neighbour item has the same port info as data.
 *		
 *	NOTE:
 *
 **********************************************************************************/
int npd_ndisc_snooping_filter_by_macvid
(
	void * data1,
	void * data2	
)
{
	struct ndisc_snooping_item_s *itemA, *itemB;
	
	if(NULL == data1 || NULL == data2 ) {
		return NPD_FALSE;
	}

	itemA = (struct ndisc_snooping_item_s *)data1;
	itemB = (struct ndisc_snooping_item_s *)data2;
	
	if( memcmp((char*)itemA->mac,(char*)itemB->mac,MAC_ADDRESS_LEN) ){
		return NPD_FALSE;
	}
	else if(itemA->vid != itemB->vid ) {
		return NPD_FALSE;
	}
		
	return NPD_TRUE;
}


/**********************************************************************************
 * npd_ndisc_snooping_filter_by_static
 *
 *  	filter neighbour snooping by isStatic
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE - find out neighbour item not matched
 *		TRUE - find out neighbour item matched(with the same static attribute
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_filter_by_static
(
	void *data1,
	void* data2
)
{
	struct ndisc_snooping_item_s *itemA = (struct ndisc_snooping_item_s *)data1;
	struct ndisc_snooping_item_s *itemB = (struct ndisc_snooping_item_s *)data2;
	
	if((NULL == itemA)||(NULL == itemB))
		return NPD_FALSE;

	if(itemA->isStatic != itemB->isStatic )
		return NPD_FALSE;

	return NPD_TRUE;
}

 

/*********************************************************
 * npd_ndisc_snooping_filter_by_network
 * 
 * INPUT:
 *     itemA  -- neighbour item 
 *     prefix_ipv6_addr  -- ipv6 address and mask len,  
 * RETURN:
 *     TRUE -- if matched 
 *     FALSE -- not matched
 *
 **********************************************************/
unsigned int npd_ndisc_snooping_filter_by_network
(
    void *data1,
    void *data2
)
{
	struct ndisc_snooping_item_s * itemA = (struct ndisc_snooping_item_s *)data1;
	prefix_ipv6_stc *prefix_ipv6_addr = (prefix_ipv6_stc *)data2;
	ip6_addr v6_mask;
	
    if((NULL == itemA)||(NULL == prefix_ipv6_addr)){
		return FALSE;
    }

    memset(&v6_mask, 0, sizeof(v6_mask));
    lib_get_maskv6_from_masklen(prefix_ipv6_addr->prefixlen, &v6_mask);
	if(IPV6_NET_EQUAL(itemA->ipAddr, prefix_ipv6_addr->prefix, v6_mask, v6_mask)){
		return TRUE;
	}
	return FALSE;
}

/**********************************************************************************
 * npd_ndisc_snooping_del_all
 *
 *  	Delete static and not static ndisc snooping info in both sw and hw
 *
 *	INPUT:
 *		item - static ndisc snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS  - if success
 *		NDISC_RETURN_CODE_NULL_PTR  - if the item is null
 *		NDISC_RETURN_CODE_NAM_ERROR  - nam operation failed
 *		NDISC_RETURN_CODE_HASH_OP_FAILED  - hash table operation failed
 *		
 *	NOTE:
 * 
 *
 **********************************************************************************/
int npd_ndisc_snooping_del_all
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag    
)
{
	struct ndisc_snooping_item_s *item = (struct ndisc_snooping_item_s *)data;
	struct ndisc_snooping_item_s ndisc_item;
	int ret = NDISC_RETURN_CODE_SUCCESS;
	int status = 0;	
	
	if(NULL == item) {
		return NDISC_RETURN_CODE_NULL_PTR;
	}
	/*first del kernel neighbour*/
	if((kern_del_flag == TRUE)&&\
		((FALSE == item->isStatic)||(TRUE == item->isValid))) 
	{		
		npd_ndisc_snooping_del_kern_neighbour(item);
		item->isValid = FALSE;
	}

	status = npd_route_v6_update_by_nhp(item->ipAddr, FALSE);

	status = dbtable_hash_delete(npd_ndiscsnp_haship_index, item, &ndisc_item);

	return ret;	
}

/**********************************************************************************
 * npd_ndisc_snooping_del
 *
 *  	Delete neighbour snooping info in both sw and hw
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS  - if success
 *		COMMON_RETURN_CODE_NULL_PTR  - if the item is null
 *		NDISC_RETURN_CODE_NAM_ERROR  - nam operation failed
 *		NDISC_RETURN_CODE_HASH_OP_FAILED  - hash table operation failed
 *		NDISC_RETURN_CODE_NOTEXISTS  -  the neighbour not a dynamic one
 *		
 *	NOTE:
 *		this procedure treat neighbour snooping items found in hash tables, so the 'item' is already found
 *		when calling the procedure, no hash table search done here.
 *
 **********************************************************************************/
int npd_ndisc_snooping_del_dynamic
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag
)
{
	int ret;
	struct ndisc_snooping_item_s *item = (struct ndisc_snooping_item_s *)data;

	if( item == NULL )
		return NDISC_RETURN_CODE_ERROR;
		
	if(TRUE == item->isStatic) {
		return NDISC_RETURN_CODE_NOTEXISTS;
	}
	
    ret = npd_ndisc_snooping_del_all(hash, item,kern_del_flag);

	return ret; 
}

/**********************************************************************************
 * npd_ndisc_snooping_del_static
 *
 *  	Delete ndisc snooping info in both sw and hw if it is a static ndisc
 *
 *	INPUT:
 *		item - ndisc snooping item info
 *           kern_del_flag - delete the neighbour from kern or not
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS  - if success
 *		COMMON_RETURN_CODE_NULL_PTR  - if the item is null
 *		NDISC_RETURN_CODE_NAM_ERROR  - nam operation failed
 *		NDISC_RETURN_CODE_HASH_OP_FAILED  - hash table operation failed
 *		NDISC_RETURN_CODE_STASTIC_NOTEXIST  - the static ndisc not exists
 *		
 *	NOTE:
 *		this procedure treat ndisc snooping items found in hash tables, so the 'item' is already found
 *		when calling the procedure, no hash table search done here.
 *
 **********************************************************************************/
int npd_ndisc_snooping_del_static
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag
)
{
	/*not static neighbour don't delete*/
	struct ndisc_snooping_item_s *item = (struct ndisc_snooping_item_s *)data;
	if( item == NULL)
		return NDISC_RETURN_CODE_ERROR;
	
	if(TRUE != item->isStatic) {
		return NDISC_RETURN_CODE_STASTIC_NOTEXIST;
	}
	
    return npd_ndisc_snooping_del_all(NULL, item,kern_del_flag);
}


/*****************************************************
*npd_static_ndisc_set_invalid
*		set static ndisc invalid,delete from hardware and kernal
* INPUT:
*		item  - the ndisc item we want to set 
* OUTPUT:
*		item  - the ndisc item we set
* RETURN:
*		NDISC_RETURN_CODE_SUCCESS   - set success
*		COMMON_RETURN_CODE_NULL_PTR  - the item is null
*		NDISC_RETURN_CODE_ERROR  - the ndisc is not static or already invalid
*		NDISC_RETURN_CODE_HASH_OP_FAILED  - hash table operation failed
* NOTE:
*
******************************************************/
int npd_ndisc_snooping_static_valid_set
(
	hash_table_index_t *hash, 
	void *data,
	unsigned int flag
)
{
	int ret = NDISC_RETURN_CODE_SUCCESS;
	int status = 0;
	struct ndisc_snooping_item_s *item = (struct ndisc_snooping_item_s *)data;

	if( NULL == item )
		return NDISC_RETURN_CODE_ERROR;

	if( item->isStatic != TRUE ) {
		syslog_ax_ndiscsnooping_err(" neighbour "IPV6STR" check flag fail: static %d Valid %d, flag %d\r\n",
										IPV6_2_STR(item->ipAddr),item->isStatic, item->isValid, flag);
		return NDISC_RETURN_CODE_ERROR;
	}	

	status = npd_ndisc_snooping_valid_check(item->ipAddr, item->ifIndex, &item->l3Index, &item->vid);
	status &= flag;
	if( item->isValid != status )
	{
		if(status)
		{
			ret = npd_route_v6_update_by_nhp(item->ipAddr, FALSE);
			ret = npd_ndisc_snooping_del_kern_neighbour(item);
		}	
		else{
			ret = npd_ndisc_snooping_create_kern_neighbour(item);
		}
		
		item->isValid = status;
		
		ret = dbtable_hash_update(npd_ndiscsnp_haship_index, item, item);
		if( 0 != ret ) {  
			syslog_ax_ndiscsnooping_err(" ndisc "IPV6STR" update flag fail: flag %d\r\n", IPV6_2_STR(item->ipAddr), flag );
			return NDISC_RETURN_CODE_ERROR;
		}
		else if(flag == TRUE )
		{
			ret = npd_route_v6_update_by_nhp(item->ipAddr, TRUE);
		}
	}

	return NDISC_RETURN_CODE_SUCCESS;
}

/********************************************************
 * npd_ndisc_snooping_static_valid_set_by_ifindex
 *
 *  	set the static ndisc items on this port to valid or invalid
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *		isVaild  - set valid or invalid
 *				0 - set invalid
 *				1 - set valid	
 *	OUTPUT:
 *		NULL
 * 	RETURN:
 *		ndisc item number set on this eth port.
 *		
 *	NOTE:
 *
 *********************************************************/
unsigned int npd_ndisc_snooping_static_valid_set_by_l3index
(
	unsigned int   ifIndex,
	unsigned int isValid
)
{
	unsigned int retVal = 0;
	struct ndisc_snooping_item_s comparator;

	memset(&comparator, 0,sizeof(struct ndisc_snooping_item_s));	
	comparator.l3Index = ifIndex;
	
	retVal = dbtable_hash_traversal(npd_ndiscsnp_hashport_index, isValid,&comparator,\
					npd_ndisc_snooping_filter_by_l3index,npd_ndisc_snooping_static_valid_set);

	if(retVal > 0){
	    syslog_ax_ndiscsnooping_dbg("npd static ndisc set %s on eth-port %#x total %d items set\n", \
					    isValid ? "Valid":"Invalid",ifIndex,retVal);
	}
	return retVal;
}

/********************************************************
 * npd_ndisc_snooping_static_valid_set_by_ifindex
 *
 *  	set the static ndisc items on this port to valid or invalid
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *		isVaild  - set valid or invalid
 *				0 - set invalid
 *				1 - set valid	
 *	OUTPUT:
 *		NULL
 * 	RETURN:
 *		ndisc item number set on this eth port.
 *		
 *	NOTE:
 *
 *********************************************************/
unsigned int npd_ndisc_snooping_static_valid_set_by_net
(
	ip6_addr *ipaddr,
	int masklen,
	unsigned int isValid
)
{
	unsigned int retVal = 0;
	prefix_ipv6_stc prefix;

	memset(&prefix, 0,sizeof(prefix_ipv6_stc));	
	prefix.prefix = *ipaddr;
	prefix.prefixlen = masklen;
	
	retVal = dbtable_hash_traversal(npd_ndiscsnp_hashport_index, isValid,&prefix,\
					npd_ndisc_snooping_filter_by_network,npd_ndisc_snooping_static_valid_set);

	if(retVal > 0){
	    syslog_ax_ndiscsnooping_dbg("npd static ndisc set %s for network "IPV6STR" masklen %d total %d items set\n", \
					    isValid ? "Valid":"Invalid",IPV6_2_STR(*ipaddr),masklen,retVal);
	}
	return retVal;
}

/**********************************************************************************
 * npd_ndisc_snooping_del_by_vid_L3index
 *
 *  	clear ndisc info learned on this port and this vlan 
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *          vid - vlan id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ndisc item number cleared on this eth port and vlan.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_del_by_l3index
(	
	unsigned int   L3ifindex
)
{
	unsigned int retVal = 0;
	struct ndisc_snooping_item_s data;
	unsigned int kern_del_flag = TRUE;

	memset(&data, 0,sizeof(struct ndisc_snooping_item_s));
	data.l3Index = L3ifindex;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal(npd_ndiscsnp_hashport_index,kern_del_flag,&data,\
								npd_ndisc_snooping_filter_by_l3index,npd_ndisc_snooping_del_dynamic);
	
	syslog_ax_ndiscsnooping_dbg("npd clear %d neighbour items on l3 index %#0x \n",retVal,L3ifindex);
	
	return retVal;
}



/**********************************************************************************
 * npd_ndisc_snooping_del_static_by_l3index
 *
 *  	clear static ndisc info on this port
 *
 *	INPUT:
 *		vid   -- vlan ID
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ndisc item number cleared on this eth port.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_del_static_by_l3index
(
	unsigned int l3index
)
{
	unsigned int retVal = 0;
	struct ndisc_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct ndisc_snooping_item_s));
	comparator.l3Index = l3index;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal(npd_ndiscsnp_hashport_index,kern_del_flag,&comparator,\
									npd_ndisc_snooping_filter_by_l3index,npd_ndisc_snooping_del_static);
	
	syslog_ax_ndiscsnooping_dbg("npd clear static ndisc on l3index %#x total %d items deleted\n", \
					l3index,retVal);
	return retVal;
}



/**********************************************************************************
 * npd_ndisc_snooping_del_static_by_ifindex
 *
 *  	clear static ndisc info on this port
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ndisc item number cleared on this eth port.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_del_static_by_ifindex
(
	unsigned int   eth_g_index
)
{
	unsigned int retVal = 0;
	unsigned char devNum = 0, portNum = 0;
	struct ndisc_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct ndisc_snooping_item_s));
	comparator.ifIndex = eth_g_index;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal_key(npd_ndiscsnp_hashport_index,kern_del_flag,&comparator,\
									npd_ndisc_snooping_filter_by_ifindex,npd_ndisc_snooping_del_static);
	
	syslog_ax_ndiscsnooping_dbg("npd clear static ndisc on eth-port %#x dev %d port %d total %d items deleted\n", \
					eth_g_index,devNum,portNum, retVal);
	return retVal;
}


/**********************************************************************************
 * npd_ndisc_snooping_del_by_ifindex
 *
 *  	clear neighbour info learned on this port
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		neighbour item number cleared on this eth port.
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_del_by_ifindex
(
	unsigned int   eth_g_index
)
{
	unsigned int retVal = 0;
	unsigned char devNum = 0, portNum = 0;
	struct ndisc_snooping_item_s comparator;
	unsigned int kern_del_flag = TRUE;

	memset(&comparator, 0,sizeof(struct ndisc_snooping_item_s));
	comparator.ifIndex = eth_g_index;

	kern_del_flag = TRUE;
	retVal = dbtable_hash_traversal_key(npd_ndiscsnp_hashport_index,kern_del_flag,&comparator,\
									npd_ndisc_snooping_filter_by_ifindex,npd_ndisc_snooping_del_dynamic);
	
	syslog_ax_ndiscsnooping_dbg("npd clear ndisc on eth-port %#x dev %d port %d total %d items deleted\n", \
					eth_g_index,devNum,portNum, retVal);
	return retVal;
}


/********************************************************
 * npd_ndisc_snooping_static_valid_set_by_ifindex
 *
 *  	set the static ndisc items on this port to valid or invalid
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *		isVaild  - set valid or invalid
 *				0 - set invalid
 *				1 - set valid	
 *	OUTPUT:
 *		NULL
 * 	RETURN:
 *		ndisc item number set on this eth port.
 *		
 *	NOTE:
 *
 *********************************************************/
unsigned int npd_ndisc_snooping_static_valid_set_by_ifindex
(
	unsigned int   ifIndex,
	unsigned int isValid
)
{
	unsigned int retVal = 0;
	struct ndisc_snooping_item_s comparator;

	memset(&comparator, 0,sizeof(struct ndisc_snooping_item_s));	
	comparator.ifIndex = ifIndex;
	
	retVal = dbtable_hash_traversal_key(npd_ndiscsnp_hashport_index, isValid,&comparator,\
					npd_ndisc_snooping_filter_by_ifindex,npd_ndisc_snooping_static_valid_set);

	if(retVal > 0){
	    syslog_ax_ndiscsnooping_dbg("npd static ndisc set %s on eth-port %#x total %d items set\n", \
					    isValid ? "Valid":"Invalid",ifIndex,retVal);
	}
	return retVal;
}

/**********************************************************************************
 * npd_ndisc_snooping_create_static_neighbour
 *	This routine is used to create static neighbour entry.
 *
 *	INPUT:
 *		ipAddr		--	ip address
 *		mac 		--	MAC address
 *		eth-g_index  -- port index
 *	
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS - if no error occurred
 *		NDISC_RETURN_CODE_STATIC_NEIGH_FULL  - if the static neighbour items are equal to 1024 or more
 *		NDISC_RETURN_CODE_NULL_PTR - if input parameters have null pointer
 *		NDISC_RETURN_CODE_TABLE_FULL	- the hash table is full
 *		NDISC_RETURN_CODE_STATIC_EXIST - static neighbour item already exists
 *		NDISC_RETURN_CODE_NO_RESOURCE - if no memory allocatable
 *		NDISC_RETURN_CODE_ERROR  - if other error occurs,get dev port failed or nexthop op failed
 *		
 *	NOTE:
 *
 **********************************************************************************/
 int npd_ndisc_snooping_create_static_neighbour
(
	ip6_addr *ipAddr,
	unsigned char* mac,
	unsigned int eth_g_index
)
{
	int ret = NDISC_RETURN_CODE_SUCCESS;
	struct ndisc_snooping_item_s ndiscItem,ndiscInfo;
	unsigned int ndiscCount = 0;
	unsigned int L3ifindex = 0;
	unsigned short vid = 0;
	int status;

	memset(&ndiscItem,0,sizeof(struct ndisc_snooping_item_s));
	ndiscCount = npd_ndisc_snooping_count_all();
	if( NPD_NDISCSNP_TABLE_SIZE <= ndiscCount){
		npd_ndisc_snooping_drop_handle();
		ndiscCount = npd_ndisc_snooping_count_all();
		if( NPD_NDISCSNP_TABLE_SIZE <= ndiscCount)
		{
			syslog_ax_ndiscsnooping_err("get static neighbour count %d \n",ndiscCount);
			return NDISC_RETURN_CODE_STATIC_NEIGHBOUR_FULL;
		}
	}
	
	status = npd_ndisc_snooping_valid_check(*ipAddr, eth_g_index, &L3ifindex, &vid);	
	syslog_ax_ndiscsnooping_dbg("static ip info ifindex %d,ipAddr "IPV6STR"\r\n",L3ifindex, IPV6_2_STR(*ipAddr));

	ndiscItem.l3Index = L3ifindex;
	memcpy(&ndiscItem.ipAddr, ipAddr, sizeof(ndiscItem.ipAddr));
	ndiscItem.isStatic = TRUE;
	ndiscItem.isValid = status;
	ndiscItem.vid = vid;
	ndiscItem.ifIndex = eth_g_index;
	memcpy(ndiscItem.mac,mac,MAC_ADDRESS_LEN);
	status = npd_ndisc_snooping_find_item_byip(*ipAddr, &ndiscInfo);
	if((0 != status) || (FALSE == ndiscInfo.isStatic)) 
	{
		if( 0 == status )
		{
			status = dbtable_hash_delete(npd_ndiscsnp_haship_index, &ndiscInfo, &ndiscInfo);
		}
		
		if((NDISC_RETURN_CODE_SUCCESS != npd_ndisc_snooping_create_kern_neighbour(&ndiscItem))){/*kernal success */
			ndiscItem.isValid = FALSE;
		}		
		syslog_ax_ndiscsnooping_dbg("static neighbour insert: ip "IPV6STR", l3index 0x%x, ifindex 0x%x vlan %d\r\n", \
			IPV6_2_STR(ndiscItem.ipAddr), ndiscItem.l3Index, ndiscItem.ifIndex, ndiscItem.vid);
		status = dbtable_hash_insert( npd_ndiscsnp_haship_index, &ndiscItem);
		
		npd_route_v6_update_by_nhp(*ipAddr, ndiscItem.isValid);
		
		if( status == 0 ) {
			ret = NDISC_RETURN_CODE_SUCCESS;					
		}		
	}
	else{
		ret = NDISC_RETURN_CODE_STATIC_EXIST;
	}
	return ret;
}



/*************************************************************
 *
 * OUTPUT:
 *		showStr : String - the result of static arp running-config 
 *					e.g. "ip static-arp 1/1 00:00:00:00:00:01 192.168.0.2/32 1\n"
 *
 *************************************************************/

#define FILTER_NEIGH_ALL 			0
#define FILTER_NEIGH_BY_IP_ADDR 	(0x1) 
#define FILTER_NEIGH_BY_MAC_ADDR	(0x1<<1)
#define FILTER_NEIGH_BY_SUBNET		(0x1<<2)
#define FILTER_NEIGH_BY_NETIF 		(0x1<<3)
#define NEIGH_NEED_GET_COUNT		(0x1<<6)
#define FILTER_NEIGH_NONE 			(0x1<<7)
/**********************************************************************************
 * npd_ndisc_snooping_del
 *
 *  	Delete neighbour snooping info in both sw and hw
 *
 *	INPUT:
 *		item - neighbour snooping item info
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS  - if success
 *		NDISC_RETURN_CODE_NULL_PTR  - if the item is null
 *		NDISC_RETURN_CODE_NAM_ERROR  - nam operation failed
 *		NDISC_RETURN_CODE_HASH_OP_FAILED  - hash table operation failed
 *		NDISC_RETURN_CODE_NOTEXISTS  -  the neighbour not a dynamic one
 *		
 *	NOTE:
 *		this procedure treat neighbour snooping items found in hash tables, so the 'item' is already found
 *		when calling the procedure, no hash table search done here.
 *
 **********************************************************************************/
int npd_ndisc_snooping_del
(
    hash_table_index_t *hash,
	void *data,
	unsigned int kern_del_flag
)
{
	int ret;
	struct ndisc_snooping_item_s *item = (struct ndisc_snooping_item_s *)data;

	if( item == NULL )
		return NDISC_RETURN_CODE_ERROR;
		
	if(TRUE == item->isStatic) {
		return NDISC_RETURN_CODE_NOTEXISTS;
	}
	
    ret = npd_ndisc_snooping_del_all(hash, item,kern_del_flag);

	return ret; 
}


/****************************************************************
 *npd_ndisc_snooping_del_by_network
 *		delete neighbour items by ipv6 address and mask len 
 * INPUT:
 *		ipv6_addr - the ipv6 address
 *		ipv6_masklen - the mask len
 *
 * OUTPUT:
 *		NONE
 * RETURN:
 *		NDISC_RETURN_CODE_SUCCESS - 
 *
 ****************************************************************/
unsigned int npd_ndisc_snooping_del_by_network(ip6_addr *ipv6_addr,unsigned int ipv6_masklen)
{			
	unsigned int delCount = 0;
	prefix_ipv6_stc prefix_ipv6;

	
	memcpy(&prefix_ipv6.prefix, ipv6_addr, sizeof(prefix_ipv6.prefix)); 
	prefix_ipv6.prefixlen = ipv6_masklen;
	delCount = dbtable_hash_traversal(npd_ndiscsnp_haship_index,NPD_TRUE,(void*)&prefix_ipv6,\
									npd_ndisc_snooping_filter_by_network,npd_ndisc_snooping_del);
	if(delCount > 0){
		syslog_ax_ndiscsnooping_dbg("ndisc delete by ipv6 and mask len,delete %d items \n",delCount);
	}
	
	return NDISC_RETURN_CODE_SUCCESS;
}


/****************************************************************
 *npd_ndisc_snooping_del_static_by_network
 *		delete neighbour items by ip and mask 
 * INPUT:
 *		ipv6_addr - the ipv6 address
 *		ipv6_masklen - the mask len
 *
 * OUTPUT:
 *		NONE
 * RETURN:
 *		NDISC_RETURN_CODE_SUCCESS - 
 *
 ****************************************************************/
unsigned int npd_ndisc_snooping_del_static_by_network(ip6_addr *ipv6_addr,unsigned int ipv6_masklen)
{			
	unsigned int delCount = 0;
	prefix_ipv6_stc prefix_ipv6;

	memcpy(&prefix_ipv6.prefix, ipv6_addr, sizeof(prefix_ipv6.prefix));
	prefix_ipv6.prefixlen = ipv6_masklen;
	delCount = dbtable_hash_traversal(npd_ndiscsnp_haship_index,NPD_TRUE,&prefix_ipv6,\
									npd_ndisc_snooping_filter_by_network,npd_ndisc_snooping_del_static);
	if(delCount > 0){
		syslog_ax_ndiscsnooping_dbg("ndisc delete by ip and mask,delete %d items \n",delCount);
	}
	
	return NDISC_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_ndisc_snooping_check_ipv6_address
 *
 *  DESCRIPTION:
 *          this routine check the layer 3 interface's ipv6 address
 *  INPUT:
 *          ipv6Address - the ip address which we want to check
 *          vid          - the vid we want to check in
 *  OUTPUT:
 *          NULL
 *  RETURN:
 *          NDISC_RETURN_CODE_NO_HAVE_THE_IP - l3intf no have the ip but have the same sub net ip with ipAddress
 *          NDISC_RETURN_CODE_NO_HAVE_ANY_IP - l3intf no have any ip address
 *          NDISC_RETURN_CODE_HAVE_THE_IP       - l3intf already have the ip address
 *          NDISC_RETURN_CODE_NOT_SAME_SUB_NET - l3intf no have the same sub net ip with ipAddress
 *          NDISC_RETURN_CODE_CHECK_IP_ERROR - check ip address error or no have l3intf
 *
 ***********************************************************************************/
int npd_ndisc_snooping_check_ipv6_address
(
    ip6_addr *ipAddress,
    unsigned short vid,
    unsigned int eth_g_index
)
{
	unsigned int ifindex = ~0UI, l3Index = ~0UI;
	int ret = NDISC_RETURN_CODE_CHECK_IP_ERROR;
	ip6_addr ipAddrs[MAX_IP_COUNT];
	ip6_addr masks[MAX_IP_COUNT];
	int i = 0;
	unsigned int haveIp = 0,haveTheIp = 0,haveSameSubnet = 0;
	unsigned int errorip = 0;
	memset(ipAddrs, 0xff, sizeof(ipAddrs));
	memset(masks, 0, sizeof(masks));

	
	if(TRUE != npd_intf_port_check(vid,eth_g_index,&ifindex)){
		return NDISC_RETURN_CODE_CHECK_IP_ERROR;
	}
	if(TRUE != npd_intf_get_global_l3index(ifindex, &l3Index)){
		return NDISC_RETURN_CODE_CHECK_IP_ERROR;
	}

	syslog_ax_ndiscsnooping_dbg("npd_intf_check_ip_address:: check for ip "IPV6STR", vid %d, ifIndex 0x%x\n", IPV6_2_STR(*ipAddress), vid, eth_g_index );
	
	if(l3Index != ~0UL){
		ret = npd_ipv6_intf_addr_ip_get(l3Index,ipAddrs,masks);
		if(NPD_TRUE != ret){
	        return NDISC_RETURN_CODE_CHECK_IP_ERROR;
		}
		for(i=0;i<MAX_IP_COUNT;i++){
			if(ipv6_addr_is_valid(&ipAddrs[i])){
			    haveIp = 1;
				if(!memcmp(&ipAddrs[i], ipAddress, sizeof(ipAddrs[i]))){
				 	haveTheIp = 1;
				    break;
				}
 				if((ipv6_addr_type(ipAddress) != IPV6_ADDR_UNICAST) && //FIXME jianchao 2012-02-27
 				 	(ipv6_addr_type(ipAddress) != IPV6_ADDR_MAPPED)&& //FIXME jianchao 2012-02-27
 				 	(ipv6_addr_type(ipAddress) != IPV6_ADDR_COMPATv4))
 				{
 					errorip = 1;
 					syslog_ax_ndiscsnooping_dbg("ipv6 dip check faile!\n");
					break;
 				}
				//if((ipAddrs[i] & masks[i]) == (ipAddress & masks[i]))
				 if(IPV6_NET_EQUAL(ipAddrs[i], *ipAddress, masks[i], masks[i])){
	                 haveSameSubnet = 1;
					 break;
				 }
			}
		}
		if(haveIp == 0){
			syslog_ax_ndiscsnooping_err("npd_intf_check_ip_address:: l3intf don't have any ip address!\n");
			ret = NDISC_RETURN_CODE_NO_HAVE_ANY_IP;
		}else if(haveTheIp == 1){
	        syslog_ax_ndiscsnooping_err("npd_intf_check_ip_address:: already have the ip address!\n");
			ret = NDISC_RETURN_CODE_HAVE_THE_IP;
		}else if(haveSameSubnet == 1){
	        syslog_ax_ndiscsnooping_err("npd_intf_check_ip_address:: l3intf don't have the ip and have the same sub net ip!\n");	
			ret = NDISC_RETURN_CODE_NO_HAVE_THE_IP;
		}else if(errorip == 1){
		    syslog_ax_ndiscsnooping_err("npd_intf_check_ip_address:: The ip is illegal ip!\n");	
			ret = NDISC_RETURN_CODE_CHECK_IP_ERROR;
		}else{
	        syslog_ax_ndiscsnooping_dbg("npd_intf_check_ip_address:: don't have same subnet ip address!\n");
			syslog_ax_ndiscsnooping_dbg("\t the ipAddress is "IPV6STR"\n", IPV6_2_STR(*ipAddress));
			syslog_ax_ndiscsnooping_dbg("\t the intf ips are \n");
			for (i=0;i<MAX_IP_COUNT;i++){
				if(ipv6_addr_is_valid(&ipAddrs[i])){
		            syslog_ax_ndiscsnooping_dbg("\t ip "IPV6STR" mask "IPV6STR"\n",\
						IPV6_2_STR(ipAddrs[i]), IPV6_2_STR(masks[i]));
				}
			}
			
			ret = NDISC_RETURN_CODE_NOT_SAME_SUB_NET;
		}
	}
    return ret;
}

int npd_ndisc_snooping_valid_check(
	ip6_addr ipv6_addr, 
	unsigned int netif_index, 
	unsigned int *gl3index,
	unsigned short *vlanId
)
{
	int ret;
	unsigned int l3_localindex = 0;
	unsigned int l3intf_status, netif_status;
	
	ret = npd_ipv6_intf_vid_get_by_ip(&ipv6_addr,netif_index,vlanId);
	if(NDISC_RETURN_CODE_SUCCESS == ret)
	{		
		if(TRUE == npd_intf_port_check(*vlanId,netif_index,&l3_localindex)) 
		{				
			if(NDISC_RETURN_CODE_NO_HAVE_THE_IP == npd_ndisc_snooping_check_ipv6_address(&ipv6_addr,*vlanId,netif_index))
			{
				npd_intf_get_global_l3index(l3_localindex, gl3index);
				l3intf_status = npd_intf_get_l3intf_status_by_ifindex(*gl3index);
				npd_check_netif_status(netif_index, &netif_status);
				if(netif_status && l3intf_status)
					return NPD_TRUE;
		    }
		}
	}

	return NPD_FALSE;
}

int npd_ndisc_snooping_age(hash_table_index_t *hash, void * data, unsigned int flag)
{
	struct ndisc_snooping_item_s *ndisc_item;

	if( data == NULL )
		return NDISC_RETURN_CODE_ERROR;

	ndisc_item = (struct ndisc_snooping_item_s *)data;

	if(TRUE == ndisc_item->isStatic)
		return NDISC_RETURN_CODE_SUCCESS;

	if( (flag & NDISCSNP_FLAG_DROP) && (ndisc_item->flag & NDISCSNP_FLAG_DROP))
	{
		npd_ndisc_snooping_del_dynamic(hash, ndisc_item, TRUE);
		return NDISC_RETURN_CODE_SUCCESS;
	}

	if( flag & NDISCSNP_FLAG_HIT )
	{
		if(!(ndisc_item->flag & NDISCSNP_FLAG_HIT))
		{
			npd_ndisc_snooping_del_dynamic(hash, ndisc_item, TRUE);
			return NDISC_RETURN_CODE_SUCCESS;
		}
		else if(ndisc_item->flag & NDISCSNP_FLAG_HIT)
		{
			ndisc_item->flag &= ~NDISCSNP_FLAG_HIT;
			dbtable_hash_update(hash, ndisc_item, ndisc_item);
			npd_ndisc_snooping_solicit_send(ndisc_item);
		}
	}
	return NDISC_RETURN_CODE_SUCCESS;;		
}



void npd_ndisc_snooping_drop_handle()
{
	dbtable_hash_traversal(npd_ndiscsnp_haship_index, NDISCSNP_FLAG_DROP, NULL, NULL, npd_ndisc_snooping_age);
}


void npd_ndisc_snooping_age_handle(struct ndisc_snooping_item_s *startEntry)
{
    unsigned int count = 0;
    int ret;
	unsigned char zero_mac[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	struct ndisc_snooping_item_s nextEntry;
	
	if (0 == memcmp(startEntry->mac, zero_mac, MAC_ADDR_LEN))
	{
    	ret = dbtable_hash_head(npd_ndiscsnp_haship_index, startEntry, startEntry, NULL);
	}
	else
	{
    	ret = dbtable_hash_head_key(npd_ndiscsnp_haship_index, startEntry, startEntry, NULL);
	}
    
    while(0 == ret)
    {
		if(startEntry->isStatic)
		{
			ret = dbtable_hash_next(npd_ndiscsnp_haship_index, startEntry, startEntry, NULL);
			continue;
		}
        count++;
        if(count > NPD_NDISC_ITEM_ONCE_HANDLE)
            break;
		ret = dbtable_hash_next(npd_ndiscsnp_haship_index, startEntry, &nextEntry, NULL);
        npd_ndisc_snooping_age(npd_ndiscsnp_haship_index, startEntry, NDISCSNP_FLAG_ALL);
		memcpy(startEntry, &nextEntry, sizeof(struct ndisc_snooping_item_s));        
    }
    if(0 != ret)
    {
        memset(startEntry, 0, sizeof(*startEntry));
		ndiscsnp_aging_continue = 0;
    }
	else
	{
		ndiscsnp_aging_continue = 1;
	}
}


/**********************************************************************************
 * npd_ndisc_snooping_count_all
 *
 *  	counte all neighbour num
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		count - the count of static neighbour 
 *		
 *	NOTE:
 *
 **********************************************************************************/
unsigned int npd_ndisc_snooping_count_all
(
    void
)
{
    unsigned int count = 0;
	count = dbtable_hash_count(npd_ndiscsnp_haship_index);
	return count;
}

unsigned short npd_iphdr_checksum(unsigned short *buffer, int size)  
{
	/* Compute Internet Checksum for "count" bytes
	*         beginning at location "addr".
	*/
	int sum = 0;
	unsigned short *source = (unsigned short *)buffer;

	while (size > 1)  
	{
		/*  This is the inner loop */
		sum += *source++;
		size -= 2;
	}

	/*  Add left-over byte, if any */
	if (size > 0) 
	{
		/* Make sure that the left-over byte is added correctly both
		* with little and big endian hosts */
		unsigned short tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return (unsigned short)~sum;
}    

int npd_ndisc_ipv6_checksum(struct ip6_hdr *iphdr)
{
	struct ipv6_checksum_pseudo ip6_csum_psedudo;
	char buffer[256];
	struct icmp6_hdr *icmp6hdr; 
	unsigned short csum = 0;

	if(iphdr == NULL)
		return -1;
	
	icmp6hdr = (struct icmp6_hdr *)(iphdr+1);

	memset(&ip6_csum_psedudo, 0, sizeof(struct ipv6_checksum_pseudo));
	memcpy(&ip6_csum_psedudo.saddr, &iphdr->ip6_src, sizeof(ip6_addr));
	memcpy(&ip6_csum_psedudo.daddr, &iphdr->ip6_dst, sizeof(ip6_addr));
	ip6_csum_psedudo.len = (unsigned int)iphdr->ip6_plen;
	ip6_csum_psedudo.nexthdr = (unsigned int)iphdr->ip6_nxt;
	
	memset(buffer,0,256);
	memcpy(buffer, icmp6hdr, iphdr->ip6_plen);
	memcpy(buffer+iphdr->ip6_plen, &ip6_csum_psedudo, sizeof(struct ipv6_checksum_pseudo));
	csum = npd_iphdr_checksum((unsigned short*)buffer, sizeof(struct ipv6_checksum_pseudo)+iphdr->ip6_plen);
	
	icmp6hdr->icmp6_cksum = csum;
	return 0;
}

int npd_ndisc_solicit_send(struct ndisc_snooping_item_s *item, ip6_addr *gateway)
{
	unsigned char sysMac[MAC_ADDR_LEN];
	unsigned char isTagged = 0;
	unsigned char	*data_buff = NULL;
	struct ether_header_t *layer2 = NULL;
	struct ip6_hdr 	*layer3 = NULL;
	struct icmp6_hdr *hdr = NULL;
	unsigned char *opt = NULL;
	int len = 0;
	
	syslog_ax_ndiscsnooping_dbg("Neighbor Solicit send to netif 0x%x\r\n", item->ifIndex);
	npd_vlan_check_contain_port(item->vid, item->ifIndex, &isTagged);
	npd_system_get_basemac(sysMac, MAC_ADDR_LEN);
	
	data_buff = npd_packet_alloc(NPD_NDISC_PACKET_NS_LEN);
	if(NULL == data_buff)
	{
		syslog_ax_ndiscsnooping_err("malloc dma err when send solicit arp\r\n");
		return COMMON_RETURN_CODE_NO_RESOURCE;
	}
	memset(data_buff,0,NPD_NDISC_PACKET_NS_LEN);
	len = sizeof(struct icmp6_hdr)+sizeof(ip6_addr)+2+ETH_ALEN;

	/* Build up neighbor solicit packet */	
	/* layer 2 */
	layer2 = (struct ether_header_t *)data_buff;
	memcpy(layer2->dmac,item->mac,ETH_ALEN);
	memcpy(layer2->smac,sysMac,ETH_ALEN);
	layer2->etherType = htons(0x86dd);

	/* layer 3 */
	layer3 = (struct ip6_hdr *)(layer2 + 1);
	*(unsigned int *)layer3 = htonl(0x60000000);
	layer3->ip6_plen = htons(len);
	layer3->ip6_nxt     = htons(IPPROTO_ICMPV6);
	layer3->ip6_hlim   = htons(255);
	memcpy(&layer3->ip6_src, gateway, sizeof(ip6_addr));
	memcpy(&layer3->ip6_dst, &item->ipAddr, sizeof(ip6_addr));
		
	/* icmp6 */
	hdr = (struct icmp6_hdr *)(layer3 + 1);
	hdr->icmp6_type     = htons(ND_NEIGHBOR_SOLICIT);
	hdr->icmp6_code     = 0;
	hdr->icmp6_cksum    = 0;
		
	/* opt , target IPv6 address*/
	opt = (unsigned char *)(hdr+1);
	memcpy(opt, &item->ipAddr, sizeof(ip6_addr));
	opt += sizeof(ip6_addr);

	/*opt, source linklayer address*/
	*opt++ = ND_OPT_SOURCE_LINKADDR;
	*opt++ = 1;
	memcpy(opt, sysMac, MAC_ADDR_LEN);
	
	/*ipv6 icmp checksum*/
	npd_ndisc_ipv6_checksum(layer3);

	if( TRUE == ndisc_aging_bcast )
	{
		nam_packet_tx_broadcast_global(21, item->vid, data_buff, NPD_NDISC_PACKET_NS_LEN);
	}
	else {
		nam_packet_tx_unicast_by_netif(21, item->ifIndex, item->vid,  isTagged, data_buff, NPD_NDISC_PACKET_NS_LEN);
	}
    npd_packet_free(data_buff);
	return ARP_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_ndisc_snooping_solicit_send
 *	This method is used to send neighbour solicit packet
 *
 *	INPUT:
 *		item - neighbour snooping SW hash table items.
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NDISC_RETURN_CODE_SUCCESS  - if no error occurred
 *		NDISC_RETURN_CODE_NULL_PTR - if item is null
 *		NDISC_RETURN_CODE_ERROR - if other error occurs
 *		
 *	NOTE:
 *
 **********************************************************************************/
int npd_ndisc_snooping_solicit_send
(
	struct ndisc_snooping_item_s *item
)
{

	int result = NDISC_RETURN_CODE_SUCCESS;
	ip6_addr gateway[MAX_IP_COUNT];
	ip6_addr mask[MAX_IP_COUNT];
	int i = 0;

	if(NULL == item || FALSE == item->isValid) {
		return NDISC_RETURN_CODE_NULL_PTR;
	}
	
	memset(gateway, 0, MAX_IP_COUNT*sizeof(ip6_addr));
	memset(mask, 0, MAX_IP_COUNT*sizeof(ip6_addr));

	result = npd_ipv6_intf_addr_ip_get(item->l3Index,gateway,mask);
	if(NPD_TRUE != result) {
		syslog_ax_ndiscsnooping_err("get gateway ip error when send neighour solicit,ret %#0x \n",result);
		return NDISC_RETURN_CODE_ERROR;
	}

	for(i=0;i<MAX_IP_COUNT;i++){
		if(!IPV6_ADDR_ZERO(gateway[i]) && IPV6_NET_EQUAL(item->ipAddr, gateway[i], mask[i], mask[i]))
		{
		    syslog_ax_ndiscsnooping_dbg("send neighbour solicit for %02x:%02x:%02x:%02x:%02x:%02x "IPV6STR" \n", \
				    item->mac[0],item->mac[1],item->mac[2],item->mac[3],item->mac[4],item->mac[5],	\
				    IPV6_2_STR(gateway[i]));
		    result = npd_ndisc_solicit_send(item,&gateway[i]);
			syslog_ax_ndiscsnooping_dbg("nam neighour solicit send %s ret %#0x \n",\
				(result == NDISC_RETURN_CODE_SUCCESS)?"SUCCESS":"FAILED",result);
			break;
		}
	}	

	return NDISC_RETURN_CODE_SUCCESS;
}


int npd_neighbour_snooping_sync(void)
{
	int dropCnt = 0;
	struct timeval tv;
	unsigned int curtime = 0;
	struct npd_ndiscsnp_cfg_s cfgItem;
    struct ndisc_snooping_item_s ndisc_item = {0};
    
	npd_init_tell_whoami("npdneighbourSync",0);
	
	while(1)
	{
		tv.tv_sec = NPD_NDISC_AGE_INTERVAL;
		tv.tv_usec = 0;		

		select(0, NULL, NULL, NULL, &tv);

		if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
			continue;
		if( dropCnt++ > NPD_NDISC_AGE_DROP_CNT)
		{
			dropCnt = 0;
#if 0
			npd_ndisc_snooping_drop_handle();
#endif
            if(ndiscsnp_aging_continue)
            {
				npd_ndisc_snooping_age_handle(&ndisc_item);
            }
		}
		dbtable_array_get(npd_ndiscsnp_cfg_index,0,&cfgItem);
		curtime++;
		if((curtime * NPD_NDISC_AGE_INTERVAL) > cfgItem.timeout)
		{
			curtime = 0;
			npd_ndisc_snooping_age_handle(&ndisc_item);
		}
	}
	
	return NDISC_RETURN_CODE_SUCCESS;
}


/********************************************************
 * npd_ndisc_snp_notify_event
 *
 *  	set the static neighbour  items on this port to valid or invalid
 *
 *	INPUT:
 *		eth_g_index - global eth-port index
 *		isVaild  - set valid or invalid
 *				0 - set invalid
 *				1 - set valid	
 *	OUTPUT:
 *		NULL
 * 	RETURN:
 *		neighbour item number set on this eth port.
 *		
 *	NOTE:
 *
 *********************************************************/
void npd_ndisc_snp_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
    )
{
    int type = npd_netif_type_get(netif_index);
	unsigned int l3index = 0;

	syslog_ax_ndiscsnooping_dbg("npd notify neighbour snp index event: index 0x%x event %d\n", netif_index, evt);

    switch(evt)
    {	    
		case PORT_NOTIFIER_STPTC:
		case PORT_NOTIFIER_DISCARD:
	    case PORT_NOTIFIER_LINKDOWN_E:		
			if( type == NPD_NETIF_ETH_TYPE ||
				type == NPD_NETIF_TRUNK_TYPE )
			{
				npd_ndisc_snooping_static_valid_set_by_ifindex( netif_index , FALSE);
				npd_ndisc_snooping_del_by_ifindex( netif_index );	
			}
			break;		
		case PORT_NOTIFIER_DELETE:
		case PORT_NOTIFIER_L2DELETE:						
			if( type == NPD_NETIF_ETH_TYPE ||
				type == NPD_NETIF_TRUNK_TYPE )
			{
				npd_ndisc_snooping_del_static_by_ifindex(netif_index);
				npd_ndisc_snooping_del_by_ifindex( netif_index );	
			}
			break;
		case PORT_NOTIFIER_LINKUP_E:
		case PORT_NOTIFIER_FORWARDING:
			if( type == NPD_NETIF_ETH_TYPE ||
				type == NPD_NETIF_TRUNK_TYPE )
			{
				npd_ndisc_snooping_static_valid_set_by_ifindex( netif_index , TRUE);
			}
	        break;
		case PORT_NOTIFIER_L3DELETE:
			if( NPD_TRUE == npd_intf_gindex_exist_check( netif_index, &l3index) )
			{
				npd_ndisc_snooping_static_valid_set_by_l3index(l3index, FALSE);
				npd_ndisc_snooping_del_by_l3index( l3index );	
			}
			break;
		case PORT_NOTIFIER_L3LINKDOWN:
			if( NPD_TRUE == npd_intf_gindex_exist_check( netif_index, &l3index) )
			{
				npd_ndisc_snooping_static_valid_set_by_l3index( l3index , FALSE);
				npd_ndisc_snooping_del_by_l3index( l3index );	
			}
			break;
		case PORT_NOTIFIER_L3LINKUP:
			if( NPD_TRUE == npd_intf_gindex_exist_check( netif_index, &l3index) )
			{
				npd_ndisc_snooping_static_valid_set_by_l3index(l3index, TRUE);
			}
			break;
	    default:
	        break;
    }

    return;
}


void npd_ndisc_snp_relate_event(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{
    int type = npd_netif_type_get(netif_index);
    unsigned long vlan_type = npd_netif_type_get(vlan_index);

	syslog_ax_ndiscsnooping_dbg("npd notify neighbour snp relate event: vlan 0x%x index 0x%x event %d\n", \
											vlan_index, netif_index, event);

	switch(event)
	{
		case PORT_NOTIFIER_LEAVE:			
	    	if(NPD_NETIF_VLAN_TYPE != vlan_type)
	    	    return;
		    if((type != NPD_NETIF_ETH_TYPE)&&(type != NPD_NETIF_TRUNK_TYPE))
		        return;			
			npd_ndisc_snooping_static_valid_set_by_ifindex(netif_index,FALSE);
			npd_ndisc_snooping_del_by_ifindex(netif_index);
			break;
		case PORT_NOTIFIER_JOIN:
			if(NPD_NETIF_VLAN_TYPE != vlan_type)
	    	    return;
		    if((type != NPD_NETIF_ETH_TYPE)&&(type != NPD_NETIF_TRUNK_TYPE))
		        return;			
			npd_ndisc_snooping_static_valid_set_by_ifindex(netif_index,TRUE);
		default:
			break;			
	}

	return;
}

DBusMessage *npd_dbus_clear_neigh(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	unsigned int eth_g_index = 0;
	unsigned int neigh_count = 0;
	ip6_addr ipAddr;
	unsigned int ipv6_masklen = 0;
	unsigned int flag = 0;
	unsigned int kern_del_flag = TRUE;
	unsigned int ret = NDISC_RETURN_CODE_SUCCESS;
	unsigned char mac[6] = {0};
	struct ndisc_snooping_item_s item;

	
	syslog_ax_ndiscsnooping_dbg("Entering clear ethport neigh!\n");

	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( msg, &err,
                            		DBUS_TYPE_UINT32,&eth_g_index,
                            	    DBUS_TYPE_UINT32,&ipAddr.u6_addr32[0],
                            	    DBUS_TYPE_UINT32,&ipAddr.u6_addr32[1],
                            	    DBUS_TYPE_UINT32,&ipAddr.u6_addr32[2],
                            	    DBUS_TYPE_UINT32,&ipAddr.u6_addr32[3],
                            	    DBUS_TYPE_UINT32,&ipv6_masklen,
									DBUS_TYPE_BYTE, &mac[0],
									DBUS_TYPE_BYTE, &mac[1],
									DBUS_TYPE_BYTE, &mac[2],
									DBUS_TYPE_BYTE, &mac[3],
									DBUS_TYPE_BYTE, &mac[4],
									DBUS_TYPE_BYTE, &mac[5],
                            	    DBUS_TYPE_UINT32,&flag,
                            		DBUS_TYPE_INVALID)))
	{
		syslog_ax_ndiscsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			syslog_ax_ndiscsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	if(FILTER_NEIGH_BY_IP_ADDR & flag)
	{
		ret = npd_ndisc_snooping_find_item_byip(ipAddr, &item);
		if(ret == 0)
		{
			ret = npd_ndisc_snooping_del(npd_ndiscsnp_haship_index, &item, TRUE);
			syslog_ax_ndiscsnooping_dbg("Neighbour delete by ip success ! \n");
		}
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	else if(FILTER_NEIGH_BY_MAC_ADDR &  flag)
	{
		memcpy(item.mac, mac, MAC_ADDR_LEN);

		neigh_count = dbtable_hash_traversal(npd_ndiscsnp_haship_index,NPD_TRUE,&item,\
										npd_ndisc_snooping_filter_by_mac,npd_ndisc_snooping_del);
		syslog_ax_ndiscsnooping_dbg("Neighbour delete by ip and mask,delete %d items \n",neigh_count);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
    else if(FILTER_NEIGH_BY_NETIF &  flag)
    {
    	neigh_count = npd_ndisc_snooping_del_by_ifindex(eth_g_index);
		syslog_ax_ndiscsnooping_dbg("Neighbour delete by ip and mask,delete %d items \n",neigh_count);
		ret = NDISC_RETURN_CODE_SUCCESS;
    }
	else if(FILTER_NEIGH_BY_SUBNET &  flag)
	{
		npd_ndisc_snooping_del_by_network(&ipAddr,ipv6_masklen);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	else if(FILTER_NEIGH_NONE & flag)
	{
    	neigh_count = dbtable_hash_traversal(npd_ndiscsnp_haship_index,kern_del_flag,NULL,NULL,npd_ndisc_snooping_del);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	
	return reply;


}

DBusMessage *npd_dbus_show_ip_neigh_info(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	unsigned char flag = 0;
	struct ndisc_snooping_item_s dbItem;
	unsigned int mask_len;
	unsigned int neigh_count = 0;
	unsigned int ret = NPD_DBUS_SUCCESS;
	
	syslog_ax_ndiscsnooping_dbg("Entering show ethport neigh!\n");

	memset(&dbItem,0, sizeof(struct ndisc_snooping_item_s));
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( message, &err,
				DBUS_TYPE_BYTE, &flag,
				DBUS_TYPE_UINT32,&dbItem.ifIndex,
				DBUS_TYPE_UINT32, &(dbItem.ipAddr.u6_addr32[0]),
				DBUS_TYPE_UINT32, &(dbItem.ipAddr.u6_addr32[1]),
				DBUS_TYPE_UINT32, &(dbItem.ipAddr.u6_addr32[2]),
				DBUS_TYPE_UINT32, &(dbItem.ipAddr.u6_addr32[3]),
				DBUS_TYPE_UINT32, &mask_len,
				DBUS_TYPE_BYTE, &dbItem.mac[0],
				DBUS_TYPE_BYTE, &dbItem.mac[1],
				DBUS_TYPE_BYTE, &dbItem.mac[2],
				DBUS_TYPE_BYTE, &dbItem.mac[3],
				DBUS_TYPE_BYTE, &dbItem.mac[4],
				DBUS_TYPE_BYTE, &dbItem.mac[5],
				DBUS_TYPE_INVALID)))
	{
		syslog_ax_ndiscsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_ndiscsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	if( FILTER_NEIGH_BY_IP_ADDR &flag )
	{
		if(NEIGH_NEED_GET_COUNT & flag)
		{
			neigh_count = dbtable_hash_traversal_key(npd_ndiscsnp_haship_index, TRUE, &dbItem, npd_ndisc_snooping_compare_byip, NULL);
			ret = dbtable_hash_head_key(npd_ndiscsnp_haship_index, &dbItem, &dbItem, npd_ndisc_snooping_compare_byip);
		}
		else
		{
			ret = dbtable_hash_next_key(npd_ndiscsnp_haship_index, &dbItem, &dbItem, npd_ndisc_snooping_compare_byip);
		}
	}
	else if( FILTER_NEIGH_BY_MAC_ADDR & flag )
	{
		if(NEIGH_NEED_GET_COUNT & flag)
		{
			neigh_count = dbtable_hash_traversal_key(npd_ndiscsnp_hashmac_index, TRUE, &dbItem, npd_ndisc_snooping_filter_by_mac, NULL);
			ret = dbtable_hash_head_key(npd_ndiscsnp_hashmac_index, &dbItem, &dbItem, npd_ndisc_snooping_filter_by_mac);
		}
		else
		{
			ret = dbtable_hash_next_key(npd_ndiscsnp_hashmac_index, &dbItem, &dbItem, npd_ndisc_snooping_filter_by_mac);
		}
	}
	else if( FILTER_NEIGH_BY_NETIF & flag )
	{
		if(NEIGH_NEED_GET_COUNT & flag)
		{
			neigh_count = dbtable_hash_traversal_key(npd_ndiscsnp_hashport_index, TRUE, &dbItem, npd_ndisc_snooping_filter_by_ifindex, NULL);
			ret = dbtable_hash_head_key(npd_ndiscsnp_hashport_index, &dbItem, &dbItem, npd_ndisc_snooping_filter_by_ifindex);
		}
		else
		{
			ret = dbtable_hash_next_key(npd_ndiscsnp_hashport_index, &dbItem, &dbItem, npd_ndisc_snooping_filter_by_ifindex);
		}
	}
	else if( FILTER_NEIGH_BY_SUBNET & flag )
	{
		prefix_ipv6_stc prefix_ipv6_addr;
		
		memcpy(&prefix_ipv6_addr.prefix, &dbItem.ipAddr, sizeof(prefix_ipv6_addr.prefix));
		prefix_ipv6_addr.prefixlen = mask_len;
		if(NEIGH_NEED_GET_COUNT & flag)
		{
			neigh_count = dbtable_hash_traversal(npd_ndiscsnp_haship_index, TRUE, &prefix_ipv6_addr, npd_ndisc_snooping_filter_by_network, NULL);
			ret = dbtable_hash_head(npd_ndiscsnp_haship_index, NULL, &dbItem, NULL);
			while(0 == ret )
			{
				if(TRUE == npd_ndisc_snooping_filter_by_network(&dbItem, &prefix_ipv6_addr))
					break;
				ret = dbtable_hash_next(npd_ndiscsnp_haship_index, &dbItem, &dbItem, NULL);
			}
		}
		else
		{
			ret = dbtable_hash_next(npd_ndiscsnp_haship_index, &dbItem, &dbItem, NULL);
			while(0 == ret )
			{
				if(TRUE == npd_ndisc_snooping_filter_by_network(&dbItem,&prefix_ipv6_addr))
					break;
				ret = dbtable_hash_next(npd_ndiscsnp_haship_index, &dbItem, &dbItem, NULL);
			}
		}
	}
	else if( FILTER_NEIGH_NONE & flag )
	{
		if(NEIGH_NEED_GET_COUNT & flag)
		{
			neigh_count = dbtable_hash_count(npd_ndiscsnp_haship_index);
			ret = dbtable_hash_head(npd_ndiscsnp_haship_index, NULL, &dbItem, NULL);
		}
		else
		{ 
			ret = dbtable_hash_next(npd_ndiscsnp_haship_index, &dbItem, &dbItem, NULL);
		}
	}

	
	dbus_message_iter_append_basic(&iter,
							DBUS_TYPE_UINT32,
									&ret);
	dbus_message_iter_append_basic (&iter,
							DBUS_TYPE_UINT32,
								 &neigh_count);
	if(NPD_DBUS_SUCCESS == ret)
	{
		int i = 0;
		for(i=0; i<4; i++){
			dbus_message_iter_append_basic (&iter,
								DBUS_TYPE_UINT32,
									 &(dbItem.ipAddr.u6_addr32[i]));
		}
			
		for (i = 0; i < 6; i++ ) {
			
			dbus_message_iter_append_basic(&iter,
							  		DBUS_TYPE_BYTE,
							  		&(dbItem.mac[i]));  /*mac*/
		}
			
		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
										&(dbItem.flag)); /*flag*/
			
		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_UINT32,
										&(dbItem.ifIndex)); /*ifIndex*/

		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_BYTE,
									  &(dbItem.isStatic)); /*isStatic*/
		dbus_message_iter_append_basic(&iter,
								DBUS_TYPE_BYTE,
									&(dbItem.isValid)); /*isStatic*/
	}
	
	return reply;
}


/********************************************************************
 *
 * OUTPUT:
 *		NDISC_RETURN_CODE_SUCCESS  - if success
 *		NDISC_RETURN_CODE_NULL_PTR  - if the item is null
 *		NDISC_RETURN_CODE_NO_SUCH_PORT
 *		NDISC_RETURN_CODE_VLAN_NOTEXISTS
 *		NDISC_RETURN_CODE_PORT_NOT_IN_VLAN
 *		NDISC_RETURN_CODE_INTERFACE_NOTEXIST
 *		NDISC_RETURN_CODE_UNSUPPORTED_COMMAND
 *		NDISC_RETURN_CODE_NAM_ERROR  - nam operation failed
 *		NDISC_RETURN_CODE_HASH_OP_FAILED  - hash table operation failed
 *		NDISC_RETURN_CODE_STASTIC_NOTEXIST - the static neighbour does not exist
 *		NDISC_RETURN_CODE_PORT_NOTMATCH  - the dev number and port number are not matched
 *		NDISC_RETURN_CODE_ERROR			 - if other error occurs,get dev and port failed
 *
 ********************************************************************/
DBusMessage * npd_dbus_no_ip_static_neigh(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;
	DBusMessageIter  iter;
	DBusError err;
	unsigned int eth_g_index = 0;
	unsigned int neigh_count = 0;
	ip6_addr ipv6_addr;
	unsigned int ipv6_masklen= 0;
	unsigned int flag = 0;
	unsigned int kern_del_flag = TRUE;
	unsigned int ret = NDISC_RETURN_CODE_SUCCESS;
	unsigned char mac[MAC_ADDRESS_LEN] = {0};
	struct ndisc_snooping_item_s item;
	
	syslog_ax_arpsnooping_dbg("Entering clear ethport neighbour!\n");

	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32,&eth_g_index,
								    DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[0],
								    DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[1],
								    DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[2],
								    DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[3],
								    DBUS_TYPE_UINT32,&ipv6_masklen,
									DBUS_TYPE_BYTE, &mac[0],
									DBUS_TYPE_BYTE, &mac[1],
									DBUS_TYPE_BYTE, &mac[2],
									DBUS_TYPE_BYTE, &mac[3],
									DBUS_TYPE_BYTE, &mac[4],
									DBUS_TYPE_BYTE, &mac[5],
								    DBUS_TYPE_UINT32,&flag,
									DBUS_TYPE_INVALID)))
	{
		syslog_ax_ndiscsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err))
		{
			syslog_ax_ndiscsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    if(FILTER_NEIGH_BY_IP_ADDR & flag)
    {
		ret = npd_ndisc_snooping_find_item_byip(ipv6_addr, &item);
		if(ret == 0)
		{
			ret = npd_ndisc_snooping_del_static(npd_ndiscsnp_haship_index, &item, TRUE);
			syslog_ax_ndiscsnooping_dbg("static neighbour delete by ip success ! \n");
		}
		ret = NDISC_RETURN_CODE_SUCCESS;
    }
	else if(FILTER_NEIGH_BY_MAC_ADDR & flag)
	{
		memcpy(item.mac, mac, MAC_ADDRESS_LEN);
		neigh_count = dbtable_hash_traversal(npd_ndiscsnp_haship_index,NPD_TRUE,&item,\
										npd_ndisc_snooping_filter_by_mac,npd_ndisc_snooping_del_static);
		syslog_ax_ndiscsnooping_dbg("neighbour delete by ip and mask,delete %d items \n",neigh_count);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	else if(FILTER_NEIGH_BY_NETIF & flag)
	{
    	neigh_count = npd_ndisc_snooping_del_static_by_ifindex(eth_g_index);
		syslog_ax_ndiscsnooping_dbg("neighbour delete by ip and mask,delete %d items \n",neigh_count);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	else if(FILTER_NEIGH_BY_SUBNET & flag)
	{
		npd_ndisc_snooping_del_static_by_network(&ipv6_addr,ipv6_masklen);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	else if(FILTER_NEIGH_NONE & flag)
	{
    	neigh_count = dbtable_hash_traversal(npd_ndiscsnp_haship_index,kern_del_flag,NULL,NULL,npd_ndisc_snooping_del_static);
		ret = NDISC_RETURN_CODE_SUCCESS;
	}
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	
	return reply;

}


/*********************************************************************************
 *
 * OUTPUT:
 *            NDISC_RETURN_CODE_SUCCESS - if no error occurred
 *            COMMON_RETURN_CODE_BADPARAM
 *            NDISC_RETURN_CODE_NULL_PTR - if input parameters have null pointer
 *            COMMON_RETURN_CODE_NO_RESOURCE - if no memory allocatable
 *            NDISC_RETURN_CODE_NO_SUCH_PORT
 *            NDISC_RETURN_CODE_VLAN_NOTEXISTS
 *            NDISC_RETURN_CODE_UNSUPPORTED_COMMAND
 *            NDISC_RETURN_CODE_PORT_NOT_IN_VLAN
 *            NDISC_RETURN_CODE_INTERFACE_NOTEXIST
 *            NDISC_RETURN_CODE_NO_HAVE_ANY_IP - l3intf no have any ip address
 *            NDISC_RETURN_CODE_HAVE_THE_IP       - l3intf already have the ip address
 *            NDISC_RETURN_CODE_NOT_SAME_SUB_NET - l3intf no have the same sub net ip with ipAddress
 *            NDISC_RETURN_CODE_CHECK_IP_ERROR - check ip address error or no have l3intf
 *	       NDISC_RETURN_CODE_MAC_MATCHED_BASE_MAC
 *            NDISC_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC
 *            NDISC_RETURN_CODE_STATIC_ARP_FULL  - if the static neighbour items are equal to 1024 or more
 *            NDISC_RETURN_CODE_TABLE_FULL  - the hash table is full
 *            NDISC_RETURN_CODE_STATIC_EXIST - static neighbour item already exists
 *            NDISC_RETURN_CODE_ERROR  - if other error occurs,get dev port failed or nexthop op failed
 *
 **********************************************************************************/
DBusMessage * npd_dbus_ipv6_static_neigh(DBusConnection *conn, DBusMessage *msg, void *user_data) {

	DBusMessage* reply;
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int ret = NDISC_RETURN_CODE_SUCCESS;
	unsigned int ipmaskLen = 0;
	unsigned short vlanId = 0;
	unsigned char macAddr[6]= {0};
	unsigned int eth_g_index = 0;
	unsigned char baseMac[MAC_ADDRESS_LEN]={0};
	ip6_addr ipv6_addr;


	memset(&ipv6_addr, 0 ,sizeof(ipv6_addr));
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
		         DBUS_TYPE_UINT32,&eth_g_index,
				 DBUS_TYPE_BYTE,&macAddr[0],
				 DBUS_TYPE_BYTE,&macAddr[1],
				 DBUS_TYPE_BYTE,&macAddr[2],
				 DBUS_TYPE_BYTE,&macAddr[3],
				 DBUS_TYPE_BYTE,&macAddr[4],
				 DBUS_TYPE_BYTE,&macAddr[5],
				 DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[0],
				 DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[1],
				 DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[2],
				 DBUS_TYPE_UINT32,&ipv6_addr.u6_addr32[3],
				 DBUS_TYPE_UINT32,&ipmaskLen,
				 DBUS_TYPE_INVALID))) {
		syslog_ax_ndiscsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_ndiscsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	syslog_ax_ndiscsnooping_dbg("npd_ndiscsnooping: ip static-neighbour 0x%x %02x:%02x:%02x:%02x:%02x:%02x ip "IPV6STR" ipmaskLen: %d vid: %d\n",
			                        eth_g_index,macAddr[0],macAddr[1],macAddr[2],
			                        macAddr[3],macAddr[4],macAddr[5],IPV6_2_STR(ipv6_addr), ipmaskLen,vlanId);
    syslog_ax_ndiscsnooping_dbg("%-15s:"IPV6STR"\n"," IP", IPV6_2_STR(ipv6_addr));	

	if(NPD_FALSE == ipv6_addr_valid_check((struct in6_addr*)&ipv6_addr))
	{
		ret = NDISC_RETURN_CODE_CHECK_IP_ERROR;
		goto reply;
	}
	
	ret = npd_system_get_basemac(baseMac, MAC_ADDRESS_LEN);
	if(0 == memcmp(macAddr,baseMac,MAC_ADDRESS_LEN))
	{	
		syslog_ax_ndiscsnooping_err("try to add static-neighbour with interface mac address FAILED!\n");
	    ret = NDISC_RETURN_CODE_MAC_MATCHED_INTERFACE_MAC;
		goto reply;
	}

    ret = npd_ndisc_snooping_create_static_neighbour(&ipv6_addr, macAddr, eth_g_index);
reply:	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);									 
	return reply;								 

}

DBusMessage *npd_dbus_ip_set_ndisc_agetime(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int flag = 0;
	unsigned int ret = NPD_DBUS_SUCCESS;
	unsigned int timeout = 0;
	unsigned int pretime = 0;
	struct npd_ndiscsnp_cfg_s cfgItem;
	DBusError err;
	
	dbus_error_init(&err);
	
	if (!(dbus_message_get_args ( message, &err,
				DBUS_TYPE_UINT32, &timeout,
				DBUS_TYPE_UINT32, &flag,
				DBUS_TYPE_INVALID)))
	{
		syslog_ax_ndiscsnooping_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			syslog_ax_ndiscsnooping_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	ret = dbtable_array_get(npd_ndiscsnp_cfg_index, 0, &cfgItem);
	if(0 == ret)
	{
		pretime = cfgItem.timeout;
		if(1 == flag){
			cfgItem.timeout = timeout;
		}
		else{
			cfgItem.timeout = NPD_NDISC_AGE_CNT;
		}
		ret = dbtable_array_update(npd_ndiscsnp_cfg_index, 0, &cfgItem, &cfgItem);
		if(0 != ret)
		{
			syslog_ax_ndiscsnooping_err("Update cfg failed!");
			ret = NPD_DBUS_ERROR;
		}
	}
	else{
		syslog_ax_ndiscsnooping_err("Get cfg failed!");
		ret = NPD_DBUS_ERROR;
	}
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	return reply;
}


/*************************************************************
 *
 * OUTPUT:
 *		showStr : 
 *
 *************************************************************/
DBusMessage *npd_dbus_ip_show_ndisc_agetime(DBusConnection *connection, DBusMessage *message, void *user_data)
{

	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int ret = NPD_DBUS_SUCCESS;
	unsigned int timeout = 0;
	struct npd_ndiscsnp_cfg_s cfgItem;
	DBusError err;
	
	dbus_error_init(&err);
	
	
	reply = dbus_message_new_method_return(message);
	
	dbus_message_iter_init_append (reply, &iter);

	ret = dbtable_array_get(npd_ndiscsnp_cfg_index, 0, &cfgItem);
	if(0 == ret)
	{
		timeout = cfgItem.timeout;
	}
	else{
		syslog_ax_arpsnooping_err("Get cfg failed!");
		ret = NPD_DBUS_ERROR;
	}
    dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32,
									&timeout);

	return reply;
}

/*************************************************************
 *
 * OUTPUT:
 *		showStr : String - the result of static ndisc running-config 
 *	       e.g. "ipv6 static-neigh port-channel 125 00:01:02:03:04:05 2001:0002:0003:0004:0005:0006:0007:0008\n"
 *
 *************************************************************/
DBusMessage *npd_dbus_static_ndisc_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter  iter;
	struct ndisc_snooping_item_s dbItem;
	struct npd_ndiscsnp_cfg_s cfgItem;
	int ret = 0;
	char *npd_ndisc_showStr = (char*)malloc(128 * NPD_NDISCSNP_TABLE_SIZE);
	int npd_ndisc_showStr_len = 0;
	char *curr = npd_ndisc_showStr;
	char ifName[33];
	char ipstring[64];
	
	memset(npd_ndisc_showStr,0,(128 * NPD_NDISCSNP_TABLE_SIZE));	
	memset(&dbItem, 0, sizeof(struct ndisc_snooping_item_s));
	dbItem.isStatic = 1;

	syslog_ax_arpsnooping_dbg("ndisc global show running-config\n");

	dbtable_array_get(npd_ndiscsnp_cfg_index,0,&cfgItem);
	if( NPD_NDISC_AGE_CNT != cfgItem.timeout)
	{
		npd_ndisc_showStr_len = sprintf(curr,"ipv6 ndp timeout %u\n", cfgItem.timeout);
		curr += npd_ndisc_showStr_len;
	}

	syslog_ax_arpsnooping_dbg("static ndisc show running-config\n");

	ret = dbtable_hash_head(npd_ndiscsnp_haship_index, &dbItem, &dbItem, npd_ndisc_snooping_filter_by_static);
	while(0 == ret)
	{
		if((npd_ndisc_showStr_len + 128) > (128 * NPD_NDISCSNP_TABLE_SIZE))
		{
			break;
		}

		if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(dbItem.ifIndex))
		{
			parse_eth_index_to_name(dbItem.ifIndex, ifName);
		}
		else
		{
			npd_netif_index_to_user_fullname(dbItem.ifIndex, ifName);
		}

		lib_get_string_from_ipv6(ipstring, &dbItem.ipAddr);
		npd_ndisc_showStr_len = sprintf(curr, "ipv6 static-neigh %s %02x:%02x:%02x:%02x:%02x:%02x %s\n",\
						ifName, dbItem.mac[0],dbItem.mac[1],dbItem.mac[2],dbItem.mac[3],\
						dbItem.mac[4],dbItem.mac[5], ipstring);
		curr += npd_ndisc_showStr_len;
		
		ret = dbtable_hash_next(npd_ndiscsnp_haship_index, &dbItem, &dbItem, npd_ndisc_snooping_filter_by_static);
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
                                     &npd_ndisc_showStr); 
								 
	free(npd_ndisc_showStr);
	npd_ndisc_showStr = NULL;
	npd_ndisc_showStr_len = 0;
	
	return reply;
}

/**********************************************************************************
 * npd_route_nexthop_key_generate
 *
 * Next-Hop brief info database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- Next-Hop brief info item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_ipv6_route_nexthop_key_generate
(
	void *data
)
{
	struct route_ipv6_nexthop_brief_s *item;
	unsigned int key = 0;


	item = (struct route_ipv6_nexthop_brief_s *)data;
	if(NULL == item) {
		syslog_ax_ndiscsnooping_err("npd neighbour snooping items make key null pointers error.\r\n");
		return ~0UI;
	}
	
	key= item->ipAddr.u6_addr32[0] ^ item->ipAddr.u6_addr32[1]\
			^ item->ipAddr.u6_addr32[2] ^ item->ipAddr.u6_addr32[3];
    key = jhash_1word(key, 0x35798642);
    key %= NPD_NDISCSNP_HASH_IP_SIZE;
	
	return key;
}
/**********************************************************************************
 * npd_route_nexthop_compare
 *
 * compare two of route nexthop brief info items
 *
 *	INPUT:
 *		itemA	- ipv6 route nexthop brief info item
 *		itemB	- ipv6 route nexthop brief info item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_ipv6_route_nexthop_compare
(
	void *data1,
	void *data2
)
{
	struct route_ipv6_nexthop_brief_s *itemA = (struct route_ipv6_nexthop_brief_s *)data1;
	struct route_ipv6_nexthop_brief_s *itemB = (struct route_ipv6_nexthop_brief_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_ndiscsnooping_err("npd neighbour snooping items compare null pointers error.\r\n");
		return NPD_FALSE;
	}
		
	if(0!= memcmp(&itemA->ipAddr, &itemB->ipAddr, sizeof(itemA->ipAddr))) {	/* ip address*/
		return NPD_FALSE;
	}
	
	return NPD_TRUE;
}


long npd_ndiscsnp_cfgtbl_handle_update( void *data1, void *data2)
{
	return NDISC_RETURN_CODE_SUCCESS;
}


int npd_ndisc_snooping_table_init()
{
	int ret;
	struct npd_ndiscsnp_cfg_s cfgItem;
	
	ret = create_dbtable( NPD_NDISCSNP_HASHTBL_NAME, NPD_NDISCSNP_TABLE_SIZE, sizeof(struct ndisc_snooping_item_s),\
					npd_ndiscsnp_dbtbl_handle_update, 
					npd_ndiscsnp_dbtbl_app_handle_update,
					npd_ndiscsnp_dbtbl_handle_insert, 
					npd_ndiscsnp_dbtbl_handle_delete,
					npd_ndiscsnp_dbtbl_app_handle_delete,
					NULL, 
					NULL, 
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_ndiscsnp_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd neighbour database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("ip", npd_ndiscsnp_dbtbl,NPD_NDISCSNP_HASH_IP_SIZE, npd_ndisc_snooping_key_generate,\
													npd_ndisc_snooping_compare, &npd_ndiscsnp_haship_index);
	if( 0  != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd neighbour ip hash table fail\n");
		return NPD_FAIL;
	}	

	ret = dbtable_create_hash_index("ifindex", npd_ndiscsnp_dbtbl, NPD_NDISCSNP_HASH_PORT_SIZE, npd_ndisc_snooping_key_port_generate,\
														npd_ndisc_snooping_compare, &npd_ndiscsnp_hashport_index);
	if( 0  != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd neighbour port hash table fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("mac", npd_ndiscsnp_dbtbl,NPD_NDISCSNP_HASH_MAC_SIZE, npd_ndisc_snooping_key_mac_generate,\
													npd_ndisc_snooping_compare, &npd_ndiscsnp_hashmac_index);
	if( 0  != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd neighbour ip hash table fail\n");
		return NPD_FAIL;
	}	

	ret = create_dbtable( NPD_NDISCSNP_HASHTBL_NAME, NPD_NDISCSNP_TABLE_SIZE, sizeof(struct route_ipv6_nexthop_brief_s),\
						NULL, NULL,	NULL, NULL,	NULL, NULL, NULL, NULL, NULL, DB_SYNC_NONE,&(npd_ipv6_nexthop_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd nexthop database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("ip", npd_ipv6_nexthop_dbtbl, NPD_NDISCSNP_HASH_IP_SIZE, npd_ipv6_route_nexthop_key_generate,\
													npd_ipv6_route_nexthop_compare, &npd_ipv6_nexthop_hash_index);
	if( 0  != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd nexthop hash table fail\n");
		return NPD_FAIL;
	}	

#if 1
	ret = create_dbtable( NPD_NDISCSNP_CFGTBL_NAME, 1, sizeof(struct npd_ndiscsnp_cfg_s),\
					npd_ndiscsnp_cfgtbl_handle_update, 
					NULL,
					NULL, 
					NULL,
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL,
					DB_SYNC_ALL,
					&(npd_ndiscsnp_cfgtbl));
	if( 0 != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd config database fail\n");
		return NPD_FAIL;
	}
	ret = dbtable_create_array_index("cfg",npd_ndiscsnp_cfgtbl,&npd_ndiscsnp_cfg_index);
	if( 0  != ret )
	{
		syslog_ax_ndiscsnooping_err("create npd config array index fail\n");
		return NPD_FAIL;
	}
		
	cfgItem.timeout = NPD_NDISC_AGE_AGE_CNT;

	ret = dbtable_array_insert_byid(npd_ndiscsnp_cfg_index, 0, &cfgItem);
	if( 0  != ret )
	{
		syslog_ax_ndiscsnooping_err("insert npd config array item fail\n");
		return NPD_FAIL;
	}
#endif

	return NPD_OK;
}

/**********************************************************************************
 * npd_init_neighbour_snooping
 *
 * neighbour snooping database(Hash table) initialization
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *
 **********************************************************************************/
void npd_init_neighbour_snooping
(
	void
)
{
	syslog_ax_ndiscsnooping_dbg("init neighbour snooping database\r\n");
	
	/* TODO: here we temporarily use 2K hash buckets to hold neighbour snooping items*/
	npd_ndisc_snooping_table_init();
	
	/*create socket deleted kernel arp */
	sysKernNeighbourSock = socket(AF_INET,SOCK_DGRAM,0);
	
	//nam_thread_create("npdArpSync",(void *)npd_arp_snooping_sync2kern,NULL,TRUE,FALSE);
	nam_thread_create("npd_neighbour_Sync",(void *)npd_neighbour_snooping_sync,NULL,TRUE,FALSE);
	
	register_netif_notifier(&ndisc_snp_netif_notifier);
	
	syslog_ax_ndiscsnooping_dbg("finish init arp snooping database\r\n");
	
	return;	
}
#endif

#ifdef __cplusplus
}
#endif


