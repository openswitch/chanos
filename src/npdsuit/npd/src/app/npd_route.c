/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_route.c
*
*
* CREATOR:
*		lizheng@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for L3 routing process.
*
* DATE:
*		03/20/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.66 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_route.h"


/* Utility function for parse rtattr. */
static void
netlink_parse_rtattr (struct rtattr **tb, int max, struct rtattr *rta,
					  int len)
{
	while (RTA_OK (rta, len))
	{
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;

		rta = RTA_NEXT (rta, len);
	}
}


hash_table_index_t *npd_mroute_haship_index = NULL;
hash_table_index_t *npd_mroute_hwid_hash_index  = NULL;
db_table_t         *npd_mroute_dbtbl = NULL;
db_table_t         *npd_mroute_hwid_dbtbl = NULL;


#ifdef HAVE_ROUTE
hash_table_index_t *npd_route_haship_index = NULL;
hash_table_index_t *npd_route_hashnhp_index = NULL;
array_table_index_t *npd_route_seqcfg_index = NULL;

db_table_t         *npd_route_dbtbl = NULL;
db_table_t         *npd_route_cfgtbl = NULL;

#ifdef HAVE_NPD_IPV6
hash_table_index_t *npd_route_haship6_index = NULL;
hash_table_index_t *npd_route_hashnhp6_index = NULL;
hash_table_index_t *npd_route_hashifi6_index = NULL;
db_table_t         *npd_route_v6_dbtbl = NULL;
db_table_t         *npd_route_v6_cfgtbl = NULL;
#endif //HAVE_NPD_IPV6


netif_event_notifier_t route_netif_notifier =
{
    .netif_event_handle_f = &npd_route_notify_event,
    .netif_relate_handle_f = &npd_route_relate_event
};

pthread_mutex_t g_npd_mroute_lock = PTHREAD_MUTEX_INITIALIZER;

int npd_route_update(	hash_table_index_t *hash, void *data, unsigned int flag);
int npd_route_delete( hash_table_index_t *hash, void *data,  unsigned int flag);
unsigned int npd_mroute_cmp_by_dstl3_ifindex(void * data1, void * data2);
int npd_route_db_op_item
(
	enum NPD_ROUTE_ACTION action,
	void *data
);

int npd_mroute_delete(hash_table_index_t * hash, void * data, unsigned int flag);
void npd_mroute_lock()
{
    pthread_mutex_lock(&g_npd_mroute_lock);
}

void npd_mroute_unlock()
{
    pthread_mutex_unlock(&g_npd_mroute_lock);
}

unsigned char get_route_info( struct rttbl_info *pRT_info,struct rtmsg * pRTmsgData,int payloadlen)
{
	struct rtattr * rtattrptr = NULL, *subrtattr = NULL;
	struct rtnexthop *rtnh = NULL;
	unsigned int nh_count = 0;
	unsigned char nh_exist = 0;
	struct rtattr *tb[__RTA_MAX + 1];
	
	if(!pRTmsgData || !pRT_info)
	{
		 syslog_ax_route_err("paraments is NULL\n");
		return -1;
	}
/* 
	check the route table 
*/
	syslog_ax_route_dbg("Debug:pRTmsgData->rtm_table %d\n",pRTmsgData->rtm_table);
	syslog_ax_route_dbg("Debug:pRTmsgData: FAMILY %d, tos %d, proto %d, scope %d, type %d, flags 0x%x\n", 
									pRTmsgData->rtm_family, pRTmsgData->rtm_tos,
									pRTmsgData->rtm_protocol, pRTmsgData->rtm_scope,
									pRTmsgData->rtm_type, pRTmsgData->rtm_flags);

	if(((AF_INET != pRTmsgData->rtm_family) && (AF_INET6 != pRTmsgData->rtm_family)) || (RT_TABLE_LOCAL == pRTmsgData->rtm_table))
	{
	    syslog_ax_route_dbg("");
		return -1;
	}
	pRT_info->rtm_family = pRTmsgData->rtm_family;
	pRT_info->rtm_flag = pRTmsgData->rtm_flags;
	
	if(RTN_BLACKHOLE == pRTmsgData->rtm_type){
		pRT_info->rt_type = RTN_BLACKHOLE;
	}
	else if(RTN_UNREACHABLE == pRTmsgData->rtm_type){
		pRT_info->rt_type = RTN_UNREACHABLE;
	}
	else if(RTN_MULTICAST == pRTmsgData->rtm_type)
	{
        pRT_info->rt_type = RTN_MULTICAST;
	}
    else{       
		pRT_info->rt_type = pRTmsgData->rtm_type;
	}
	
	rtattrptr = (struct rtattr *) RTM_RTA(pRTmsgData);
	
	memset (tb, 0, sizeof tb);
	netlink_parse_rtattr (tb, __RTA_MAX, rtattrptr, payloadlen);

	if (pRT_info->rtm_family == AF_INET)
	{
		if(tb[RTA_DST])	{
			pRT_info->DIP = *(unsigned int*)(RTA_DATA(tb[RTA_DST]));
		}
		if(tb[RTA_GATEWAY])	{
			nh_exist = 1;
			pRT_info->nexthop[nh_count] = *(unsigned int*)(RTA_DATA(tb[RTA_GATEWAY]));
		}
		if(tb[RTA_OIF]) {
			nh_exist = 1;
			pRT_info->ifindex[nh_count] = *(unsigned int*)(RTA_DATA(tb[RTA_OIF]));
		}
        if(tb[RTA_IIF]) {
            pRT_info->sifindex = *(unsigned int*)(RTA_DATA(tb[RTA_IIF]));
        }
        if (tb[RTA_SRC]) {
            pRT_info->SIP = *(unsigned int *)(RTA_DATA(tb[RTA_SRC]));
        }
		if (tb[RTA_PREFSRC]) {
			pRT_info->sgateway = *(unsigned int *)(RTA_DATA(tb[RTA_PREFSRC]));
		}
		if(tb[RTA_MULTIPATH]) {
			rtnh = (struct rtnexthop *)RTA_DATA(tb[RTA_MULTIPATH]); 
			
			/* rtnh can't beyond the rt payload length */
			for(;RTNH_OK(rtnh, rtnh->rtnh_len) && (rtnh < (struct rtnexthop * )(((char *)rtattrptr)+payloadlen));
                            rtnh=RTNH_NEXT(rtnh))
			{   
				nh_exist++;
		        pRT_info->ifindex[nh_count] = rtnh->rtnh_ifindex;
				pRT_info->nexthop_flag[nh_count] = rtnh->rtnh_flags;
                pRT_info->multipath = 1;
				subrtattr = (struct rtattr * )RTNH_DATA(rtnh);
                if(subrtattr == (struct rtattr * )RTNH_NEXT(rtnh)) {     
                    nh_count++;
                    continue;
                }
                                			
				switch(subrtattr->rta_type)
				{
					case RTA_GATEWAY:
						pRT_info->nexthop[nh_count] = *(unsigned int *)RTA_DATA(subrtattr);						
						break;
					default:
						break;
				}
                nh_count++;
			}
		}		
	}
#ifdef HAVE_NPD_IPV6	
	else if (pRT_info->rtm_family == AF_INET6)
	{
		if(tb[RTA_DST])	{
			memcpy((char*)&pRT_info->DIP6, (char*)(RTA_DATA(tb[RTA_DST])), 16);
		}
		if(tb[RTA_GATEWAY])	{
			nh_exist = 1;
			memcpy((char*)&pRT_info->nexthop6[nh_count], (char*)(RTA_DATA(tb[RTA_GATEWAY])), 16);
		}
		if(tb[RTA_OIF]) {
			nh_exist = 1;
			pRT_info->ifindex[nh_count] = *(unsigned int*)(RTA_DATA(tb[RTA_OIF]));
		}
        if(tb[RTA_IIF]) {
            pRT_info->sifindex = *(unsigned int*)(RTA_DATA(tb[RTA_IIF]));
        }
        if (tb[RTA_SRC]) {
            memcpy((char*)&pRT_info->SIP6, (char*)(RTA_DATA(tb[RTA_SRC])), 16);
        }
		if(tb[RTA_MULTIPATH]) {
			rtnh = (struct rtnexthop *)RTA_DATA(tb[RTA_MULTIPATH]); 
			for(;RTNH_OK(rtnh, rtnh->rtnh_len);rtnh=RTNH_NEXT(rtnh))
			{   
				nh_exist++;
		        pRT_info->ifindex[nh_count] = rtnh->rtnh_ifindex;
				pRT_info->nexthop_flag[nh_count] = rtnh->rtnh_flags;
                pRT_info->multipath = 1;
				subrtattr = (struct rtattr * )RTNH_DATA(rtnh);
                if(subrtattr == (struct rtattr * )RTNH_NEXT(rtnh)) {     
                    nh_count++;
                    continue;
                }

				switch(subrtattr->rta_type)
				{
					case RTA_GATEWAY:
						memcpy((char*)&pRT_info->nexthop6[nh_count], (char*)(RTA_DATA(tb[RTA_DST])), 16);
						break;
					default:
						break;
				}
    			nh_count++;
			}
		}		
	}
#endif	//HAVE_NPD_IPV6
	pRT_info->masklen = pRTmsgData->rtm_dst_len;

	if (pRT_info->rtm_family == AF_INET)
		syslog_ax_route_dbg("Get route info: DIP %#x,masklen %d,nexthop %#x,ifindex %d\n",pRT_info->DIP,pRT_info->masklen,pRT_info->nexthop,pRT_info->ifindex);
	else
		syslog_ax_route_dbg("Get route info: v6");
	
    if( nh_exist == 0 
		&& (RTN_UNICAST==pRTmsgData->rtm_type||RTN_MULTICAST==pRTmsgData->rtm_type))
    {
        syslog_ax_route_dbg("Nexthop not exist.\r\n");
        return -1;
    }
    
	return 0;
}

#ifdef HAVE_NPD_IPV6	
int npd_route_v6_delete( hash_table_index_t *hash,void *data, unsigned int flag);
int npd_route_v6_update(hash_table_index_t *hash, void *data,unsigned int flag);
#endif //HAVE_NPD_IPV6


void npd_route_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
    )
{
	struct npd_route_item_s routeEntry; 
    struct npd_mroute_item_s mrouteEntry;
#ifdef HAVE_NPD_IPV6	
	struct npd_route_item_v6_s route6Entry;
#endif	
	unsigned int ifindex;

	syslog_ax_route_dbg("npd notify route index event: index 0x%x event %d\n", netif_index, evt);
	memset(&routeEntry, 0, sizeof(routeEntry));
	memset(&mrouteEntry, 0, sizeof(mrouteEntry));
	
	if( NPD_TRUE != npd_intf_gindex_exist_check( netif_index, &ifindex) )
	{
		return;
	}

	routeEntry.ifindex = ifindex;
    mrouteEntry.dstl3_netif_index = netif_index;
#ifdef HAVE_NPD_IPV6	
	route6Entry.ifindex = ifindex;
#endif
	
    switch(evt)
    {	    
		case PORT_NOTIFIER_L3LINKDOWN:			
#if 0
			dbtable_hash_traversal(npd_route_haship_index, FALSE, &routeEntry, npd_route_cmp_by_ifindex, npd_route_delete);
#ifdef HAVE_IGMP_SNP
	    	npd_mroute_lock();
    		dbtable_hash_lock(npd_mroute_hwid_hash_index);
            dbtable_hash_traversal(npd_mroute_haship_index, FALSE, &mrouteEntry, npd_mroute_cmp_by_dstl3_ifindex, npd_mroute_delete);
			dbtable_hash_unlock(npd_mroute_hwid_hash_index);
		    npd_mroute_unlock();
#endif
#ifdef HAVE_NPD_IPV6			
			dbtable_hash_traversal(npd_route_haship6_index, FALSE, &route6Entry, npd_routev6_cmp_by_ifindex, npd_route_v6_delete);
#endif //HAVE_NPD_IPV6
#endif
			break;
		case PORT_NOTIFIER_L3DELETE:
#if 0				
			dbtable_hash_traversal(npd_route_haship_index, FALSE, &routeEntry, npd_route_cmp_by_ifindex, npd_route_delete);
#ifdef HAVE_IGMP_SNP
			npd_mroute_lock();
    		dbtable_hash_lock(npd_mroute_hwid_hash_index);
			dbtable_hash_traversal(npd_mroute_haship_index, FALSE, &mrouteEntry, npd_mroute_cmp_by_dstl3_ifindex, npd_mroute_delete);
			dbtable_hash_unlock(npd_mroute_hwid_hash_index);
		    npd_mroute_unlock();
#endif
#ifdef HAVE_NPD_IPV6
			dbtable_hash_traversal(npd_route_haship6_index, FALSE, &route6Entry, npd_routev6_cmp_by_ifindex, npd_route_v6_delete);
#endif //HAVE_NPD_IPV6
#endif
			break;
		case PORT_NOTIFIER_L3LINKUP:
#if	0		
			dbtable_hash_traversal(npd_route_hashnhp_index, TRUE, &routeEntry, npd_route_cmp_by_ifindex, npd_route_update);
#ifdef HAVE_NPD_IPV6
			dbtable_hash_traversal(npd_route_hashnhp6_index, TRUE, &route6Entry, npd_routev6_cmp_by_ifindex, npd_route_v6_update);
#endif //HAVE_NPD_IPV6
#endif
			break;
	    default:
	        break;
    }

    return;
}

void npd_route_relate_event(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
    )
{
	return;
}


int npd_route_delete( 
    hash_table_index_t *hash,
    void *data, 
    unsigned int flag)
{
	struct npd_route_item_s *delItem = NULL;
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	delItem = (struct npd_route_item_s *)data;
	
	syslog_ax_route_dbg("Del Route: ip 0x%x masklen %d nexthop 0x%x Ifindex 0x%x rt_type %d\n", delItem->DIP,\
												delItem->masklen, delItem->nexthop, delItem->ifindex, delItem->rt_type);
	ret = dbtable_hash_delete(npd_route_haship_index, delItem, delItem);

	return ret;	
}

int npd_route_del_by_network(unsigned int ipAddr, unsigned int mask)
{
	int ret = 0;
	unsigned int ip_mask[2];
	ip_mask[0] = ipAddr;
	ip_mask[1] = mask; 
	
	ret = dbtable_hash_traversal(npd_route_haship_index, TRUE, (void *)ip_mask, 
												npd_route_cmp_by_nhnetwork, npd_route_delete);

	return ROUTE_RETURN_CODE_SUCCESS;
}

#ifdef HAVE_CAPWAP_ENGINE
int npd_route_find_route(unsigned int ipAddr, unsigned int masklen, struct npd_route_item_s *entry)
{
	int ret = 0;
	struct npd_route_item_s item;

	item.DIP = ipAddr;
	item.masklen = masklen;
	ret = dbtable_hash_search(npd_route_haship_index, &item, npd_route_cmp_by_network, &item);
	if(ret == 0 && entry != NULL)
	{
		memcpy(entry, &item, sizeof(struct npd_route_item_s));
	}

	return ret;
}
#endif //HAVE_CAPWAP_ENGINE

#ifdef HAVE_M4_TUNNEL
unsigned int npd_route_filter_by_network(void* data, void* data_inner)
{
    unsigned int mask = 0;
    struct npd_route_item_s * entry = (struct npd_route_item_s *)data;
    struct npd_route_item_s * entry_inner = (struct npd_route_item_s *)data_inner;

    if ((NULL == entry_inner) || (NULL == entry))
    {
        syslog_ax_route_err("Npd route filter by network: null pointer.\n");
        return FALSE;
    }

    if (entry_inner->masklen == entry->masklen)
    {
        lib_get_mask_from_masklen(entry->masklen, &mask);
        if ((entry_inner->DIP & mask) == (entry->DIP & mask))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int npd_route_next_hop_get_by_network(unsigned int ipAddr, int masklen, unsigned int nh[])
{
    int count = 0;
    unsigned int mask = 0;
    struct npd_route_item_s entry;

    lib_get_mask_from_masklen(masklen, &mask);
    memset(&entry, 0, sizeof(entry));
    entry.masklen = masklen;
    entry.DIP = ipAddr & mask;

    if ((0 == dbtable_hash_head(npd_route_haship_index, &entry, &entry, npd_route_filter_by_network))
        && (0 != entry.nexthop))
    {
        do 
        {
            nh[count++] = entry.nexthop;
        } while ((0 == dbtable_hash_next(npd_route_haship_index, &entry, &entry, npd_route_filter_by_network))
                && (0 != entry.nexthop));
    }

    return count;
}
#endif /* HAVE_M4_TUNNEL */

int npd_route_add( void *data)
{
	struct npd_route_item_s *addItem = NULL;
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	addItem = (struct npd_route_item_s *)data;
	
	syslog_ax_route_dbg("Add Route: ip 0x%x masklen %d nexthop 0x%x Ifindex 0x%x rt_type %d, flag %#x\n", addItem->DIP,\
												addItem->masklen, addItem->nexthop, 
												addItem->ifindex, addItem->rt_type,addItem->flag);
	ret = dbtable_hash_insert(npd_route_haship_index, addItem);

	return ret;	
}

int npd_route_update(hash_table_index_t *hash, void *data,unsigned int flag)
{
	struct npd_route_item_s *item = NULL;
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	item = (struct npd_route_item_s *)data;
	
	syslog_ax_route_dbg("update Route: ip 0x%x masklen %d nexthop 0x%x flag 0x%x current flag %d\n", item->DIP,\
													item->masklen, item->nexthop, item->flag, flag);

	if(TRUE == flag)
		item->flag |= NPD_ROUTE_VALID;
	else
		item->flag &= ~NPD_ROUTE_VALID;

	ret = dbtable_hash_update(npd_route_hashnhp_index, item, item);

 	return ret;
}

int npd_route_update_by_nhp(unsigned int ipAddr, int valid)
{
	struct npd_route_item_s data = {0};
	int ret = 0;

	data.nexthop = ipAddr;

	ret = dbtable_hash_traversal_key(npd_route_hashnhp_index, valid, &data, npd_route_compare_bynhp, npd_route_update);
    
    syslog_ax_route_dbg("npd update routes, for nexthop %#x %s", ipAddr, valid?"Valid":"Invalid" );
	
	return ROUTE_RETURN_CODE_SUCCESS;
}
#ifdef HAVE_ROUTE

#ifdef HAVE_NPD_IPV6
int in6_addr_cmp(ip6_addr *ipaddr1, ip6_addr *ipaddr2)
{
	int i = 0;
	for(i=0;i<4;i++)
	{
		if(ipaddr1->u6_addr32[i] != ipaddr2->u6_addr32[i])
			return 1;
	}

	return 0;
}


/**********************************************************************************
 * npd_route_compare_bynhp
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_route_v6_compare_bynhp
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct npd_route_item_v6_s *itemA = (struct npd_route_item_v6_s *)data1;
	struct npd_route_item_v6_s *itemB = (struct npd_route_item_v6_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(in6_addr_cmp(&(itemA->nexthop), &(itemB->nexthop)))
	{
		equal = FALSE;
	}

	return equal;
}

/**********************************************************************************
 * npd_route_cmp_by_ifindex
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_routev6_cmp_by_ifindex
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct npd_route_item_v6_s *itemA = (struct npd_route_item_v6_s *)data1;
	struct npd_route_item_v6_s *itemB = (struct npd_route_item_v6_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->ifindex != itemB->ifindex)
	{ 
		equal = FALSE;
	}

	return equal;

}


/**********************************************************************************
 * npd_routev6_cmp_by_nhnetwork
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- routev6 database item
 *		itemB	- routev6 database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_routev6_cmp_by_nhnetwork
(
	void *data1,
	void *data2
)
{
	struct npd_route_item_v6_s *itemA = (struct npd_route_item_v6_s*)data1;
	prefix_ipv6_stc *prefix_ipv6_addr = (prefix_ipv6_stc*)data2;
	ip6_addr v6_mask;


	if((NULL==itemA)||(NULL==prefix_ipv6_addr)) {
		syslog_ax_route_err("npd neighbour snooping items compare null pointers error.");
		return FALSE;
	}


	memset(&v6_mask, 0, sizeof(v6_mask));
	lib_get_maskv6_from_masklen( prefix_ipv6_addr->prefixlen, &v6_mask);
	
	syslog_ax_route_dbg("npd routev6 cmp: routev6 nh "IPV6STR", ipv6 addr "IPV6STR" ipv6 mask %#x\n",\
		IPV6_2_STR(itemA->nexthop), IPV6_2_STR(prefix_ipv6_addr->prefix), prefix_ipv6_addr->prefixlen);

	if(IPV6_NET_EQUAL(prefix_ipv6_addr->prefix, itemA->nexthop, v6_mask, v6_mask))
	{ 
		return TRUE;
	}

	return FALSE;

}


/**********************************************************************************
 * npd_route_v6_compare
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_route_v6_compare
(
	void *data1,
	void *data2
)
{
	unsigned int equal = TRUE;
	struct npd_route_item_v6_s *itemA = (struct npd_route_item_v6_s *)data1;
	struct npd_route_item_v6_s *itemB = (struct npd_route_item_v6_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(in6_addr_cmp(&(itemA->DIP), &(itemB->DIP)))
	{
		equal = FALSE;
	}
	else if(itemA->masklen != itemB->masklen) 
	{
		equal = FALSE;
	}
	else if(in6_addr_cmp(&(itemA->nexthop), &(itemB->nexthop)))
	{ 
		equal = FALSE;
	}
	else if (itemA->ifindex != itemB->ifindex)
	{
		equal = FALSE;
	}

	return equal;

}


/**********************************************************************************
 * npd_route_key_ip_v6_generate
 *
 * route database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_route_key_ip_v6_generate
(
	void *data	
)
{
	unsigned int key = 0;
	struct npd_route_item_v6_s *item = (struct npd_route_item_v6_s *)data;

	if(NULL == item) {
		syslog_ax_route_err("npd arp snooping items make key null pointers error.");
		return ~0UI;
	}
    key = jhash2((const unsigned int*)(&(item->DIP)), 4, 0x35798642);
    key %= NPD_ROUTE_HASH_IP_SIZE;

	return key;
}


/**********************************************************************************
 * npd_route_key_nhp_v6_generate
 *
 * route database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_route_key_nhp_v6_generate
(
	void *data
)
{
	unsigned int key = 0;
	struct npd_route_item_v6_s *item = (struct npd_route_item_v6_s *)data;

	if(NULL == item) {
		syslog_ax_route_err("npd arp snooping items make key null pointers error.");
		return ~0UI;
	}

    key = jhash2((const unsigned int*)(&(item->nexthop)), 4, 0x35798642);

	key %= (NPD_ROUTE_HASH_NHP_SIZE);
	
	return key;
}

unsigned int npd_route_key_ifi_v6_generate
(
	void *data
)
{
    struct npd_route_item_v6_s* item = (struct npd_route_item_v6_s *)data;

    if (NULL == item)
    {
        syslog_ax_route_err("Npd route6 ifindex make key null pointers error.\n");
        return ~0UL;
    }

    return item->ifindex % (NPD_ROUTE_HASH_NHP_SIZE);
}

long npd_route_v6_dbtbl_handle_insert( void *newItem )
{
	int ret = ROUTE_RETURN_CODE_SUCCESS;
	struct npd_route_item_v6_s *opItem = NULL;

	if( newItem == NULL )
		return ROUTE_RETURN_CODE_NULL_PTR;

	opItem = (struct npd_route_item_v6_s *)(newItem);	

	syslog_ax_route_dbg("route table insert: add entry 0x%x, masklen %d, nhp 0x%x\n", opItem->DIP, opItem->masklen, opItem->nexthop);

	if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_ADD_ITEM_V6, opItem) )
	{
		syslog_ax_route_err("Add entry 0x%x is wrong\n", opItem->DIP );
		ret =  ROUTE_RETURN_CODE_ERROR;
	}
		
	return ret;
}

long npd_route_v6_dbtbl_handle_delete( void *delItem )
{
	int ret = ROUTE_RETURN_CODE_SUCCESS;
	struct npd_route_item_v6_s *opItem = NULL;

	if( delItem == NULL )
		return ROUTE_RETURN_CODE_NULL_PTR;

	opItem = (struct npd_route_item_v6_s *)(delItem);	

	syslog_ax_route_dbg("route table delete: del entry 0x%x, masklen %d, nhp 0x%x\n", opItem->DIP, opItem->masklen, opItem->nexthop);

	if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_DEL_ITEM_V6, opItem) )
	{
		syslog_ax_route_err("Del entry 0x%x is wrong\n", opItem->DIP );
		ret =  ROUTE_RETURN_CODE_ERROR;
	}
	
	return ret;
}

long npd_route_v6_dbtbl_handle_update(  void *newItem, void *oldItem )
{
	struct npd_route_item_v6_s *newRoute = NULL;
	struct npd_route_item_v6_s *oldRoute = NULL;

	if( (newItem == NULL) || (oldItem == NULL))
		return ROUTE_RETURN_CODE_NULL_PTR;

	newRoute = (struct npd_route_item_v6_s *)(newItem);	
	oldRoute = (struct npd_route_item_v6_s *)(oldItem);	
	
	syslog_ax_route_dbg("route table update: update entry 0x%x\n", newRoute->DIP);

	if(newRoute->flag != oldRoute->flag)
	{
		if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_DEL_ITEM_V6, oldRoute) )
		{
			syslog_ax_route_err("Del entry 0x%x is wrong\n", oldRoute->DIP );
			return ROUTE_RETURN_CODE_ERROR;
		}	
		if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_ADD_ITEM_V6, newRoute) )
		{
			syslog_ax_route_err("Add entry 0x%x is wrong\n", newRoute->DIP );
			return ROUTE_RETURN_CODE_ERROR;
		}	
	}
		
	return ROUTE_RETURN_CODE_SUCCESS;
}

#endif //HAVE_NPD_IPV6


extern unsigned int npd_route_nexthop_key_generate
(
	void *data
);


#endif

#ifdef HAVE_NPD_IPV6

int npd_route_v6_delete( 
    hash_table_index_t *hash,
    void *data, 
    unsigned int flag)
{
	struct npd_route_item_v6_s *delItem = NULL;
	char ipstring[50] = {0};
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	delItem = (struct npd_route_item_v6_s *)data;
	
	syslog_ax_route_dbg("Del Route: ip %s masklen %d nexthop %s Ifindex 0x%x rt_type %d\n", 
								lib_get_string_from_ipv6(ipstring,&(delItem->DIP)),	delItem->masklen, 
								lib_get_string_from_ipv6(ipstring,&(delItem->nexthop)), delItem->ifindex, delItem->rt_type);
	ret = dbtable_hash_delete(npd_route_haship6_index, delItem, delItem);

	return ret;	
}

unsigned int npd_routev6_cmp_by_nhnetwork
(
	void *data1,
	void *data2
);

int npd_route_v6_del_by_network(ip6_addr *ipAddr, int v6_mask_len)
{
	int ret = 0;
	prefix_ipv6_stc prefix_ipv6_addr;
	
	memcpy(&prefix_ipv6_addr.prefix,ipAddr, sizeof(prefix_ipv6_addr.prefix));
	prefix_ipv6_addr.prefixlen = v6_mask_len;
	
	ret = dbtable_hash_traversal(npd_route_haship6_index, TRUE, &prefix_ipv6_addr, 
												npd_routev6_cmp_by_nhnetwork, npd_route_v6_delete);

	return ret;
}

int npd_route_v6_add( void *data)
{
	struct npd_route_item_v6_s *addItem = NULL;
	char ipstring[50] = {0};
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	addItem = (struct npd_route_item_v6_s *)data;
	
	syslog_ax_route_dbg("Add Route: ip %s masklen %d nexthop %s Ifindex 0x%x rt_type %d, flag %#x\n", 
									lib_get_string_from_ipv6(ipstring, &(addItem->DIP)), addItem->masklen, 
									lib_get_string_from_ipv6(ipstring, &(addItem->nexthop)), addItem->ifindex, addItem->rt_type,addItem->flag);
	ret = dbtable_hash_insert(npd_route_haship6_index, addItem);

	return ret;	
}

int npd_route_v6_update(hash_table_index_t *hash, void *data,unsigned int flag)
{
	struct npd_route_item_v6_s *item = NULL;	
	char ipstring[50] = {0};
	char nhstring[50] = {0};
	
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	item = (struct npd_route_item_v6_s *)data;
	
	syslog_ax_route_dbg("update Route: ip %s masklen %d nexthop %s flag 0x%x current flag %d\n", 
												lib_get_string_from_ipv6(ipstring, &(item->DIP)), item->masklen, 
												lib_get_string_from_ipv6(nhstring, &(item->nexthop)), item->flag, flag);

	if(TRUE == flag)
		item->flag |= NPD_ROUTE_VALID;
	else
		item->flag &= ~NPD_ROUTE_VALID;

	ret = dbtable_hash_update(hash, item, item);

 	return ret;
}
unsigned int npd_route_v6_compare_bynhp
(
	void *data1,
	void *data2
);
int npd_route_v6_update_by_nhp(ip6_addr ipAddr, int valid)
{
	struct npd_route_item_v6_s data;
	char ipstring[50] = {0};
	int ret = 0;

	memset(&data, 0, sizeof(struct npd_route_item_v6_s));
	memcpy((char*)&(data.nexthop), (char*)&ipAddr, sizeof(ip6_addr));

	ret = dbtable_hash_traversal_key(npd_route_hashnhp6_index, valid, &data, npd_route_v6_compare_bynhp, npd_route_v6_update);
    
    syslog_ax_route_dbg("npd update routes, for nexthop %s %s", lib_get_string_from_ipv6(ipstring,&ipAddr), valid?"Valid":"Invalid" );
	
	return ROUTE_RETURN_CODE_SUCCESS;
}

#ifdef HAVE_M4_TUNNEL
unsigned int npd_route6_compare_tunnel(void* data1, void* data2)
{
    struct npd_route_item_v6_s* itemA = (struct npd_route_item_v6_s *)data1;
    struct npd_route_item_v6_s* itemB = (struct npd_route_item_v6_s *)data2;

    if ((NULL == itemA) || (NULL == itemB))
    {
        syslog_ax_route_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (itemA->ifindex != itemB->ifindex)
    { 
        return FALSE;
    }

    return TRUE;
}

int  npd_route6_update_tunnel(hash_table_index_t* hash, void* data, unsigned int flag)
{
    int ret = 0;
    struct npd_route_item_v6_s *item = NULL;
    char ipstring[50];
	char nhstring[50];

    if (data == NULL)
    {
        return ROUTE_RETURN_CODE_ERROR;
    }

    item = (struct npd_route_item_v6_s *)data;
    syslog_ax_route_dbg("update Route: ip %s masklen %d nexthop %s flag 0x%x current flag %d\n", 
                                                lib_get_string_from_ipv6(ipstring, &(item->DIP)), item->masklen, 
                                                lib_get_string_from_ipv6(nhstring, &(item->nexthop)), item->flag, flag);


    if (0 == flag)
    {
        item->flag &= ~NPD_ROUTE_VALID;
        dbtable_hash_update(hash, item, item);
    }
    else if (1 == flag)
    {
        item->flag |= NPD_ROUTE_VALID;
        dbtable_hash_update(hash, item, item);
    }

 	return 0;
}

void npd_route6_update_by_tunnel(unsigned int ifindex, int cmd)
{
    struct npd_route_item_v6_s data;

    memset(&data, 0, sizeof(data));
    data.ifindex = ifindex;
    
    dbtable_hash_traversal_key
                (npd_route_hashifi6_index,
                cmd,
                &data,
                npd_route6_compare_tunnel,
                npd_route6_update_tunnel);
}
#endif  /* end HAVE_M4_TUNNEL */
#endif //HAVE_NPD_IPV6

#endif// HAVE_ROUTE

unsigned int get_NextHop_TBL_Entry(unsigned int ifIndex,unsigned int nextHopIp)
{
	unsigned int tblIndex;
	unsigned int retVal = 0;


	retVal = npd_route_nexthop_tblindex_get(ifIndex,nextHopIp,&tblIndex);
	if(ROUTE_RETURN_CODE_SUCCESS != retVal) {
		 syslog_ax_route_err("get route next hop ip %#0x index %#0x at table error %d\n",ifIndex,nextHopIp,retVal);
		return 0;/* Next-Hop table entry 0 with action:TRAP-to-CPU*/
	}
	
	 syslog_ax_route_dbg("get route next hop ip %#0x index %#0x at table line %d\n",nextHopIp,ifIndex,tblIndex);
	return tblIndex;

}

#ifdef HAVE_NPD_IPV6
unsigned int get_ipv6_NextHop_TBL_Entry(unsigned int ifIndex, ip6_addr nextHopIp)
{
	unsigned int tblIndex;
	unsigned int retVal = 0;


	retVal = npd_ipv6_route_nexthop_tblindex_get(ifIndex,nextHopIp,&tblIndex);
	if(ROUTE_RETURN_CODE_SUCCESS != retVal) {
		 syslog_ax_route_err("get route next hop ip "IPV6STR" index %#0x at table error %d\n",
		 			IPV6_2_STR(nextHopIp), ifIndex, retVal);
		return 0;/* Next-Hop table entry 0 with action:TRAP-to-CPU*/
	}
	
	syslog_ax_route_dbg("get route next hop ip "IPV6STR" index %#0x at table line %d\n", \
		IPV6_2_STR(nextHopIp), ifIndex, tblIndex);
	return tblIndex;

}
#endif //HAVE_NPD_IPV6

int npd_route_arp_solicit_send
(
	unsigned int rt_type, 
	unsigned int destIp,
	unsigned int ifindex
)
{
	int ret = 0;
	unsigned int g_index = 0;
	unsigned int netif_index = 0;
	struct arp_snooping_item_s arp;

	if(rt_type == RTN_BLACKHOLE || rt_type == RTN_UNREACHABLE)
		return ROUTE_RETURN_CODE_SUCCESS;
	
	ret = npd_intf_get_global_l3index(ifindex, &g_index);
	if( TRUE != ret )
		return ROUTE_RETURN_CODE_ERROR;
	
	ret = npd_intf_netif_get_by_ifindex(g_index, &netif_index);
	if( NPD_TRUE != ret )
		return ROUTE_RETURN_CODE_ERROR;
	
	memset(&arp, 0, sizeof(arp));
	arp.ipAddr = destIp;
	if(NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
	{
		arp.vid = (unsigned short)npd_netif_vlan_get_vid(netif_index);
	}
	else
	{
		arp.vid = NPD_PORT_L3INTF_VLAN_ID;
		arp.ifIndex = netif_index;
	}
	
	ret = npd_arp_snooping_solicit_send(&arp);
	if( ARP_RETURN_CODE_SUCCESS != ret )
		return ROUTE_RETURN_CODE_ERROR;

	return ROUTE_RETURN_CODE_SUCCESS;
}


int recv_rtMSG_from_kernel(int sockfd,void* ptr,int* buflen)
{
	int msglen=0,totlemsglen=0;
	char* p =ptr;
	int	status;

	while(1)
	{
		struct nlmsghdr *tmp=NULL;
		msglen = recv(sockfd, p, *buflen - totlemsglen, 0);
		if(msglen <= 0)
		{
			syslog_ax_route_dbg("recv msg return error\n");
			status = 1;
			break;
		}
		tmp = (struct nlmsghdr *) p;

		if( (NLM_F_MULTI == tmp->nlmsg_flags) && (NLMSG_DONE == tmp->nlmsg_type)) 
		{
			syslog_ax_route_dbg("route receive message type is NLMSG_DONE\n");
			status = 0;
			break;
		}
		else if( NLMSG_ERROR == tmp->nlmsg_type)
		{
			syslog_ax_route_dbg("recv msg error\n");
			status = 1;
			break;
		}		
		else if( NLMSG_OVERRUN == tmp->nlmsg_type )
		{
			syslog_ax_route_dbg("the buf is overrun when recv msg\n");
			status = 1;
			break;
		}
		else if( NLMSG_NOOP == tmp->nlmsg_type )
		{
			syslog_ax_route_dbg("recev nothing\n");
			status = 1;
			break;
		}
		p += msglen;
		totlemsglen += msglen;

	}
	*buflen = totlemsglen;
	return status;

}


int npd_route_read_link(struct nlmsghdr *nlp, unsigned int srcpid)
{	
	struct  ifinfomsg *rtEntry = NULL;
	int payloadoff = 0;
	struct rtattr *tb[__RTA_MAX + 1];
	int len = 0;
	unsigned int ifindex = 0,eth_g_index = 0;
	unsigned int ifi_flags = 0, ifi_index = ~0UI;;
	int state_up;

	rtEntry = (struct ifinfomsg *) NLMSG_DATA(nlp);
	payloadoff = RTM_PAYLOAD(nlp);
	
	switch( nlp->nlmsg_type ) {
	  case RTM_NEWLINK:
	  	ifi_flags = rtEntry->ifi_flags;  /* get interface */
	  	ifi_index = rtEntry->ifi_index; /* ifindex for kernel*/

		len = nlp->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
		if (len < 0)
			return ROUTE_RETURN_CODE_ERROR;
	  
		memset (tb, 0, sizeof tb);
		netlink_parse_rtattr (tb, __RTA_MAX, IFA_RTA (rtEntry), len);				
		break;
	  case RTM_DELLINK:
	  	ifi_flags = rtEntry->ifi_flags;  /* get interface */
	  	ifi_index = rtEntry->ifi_index; /* ifindex for kernel*/
		if( TRUE == npd_intf_get_global_l3index(ifi_index, &ifindex))
		{
	  		npd_intf_del_all_ip_addr(ifindex);
		}
		return ROUTE_RETURN_CODE_SUCCESS;
	  default:
		 break;
	}	

	if(ifi_flags & (IFF_UP | IFF_RUNNING)) {
		state_up = PORT_NOTIFIER_L3LINKUP; 
	}
	else {
		state_up = PORT_NOTIFIER_L3LINKDOWN;
	}

	if( TRUE == npd_intf_get_global_l3index(ifi_index, &ifindex))
	{
		if(NPD_TRUE == npd_intf_netif_get_by_ifindex(ifindex, &eth_g_index ))
		{
			npd_key_database_lock();
			netif_notify_event(eth_g_index, state_up);
            npd_key_database_unlock();
		}
	}

	return ROUTE_RETURN_CODE_SUCCESS;
}



int npd_route_read_ipv4(struct nlmsghdr *nlp, unsigned int srcpid)
{
	int len = 0;
	unsigned int isAdd = 1;
	struct ifaddrmsg *ifa;
	struct rtattr *tb[__RTA_MAX + 1];
	static char buf[BUFLENGTH],*ip = NULL;
	unsigned int ipAddr = 0;
	unsigned int netAddr = 0;
	unsigned int mask = 0;
	unsigned int l3Index = 0;

	ifa = NLMSG_DATA (nlp);
	if(ifa == NULL)
		return ROUTE_RETURN_CODE_ERROR;

	len = nlp->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifaddrmsg));
	if (len < 0)
		return ROUTE_RETURN_CODE_ERROR;

	if(nlp->nlmsg_type == RTM_NEWADDR){
		isAdd = 1;
	}
	else if(nlp->nlmsg_type == RTM_DELADDR){
		isAdd = 0;
	}

	memset (tb, 0, sizeof tb);
	netlink_parse_rtattr (tb, __RTA_MAX, IFA_RTA (ifa), len);
	syslog_ax_route_dbg("intf index is %d\n",ifa->ifa_index);

	if (tb[IFA_LOCAL]){
		syslog_ax_route_dbg("  IFA_LOCAL	 %s/%d",
			inet_ntop (ifa->ifa_family, RTA_DATA (tb[IFA_LOCAL]),buf, BUFLENGTH), ifa->ifa_prefixlen);
		ip = buf;
	}
	else{
		syslog_ax_route_dbg(" LOCAL ip address is not found\n");
		return ROUTE_RETURN_CODE_ERROR;
	}

	if( TRUE != npd_intf_get_global_l3index(ifa->ifa_index, &l3Index) )
	{
		syslog_ax_route_dbg(" layer 3 index is not found\n");
		return ROUTE_RETURN_CODE_ERROR;
	}
	  
	memcpy(&ipAddr,RTA_DATA (tb[IFA_LOCAL]),sizeof(unsigned int));
	lib_get_mask_from_masklen((int) ifa->ifa_prefixlen, (int *)&mask);
	  
	if(!isAdd ){
		netAddr = ipAddr & mask;
		syslog_ax_route_dbg("delete arp by network %d.%d.%d.%d/%d in read_ip\n",\
		(netAddr>>24)&0xff,(netAddr>>16)&0xff,(netAddr>>8)&0xff,netAddr&0xff,\
		(ifa->ifa_prefixlen));
#ifdef HAVE_ROUTE
		npd_route_del_by_network(ipAddr, mask);
#endif
		npd_arp_snooping_del_by_network(ipAddr,mask);
		npd_arp_snooping_del_static_by_network(ipAddr, mask);		
		npd_intf_del_ip_addr(l3Index, ipAddr, mask);		
		syslog_ax_route_dbg("delete to kernel \n");
	}
	else{
		syslog_ax_route_dbg("intf add addr \n");
		npd_intf_add_ip_addr(l3Index, ipAddr, mask);
	}
	
	return ROUTE_RETURN_CODE_SUCCESS;
}


#ifdef HAVE_NPD_IPV6
int npd_route_read_ipv6(struct nlmsghdr *nlp, unsigned int srcpid)
{
	int len = 0;
	unsigned int isAdd = 1;
	struct ifaddrmsg *ifa;
	struct rtattr *tb[__RTA_MAX + 1];
	static char buf[BUFLENGTH],*ip = NULL;
	ip6_addr ipAddr;
	ip6_addr mask;
	ip6_addr netAddr;
	unsigned int l3Index = 0;
		  
	ifa = NLMSG_DATA (nlp);
	if(ifa == NULL)
		return ROUTE_RETURN_CODE_ERROR;

	len = nlp->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifaddrmsg));
	if (len < 0)
		return ROUTE_RETURN_CODE_ERROR;

	if(nlp->nlmsg_type == RTM_NEWADDR){
		isAdd = 1;
	}
	else if(nlp->nlmsg_type == RTM_DELADDR){
		isAdd = 0;
	}

	memset (tb, 0, sizeof tb);
	netlink_parse_rtattr (tb, __RTA_MAX, IFA_RTA (ifa), len);
	syslog_ax_route_dbg("intf index is %d\n",ifa->ifa_index);

	if(tb[IFA_ADDRESS]){
		syslog_ax_route_dbg("  IFA_ADDRESS	 %s/%d", 
				inet_ntop(ifa->ifa_family, RTA_DATA (tb[IFA_ADDRESS]), buf, BUFLENGTH), ifa->ifa_prefixlen);
		ip = buf;
	}
	else{
		syslog_ax_route_dbg(" LOCAL ip address is not found\n");
		return ROUTE_RETURN_CODE_ERROR;
	}

	if( TRUE != npd_intf_get_global_l3index(ifa->ifa_index, &l3Index) )
	{
		syslog_ax_route_dbg(" layer 3 index is not found\n");
		return ROUTE_RETURN_CODE_ERROR;
	}

	memcpy(&ipAddr,RTA_DATA (tb[IFA_ADDRESS]),sizeof(ipAddr));
	memset(&mask, 0, sizeof(mask));
	lib_get_maskv6_from_masklen((int) ifa->ifa_prefixlen, &mask);
	  
	if(!isAdd ){
		int i;
		for(i=0; i<4; i++){
			netAddr.u6_addr32[i] = ipAddr.u6_addr32[i]&mask.u6_addr32[i];
		}
		syslog_ax_route_dbg("delete neighbour by network "IPV6STR"/%d in read_ipv6\n",\
				IPV6_2_STR(netAddr),ifa->ifa_prefixlen);
				
		npd_route_v6_del_by_network(&ipAddr, ifa->ifa_prefixlen);
		npd_ndisc_snooping_del_by_network(&ipAddr,ifa->ifa_prefixlen);
		npd_ndisc_snooping_static_valid_set_by_net(&ipAddr, ifa->ifa_prefixlen, FALSE);
		npd_intf_del_ipv6_addr(l3Index, &ipAddr,&mask);		
		syslog_ax_route_dbg("delete to kernel \n");
		//npd_intf_netif_get_by_ifindex(l3Index, &netif_index);
		//ipno[0] = ipAddr;
		//ipno[1] = (int) ifa->ifa_prefixlen;
		//netif_app_notify_relate_event(netif_index,netif_index,PORT_NOTIFIER_ADDR_DEL, &ipno, sizeof(ipno));
	}
	else{
		syslog_ax_route_dbg("intf add addr \n");
		npd_intf_add_ipv6_addr(l3Index, &ipAddr, &mask);
		npd_ndisc_snooping_static_valid_set_by_net( &ipAddr, ifa->ifa_prefixlen , TRUE);
	}

	return ROUTE_RETURN_CODE_SUCCESS;
}

#endif //HAVE_NPD_IPV6
int npd_route_read_ip(struct nlmsghdr *nlp, unsigned int srcpid)
{
	  struct ifaddrmsg *ifa;
	 	  
	  ifa = NLMSG_DATA (nlp);
	  if(ifa == NULL)
	  	return ROUTE_RETURN_CODE_ERROR;
	  	
	  switch(ifa->ifa_family){
		  case AF_INET:{
		  	return npd_route_read_ipv4(nlp, srcpid);
		  	break;
		  }
#ifdef HAVE_NPD_IPV6		  
		  case AF_INET6:{
		  	return npd_route_read_ipv6(nlp, srcpid);
		  	break;
		  }
#endif //HAVE_NPD_IPV6		  
		  default:{
		  	syslog_ax_route_dbg("Unknown protocol family %d\n",ifa->ifa_family);
		  	return ROUTE_RETURN_CODE_ERROR;
		  	break;
		  }
	  }
	  return ROUTE_RETURN_CODE_ERROR;
}

#ifdef HAVE_ROUTE
int npd_route_set_route_v4(struct nlmsghdr *nlp, struct rttbl_info *rtInfo, int nh_count)
{
	struct npd_route_item_s dbItem;	
	struct rtmsg *rtEntry=NULL;
	struct route_nexthop_brief_s nexthopItem;
	unsigned int attribute = 0;
	int ret = ROUTE_RETURN_CODE_SUCCESS;
    struct npd_route_cfg_s cfg = {0};


	rtEntry = (struct rtmsg *) NLMSG_DATA(nlp);
	
	memset(&dbItem, 0, sizeof(struct npd_route_item_s));
	dbItem.DIP = rtInfo->DIP;
    dbItem.sifindex = rtInfo->sifindex;
    dbItem.sip = rtInfo->SIP;
	dbItem.nexthop = rtInfo->nexthop[nh_count];
	dbItem.masklen = rtInfo->masklen;		
	dbItem.rt_type = rtInfo->rt_type;
	ret = npd_intf_get_global_l3index(rtInfo->ifindex[nh_count], &(dbItem.ifindex));
	if(ret == TRUE)
	{
		ret = npd_intf_attribute_get_by_ifindex(dbItem.ifindex, &attribute);
	}
    ret = npd_intf_get_global_l3index(rtInfo->sifindex, &dbItem.sifindex);

    dbtable_array_get(npd_route_seqcfg_index, 0, &cfg);
    switch(cfg.urpf_strict_enable)
    {
        case URPF_STRICT_ALL:
		    dbItem.flag |= NPD_ROUTE_URPF;
            break;
        case URPF_STRICT_EXCLUDE_DEFAULT:
            if(dbItem.DIP != 0)
		        dbItem.flag |= NPD_ROUTE_URPF;
            break;
        default:
            break;
    }
                
	dbItem.flag |= NPD_ROUTE_VALID;
    if(rtInfo->rt_type == RTN_MULTICAST)
        dbItem.flag |= NPD_ROUTE_MCAST;
	if(rtInfo->multipath)
		dbItem.flag |= NPD_ROUTE_ECMP;
	
	if( rtEntry->rtm_protocol == RTPROT_STATIC)
		dbItem.isStatic = TRUE;
	else
		dbItem.isStatic = FALSE;

	switch( nlp->nlmsg_type )
	{
		case RTM_NEWROUTE:
		case RTM_GETROUTE:
			if(dbItem.nexthop && !(dbItem.flag & NPD_ROUTE_MCAST) && NPD_FALSE == npd_route_nexthop_find(dbItem.nexthop, &nexthopItem))
			{
				npd_route_arp_solicit_send(dbItem.rt_type, dbItem.nexthop, dbItem.ifindex );
				dbItem.flag &= ~NPD_ROUTE_VALID;
			}
			ret = npd_route_add(&dbItem);
#ifdef HAVE_M4_TUNNEL
            if ((0 == ret) && (RTM_NEWROUTE == nlp->nlmsg_type))
            {
                npd_tunnel_update_by_route_add(dbItem.DIP, dbItem.masklen, dbItem.nexthop);
            }
#endif
			break;
		case RTM_DELROUTE:
			ret = npd_route_delete(NULL, &dbItem, 0);
#ifdef HAVE_M4_TUNNEL
            if (0 == ret)
            {
                npd_tunnel_update_by_route_delete(dbItem.DIP, dbItem.masklen, dbItem.nexthop);
            }
#endif
			break;			
		default:
			syslog_ax_route_err("ignore msg type %d\n",nlp->nlmsg_type);
			ret = ROUTE_RETURN_CODE_ERROR;
			break;
	}

	return ret;
}

#ifdef HAVE_NPD_IPV6
unsigned int in6_addr_is_zero(ip6_addr *ipaddr)
{
	return !(ipaddr->u6_addr32[0]| ipaddr->u6_addr32[1]|ipaddr->u6_addr32[2]|ipaddr->u6_addr32[3]);
}

int npd_route_set_route_v6(struct nlmsghdr *nlp, struct rttbl_info *rtInfo, int nh_count)
{
	struct npd_route_item_v6_s dbItem;	
	struct route_ipv6_nexthop_brief_s nexthopEntry;
	struct rtmsg *rtEntry=NULL;
	int ret = ROUTE_RETURN_CODE_SUCCESS;


	rtEntry = (struct rtmsg *) NLMSG_DATA(nlp);
		
	memset(&dbItem, 0, sizeof(typeof(dbItem)));
	memcpy(&dbItem.DIP, &rtInfo->DIP6, sizeof(typeof(dbItem.DIP)));
	memcpy(&dbItem.nexthop, &rtInfo->nexthop6[nh_count], sizeof(typeof(dbItem.nexthop)));
	dbItem.masklen = rtInfo->masklen;		
	dbItem.rt_type = rtInfo->rt_type;
	npd_intf_get_global_l3index(rtInfo->ifindex[nh_count], &(dbItem.ifindex));

#ifdef HAVE_M4_TUNNEL
    if (in6_addr_is_zero(&dbItem.nexthop))
    {
        unsigned int netif_index = 0;
        
        npd_intf_netif_get_by_ifindex(dbItem.ifindex, &netif_index);
        if (NPD_NETIF_TUNNEL_TYPE == npd_netif_type_get(netif_index))
        {
            dbItem.flag |= NPD_ROUTE_NH_TUNNEL;
        }
    }
#endif
    
	dbItem.flag |= NPD_ROUTE_VALID;
	if(rtInfo->multipath)
		dbItem.flag |= NPD_ROUTE_ECMP;
	
	if( rtEntry->rtm_protocol == RTPROT_STATIC)
		dbItem.isStatic = TRUE;
	else
		dbItem.isStatic = FALSE;

	switch( nlp->nlmsg_type )
	{
		case RTM_NEWROUTE:
		case RTM_GETROUTE:
			if(!in6_addr_is_zero(&dbItem.nexthop) && !(dbItem.flag & NPD_ROUTE_MCAST) && NPD_FALSE == npd_ipv6_route_nexthop_find(dbItem.nexthop, &nexthopEntry))
			{
				//npd_ndisc_snooping_solicit_send();
				dbItem.flag &= ~NPD_ROUTE_VALID;
			}
			ret = npd_route_v6_add(&dbItem);
			break;
		case RTM_DELROUTE:
			ret = npd_route_v6_delete(NULL, &dbItem, 0);
			break;				
		default:
			syslog_ax_route_err("ignore msg type %d\n",nlp->nlmsg_type);
			ret = ROUTE_RETURN_CODE_ERROR;
			break;
	}
	return ret;
}

#endif //HAVE_NPD_IPV6

int routing_socket = -1;
static pid_t pid;
static __u32 seq;

static int getmsg(struct rtmsg *rtm, int msglen, unsigned int source, unsigned int *nexthop);


static int
addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
    int len = RTA_LENGTH(4);
    struct rtattr *rta;

    if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
    	return -1;
    rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    memcpy(RTA_DATA(rta), &data, 4);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
    return 0;
}

static int
parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
    while (RTA_OK(rta, len)) {
    	if (rta->rta_type <= max)
    	    tb[rta->rta_type] = rta;
    	rta = RTA_NEXT(rta, len);
    }
    return 0;
}

/* open and initialize the routing socket */
int init_routesock(void)
{
    unsigned int addr_len;
    struct sockaddr_nl local;

    routing_socket = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (routing_socket < 0) {
    	return -1;
    }
    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_groups = 0;
    
    if (bind(routing_socket, (struct sockaddr *) &local, sizeof(local)) < 0) {
    	return -1;
    }
    addr_len = sizeof(local);
    if (getsockname(routing_socket, (struct sockaddr *) &local, &addr_len) < 0) {
    	return -1;
    }
    if (addr_len != sizeof(local)) {
    	return -1;
    }
    if (local.nl_family != AF_NETLINK) {
    	return -1;
    }
    pid = local.nl_pid;
    seq = time(NULL);
    return 0;
}

/* get the rpf neighbor info */
int
k_req_incoming(unsigned int source, unsigned int *nexthop)
{
    int rlen;
    register int l;
    char buf[512];
    struct nlmsghdr *n = (struct nlmsghdr *) buf;
    struct rtmsg *r = NLMSG_DATA(n);
    struct sockaddr_nl addr;
    
    
    n->nlmsg_type = RTM_GETROUTE;
    n->nlmsg_flags = NLM_F_REQUEST;
    n->nlmsg_len = NLMSG_LENGTH(sizeof(*r));
    n->nlmsg_pid = pid;
    n->nlmsg_seq = ++seq;
    
    memset(r, 0, sizeof(*r));
    r->rtm_family = AF_INET;
    r->rtm_dst_len = 32;
    addattr32(n, sizeof(buf), RTA_DST, source);

    addr.nl_family = AF_NETLINK;
    addr.nl_groups = 0;
    addr.nl_pid = 0;
    
    if ((rlen = sendto(routing_socket, buf, n->nlmsg_len, 0, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
    	npd_syslog_warning("Error %d writing to routing socket\n", errno);
    	return FALSE;
    }
    do {
    	unsigned int alen = sizeof(addr);
    	l = recvfrom(routing_socket, buf, sizeof(buf), 0, (struct sockaddr *) &addr, &alen);
    	if (l < 0) {
    	    if (errno == EINTR)
        		continue;
    	    npd_syslog_warning("Error %d writing to routing socket\n", errno);
    	    return FALSE;
    	}
    } while (n->nlmsg_seq != seq || n->nlmsg_pid != pid);
    
    if (n->nlmsg_type != RTM_NEWROUTE) {
    	if (n->nlmsg_type != NLMSG_ERROR) {
    	    npd_syslog_warning("netlink: Get wrong answer type %d\n",
    		   n->nlmsg_type);
    	} else {
    	    npd_syslog_warning("netlink: Get route msg\n");
    	}
    	return FALSE;
    }
    return getmsg(NLMSG_DATA(n), l - sizeof(*n), source, nexthop);
}

static int
getmsg(struct rtmsg *rtm, int msglen, unsigned int source, unsigned int *nexthop)
{
    struct rtattr *rta[RTA_MAX + 1];
    
    if (rtm->rtm_type == RTN_LOCAL) {
        *nexthop = source;
    }
    
    if (rtm->rtm_type != RTN_UNICAST) {
        *nexthop = 0;
    	return FALSE;
    }
    
    memset(rta, 0, sizeof(rta));
    
    parse_rtattr(rta, RTA_MAX, RTM_RTA(rtm), msglen - sizeof(*rtm));
    
    if (rta[RTA_GATEWAY]) {
    	*nexthop = *(unsigned int *) RTA_DATA(rta[RTA_GATEWAY]);
    } 
    else
        *nexthop = source;
    return TRUE;
}

unsigned int npd_mroute_compar_ip
(
	void *data1,
	void *data2
);

#ifdef HAVE_IGMP_SNP
int npd_mroute_nexthop_alloc(struct npd_mroute_item_s *mroute)
{
	int ret = 0;
	struct route_nexthop_hwid_s nexthop = {0};	
	nexthop.ipAddr = mroute->dip;
	nexthop.srcIp = mroute->sip;
#ifdef HAVE_NPD_IPV6
	IPV6_ADDR_COPY(&nexthop.dipv6, &mroute->dipv6);
    IPV6_ADDR_COPY(&nexthop.sipv6, &mroute->sipv6);	
#endif

	ret = dbtable_hash_search(npd_mroute_hwid_hash_index, &nexthop, NULL, &nexthop);
	if(0 != ret)
	{
        l3_mc_index_alloc(&nexthop.tblIndex);
		l2_mc_index_alloc(&nexthop.l2mc_index);		
		ret = dbtable_hash_insert(npd_mroute_hwid_hash_index, &nexthop);
	}
	
	mroute->tbl_index = nexthop.tblIndex;
    mroute->l2mc_index = nexthop.l2mc_index; 

	return ret;
}

int npd_mroute_nexthop_check_free(struct npd_mroute_item_s *mroute)
{
	struct route_nexthop_hwid_s nexthop = {0};
    struct npd_mroute_item_s temp = {0};
	int delete_mroute = TRUE;
    int ret = 0;

    ret = dbtable_hash_head_key(npd_mroute_haship_index, mroute, &temp, npd_mroute_compar_ip);
    if (0 == ret)
    {
        delete_mroute = FALSE;
    }
        
	if(TRUE == delete_mroute)
	{
        nexthop.ipAddr = mroute->dip;
	    nexthop.srcIp = mroute->sip;
#ifdef HAVE_NPD_IPV6
        IPV6_ADDR_COPY(&nexthop.dipv6, &mroute->dipv6);
        IPV6_ADDR_COPY(&nexthop.sipv6, &mroute->sipv6);
#endif
#ifdef HAVE_ROUTE
        dbtable_hash_delete(npd_mroute_hwid_hash_index, &nexthop, &nexthop);
		l3_mc_index_free(mroute->tbl_index);
		l2_mc_index_free(mroute->l2mc_index);
#endif		
	}

    return 0;
}

int npd_mroute_set(struct nlmsghdr *nlp, struct rttbl_info *rtInfo)
{
    struct npd_mroute_item_s mroute = {0};	
	struct npd_mroute_item_s mroute_exist = {0};
    struct arp_snooping_item_s item = {0};
    struct arp_snooping_item_s dbItem = {0};
    unsigned int ifindex,netif_index;
    unsigned int swPortIndex = 0;
    int ret;
    int nh_count;

	mroute.dip = rtInfo->DIP;
    mroute.sip = rtInfo->SIP;
    mroute.rt_type = rtInfo->rt_type;
	mroute.flag |= (NPD_ROUTE_VALID|NPD_ROUTE_MCAST);
    mroute.family = rtInfo->rtm_family;
    if(rtInfo->sifindex != -1)
    {
        ret = npd_intf_get_global_l3index(rtInfo->sifindex, &mroute.srcl3_g_index);
		if(ret)
		{
            ret = npd_intf_netif_get_by_ifindex(mroute.srcl3_g_index, &mroute.srcl3_netif_index);
            if(NPD_NETIF_VLAN_TYPE == npd_netif_type_get(mroute.srcl3_netif_index))
            {
                mroute.svid = npd_netif_vlan_get_vid(mroute.srcl3_netif_index);
            }
			else
				mroute.svid = NPD_PORT_L3INTF_VLAN_ID;
		}
		else 
		{
            mroute.srcl3_g_index = -1;
            mroute.srcl3_netif_index = -1;
            mroute.srcl2_netif_index = -1;
		}
    }
    else
    {
        mroute.srcl3_g_index = -1;
        mroute.srcl3_netif_index = -1;
        mroute.srcl2_netif_index = -1;
    }
        
	if(rtInfo->sgateway != 0)
	{
		ret = npd_arp_snooping_find_item_byip(rtInfo->sgateway, &item); 
        if(0 != ret)
        {
            mroute.srcl2_netif_index = -1;
        }
        else
        {
			if(item.l3Index != mroute.srcl3_g_index)
				mroute.srcl2_netif_index = -1;
			else
				mroute.srcl2_netif_index = item.ifIndex;
        }
	}
	else if(rtInfo->SIP != 0)
    {
        unsigned int nexthop = 0;
        k_req_incoming(mroute.sip, &nexthop);
        ret = npd_arp_snooping_find_item_byip(nexthop, &item); 
        if(0 != ret)
        {
            mroute.srcl2_netif_index = -1;
        }
        else
        {
			if(item.l3Index != mroute.srcl3_g_index)
				mroute.srcl2_netif_index = -1;
			else
				mroute.srcl2_netif_index = item.ifIndex;
        }
    }	

	/*Delete (S,G) or (*,G) multicast route entry */
	if(((nlp->nlmsg_type == RTM_DELROUTE) && (rtInfo->rtm_flag & 0x1000))  
		||(nlp->nlmsg_type == RTM_NEWROUTE))
	{
		npd_key_database_lock();
        npd_mroute_lock();
        dbtable_hash_lock(npd_mroute_hwid_hash_index);
        ret = dbtable_hash_head_key(npd_mroute_haship_index, &mroute, &mroute_exist, npd_mroute_compar_ip);
		while(ret==0)
		{			
			ret = dbtable_hash_next_key(npd_mroute_haship_index, &mroute_exist, &mroute, npd_mroute_compar_ip);
			if(mroute_exist.rt_type == RTN_L2L3MULTICAST && !NPD_PBMP_IS_NULL(mroute_exist.dst_l2_ports))
            {
                mroute_exist.rt_type = RTN_L2MULTICAST;
                mroute_exist.svid = mroute_exist.dst_vid;
                mroute_exist.srcl3_g_index = -1;
                mroute_exist.srcl2_netif_index = -1;
                mroute_exist.srcl3_netif_index = -1;
                dbtable_hash_update(npd_mroute_haship_index, NULL, &mroute_exist);
            }
            else if(mroute_exist.rt_type == RTN_MULTICAST || NPD_PBMP_IS_NULL(mroute_exist.dst_l2_ports))
            {
                dbtable_hash_delete(npd_mroute_haship_index, &mroute_exist, &mroute_exist);
                syslog_ax_route_dbg("Delete mroute dip %x, dstvid %d, sip %x, svid %d, mc index %d", 
												mroute_exist.dip, mroute_exist.dst_vid,mroute_exist.sip, 
												mroute_exist.svid, mroute_exist.tbl_index);
                npd_mroute_nexthop_check_free(&mroute_exist);
            }
			memcpy(&mroute_exist, &mroute, sizeof(struct npd_mroute_item_s));
		}
		dbtable_hash_unlock(npd_mroute_hwid_hash_index);
        npd_mroute_unlock();
		npd_key_database_unlock();
		if(nlp->nlmsg_type == RTM_DELROUTE)
			return 0;
	}
	
    for(nh_count = 0; nh_count < NPD_ROUTE_MAX_NH_NUM; nh_count++)
    {
        unsigned int dst_netif_index = -1;
        unsigned int l3index = -1;
        NPD_PBMP_CLEAR(mroute.dst_l2_ports);
        
		if( (0 != nh_count) 
            && (0 == rtInfo->nexthop[nh_count]) 
            && (0 == rtInfo->ifindex[nh_count])
            )
		    continue;
        ret = npd_intf_get_global_l3index(rtInfo->ifindex[nh_count], &l3index);
        if(FALSE == ret)
            continue;
		if(rtInfo->nexthop_flag[nh_count] & RTNH_F_DEAD) 
		    mroute.flag |= NPD_ROUTE_MCAST_CPU;
        else
			mroute.flag &= (~NPD_ROUTE_MCAST_CPU);
        mroute.dstl3_g_index = l3index;
        ret = npd_intf_netif_get_by_ifindex(l3index, &dst_netif_index);
        if(NPD_FALSE == ret)
            continue;
        mroute.dstl3_netif_index = dst_netif_index;
        if(NPD_NETIF_VLAN_TYPE == npd_netif_type_get(dst_netif_index))
        {
            mroute.dst_vid = npd_netif_vlan_get_vid(dst_netif_index);
        }
        else
        {
            int array_port = netif_array_index_from_ifindex(dst_netif_index);
			if(array_port != -1)
			{
                NPD_PBMP_PORT_ADD(mroute.dst_l2_ports, array_port);
			}
            mroute.dst_vid = NPD_PORT_L3INTF_VLAN_ID;
        }

		/*for bidirect PIM, the (*,G) would be created by multicat data flow,
		  and corresponding out if is RPF, which could be get from IGMP Snooping,
		  instead, the output port need to be found from ARP or ports from vlan*/
        if (0 != rtInfo->nexthop[nh_count])
        {
            ret = npd_intf_netif_get_by_ip(&ifindex, rtInfo->nexthop[nh_count]);
            if(ret == NPD_TRUE && NPD_TRUE == npd_intf_netif_get_by_ifindex(ifindex, &netif_index ))
            {
                switch(npd_netif_type_get(netif_index))
                {
                
                    case NPD_NETIF_VLAN_TYPE:
                        {
                            struct vlan_s *vlanEntry = NULL;
                            vlanEntry = npd_find_vlan_by_vid(npd_netif_vlan_get_vid(netif_index));
                            if(vlanEntry)
                            {
                                NPD_PBMP_OR(mroute.dst_l2_ports, vlanEntry->untag_ports);
                                NPD_PBMP_OR(mroute.dst_l2_ports, vlanEntry->tag_ports);
                                npd_put_vlan(vlanEntry);
                            }
                            break;
                        }
                    case NPD_NETIF_ETH_TYPE:
                        NPD_PBMP_PORT_ADD(mroute.dst_l2_ports, netif_index);
                        break;
                    case NPD_NETIF_TRUNK_TYPE:
                        {
                            struct trunk_s trunkEntry;
                            if(0 == npd_find_trunk(npd_netif_trunk_get_tid(netif_index), &trunkEntry))
                            {
                                  NPD_PBMP_OR(mroute.dst_l2_ports, trunkEntry.ports);
                            }
                            break;
                        }
                     default:
                        break;
                   }
            }
            else {
                ret = npd_arp_snooping_find_item_byip(rtInfo->nexthop[nh_count], &dbItem);
                if (ret == 0)
                {
                    swPortIndex = netif_array_index_from_ifindex(dbItem.ifIndex); 
                    NPD_PBMP_PORT_ADD(mroute.dst_l2_ports, swPortIndex);
                }
            }
        }
		
        npd_key_database_lock();
        npd_mroute_lock();
        dbtable_hash_lock(npd_mroute_hwid_hash_index);
        ret = dbtable_hash_search(npd_mroute_haship_index, &mroute, NULL, &mroute_exist);
    	switch( nlp->nlmsg_type )
    	{
    		case RTM_NEWROUTE:
    		case RTM_GETROUTE:
                if(0 != ret)
                {
                    mroute_exist = mroute;
                    mroute_exist.sip = 0;
                    ret = dbtable_hash_search(npd_mroute_haship_index, &mroute_exist, 
                        NULL, &mroute_exist);
                    if(0 != ret)
                    {
						if(0 != npd_mroute_nexthop_alloc(&mroute))
						{
							npd_syslog_route_err("pim alloc nexthop fail, mroute dip %x dstvid %d, mcindex %d", 
			                     										mroute.dip, mroute.dst_vid, mroute.tbl_index);
							ret = ROUTE_RETURN_CODE_ERROR;
							goto switch_end;
						}
                        if(0 != dbtable_hash_insert(npd_mroute_haship_index, &mroute))
                        {
                        	npd_syslog_route_err("mroute fail to insert dip %x, dstvid %d,"
			                            "sip %x, svid %d, mc index %d", mroute.dip, mroute.dst_vid,
					                            mroute.sip, mroute.svid, mroute.tbl_index);
							npd_mroute_nexthop_check_free(&mroute);
							ret = ROUTE_RETURN_CODE_ERROR;
							goto switch_end;
                        }
						npd_syslog_route_dbg("mroute insert dip %x, dstvid %d,"
			                            "sip %x, svid %d, mc index %d", mroute.dip, mroute.dst_vid,
								                            mroute.sip, mroute.svid, mroute.tbl_index);
                    }
                    else
                    {
                        NPD_PBMP_OR(mroute.dst_l2_ports, mroute_exist.dst_l2_ports);
						if(0 != npd_mroute_nexthop_alloc(&mroute))
						{
							npd_syslog_route_err("pim alloc nexthop fail, mroute dip %x dstvid %d, mcindex %d", 
			                     										mroute.dip, mroute.dst_vid, mroute.tbl_index);
							ret = ROUTE_RETURN_CODE_ERROR;
							goto switch_end;
						}     
                        if(0 != dbtable_hash_insert(npd_mroute_haship_index, &mroute))
						{
                        	npd_syslog_route_err("mroute fail to insert dip %x, dstvid %d, sip %x, svid %d, mc index %d", 
														mroute.dip, mroute.dst_vid, mroute.sip, mroute.svid, mroute.tbl_index);
							npd_mroute_nexthop_check_free(&mroute);
							ret = ROUTE_RETURN_CODE_ERROR;
							goto switch_end;
                        }
                        npd_syslog_route_dbg("Insert mroute dip %x, dstvid %d, sip %x, svid %d, mc index %d", 
													mroute.dip, mroute.dst_vid, mroute.sip, mroute.svid, mroute.tbl_index);
                    }
                }
                else
                {
                    if(mroute_exist.rt_type == RTN_L2MULTICAST)
                        mroute.rt_type = RTN_L2L3MULTICAST;
					else 
						mroute.rt_type = mroute_exist.rt_type;
                    NPD_PBMP_OR(mroute.dst_l2_ports, mroute_exist.dst_l2_ports);
					NPD_PBMP_OR(mroute.l2_real_ports, mroute_exist.l2_real_ports);
					mroute.tbl_index = mroute_exist.tbl_index;
                    mroute.l2mc_index = mroute_exist.l2mc_index;
                    ret = dbtable_hash_update(npd_mroute_haship_index, NULL, &mroute);
                    syslog_ax_route_dbg("Update mroute dip %x, dstvid %d,sip %x, svid %d to svid %d", 
											mroute.dip, mroute.dst_vid,	mroute.sip, mroute_exist.svid, mroute.svid);
                }
                break;
            case RTM_DELROUTE:
				if(0 == ret)
				{
					if(mroute.flag & NPD_ROUTE_MCAST_CPU)
					{
						mroute_exist.flag |= NPD_ROUTE_MCAST_CPU;
                        ret = dbtable_hash_update(npd_mroute_haship_index, NULL, &mroute_exist);
					}
                    else if(mroute_exist.rt_type == RTN_L2L3MULTICAST && !NPD_PBMP_IS_NULL(mroute_exist.dst_l2_ports))
                    {
                        mroute_exist.rt_type = RTN_L2MULTICAST;
                        mroute_exist.svid = mroute_exist.dst_vid;
                        mroute_exist.srcl3_g_index = -1;
                        mroute_exist.srcl2_netif_index = -1;
                        mroute_exist.srcl3_netif_index = -1;
                        ret = dbtable_hash_update(npd_mroute_haship_index, NULL, &mroute_exist);
                    }
                    else if(mroute_exist.rt_type == RTN_MULTICAST || NPD_PBMP_IS_NULL(mroute_exist.dst_l2_ports))
                    {
                        ret = dbtable_hash_delete(npd_mroute_haship_index, &mroute_exist, &mroute_exist);
                        syslog_ax_route_dbg("Delete mroute dip %x, dstvid %d, sip %x, svid %d, mc index %d", 
														mroute_exist.dip, mroute_exist.dst_vid,mroute_exist.sip, 
														mroute_exist.svid, mroute_exist.tbl_index);
                        npd_mroute_nexthop_check_free(&mroute_exist);
                    }			
				}
                break;
            default:
                break;
    	} 
switch_end:		
        dbtable_hash_unlock(npd_mroute_hwid_hash_index);
        npd_mroute_unlock();
		npd_key_database_unlock();
    }       

    return ret;
    
}

int npd_mroute_delete( 
    hash_table_index_t *hash,
    void *data, 
    unsigned int flag)
{
	struct npd_mroute_item_s *delItem = NULL;
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	delItem = (struct npd_mroute_item_s *)data;

    if(delItem->rt_type != RTN_L2MULTICAST)  
    {
        if(delItem->rt_type == RTN_L2L3MULTICAST)
        {
            delItem->rt_type = RTN_L2MULTICAST;
            delItem->svid = delItem->dst_vid;
            delItem->srcl3_g_index = -1;
            delItem->srcl2_netif_index = -1;
            delItem->srcl3_netif_index = -1;
            ret = dbtable_hash_update(npd_mroute_haship_index, NULL, delItem);
			return ret;
        }
    }
    
	ret = dbtable_hash_delete(npd_mroute_haship_index, delItem, delItem);
	npd_mroute_nexthop_check_free(delItem);

	return ret;	
}
#endif


int npd_route_read_route(struct nlmsghdr *nlp, unsigned int srcpid)
{
	int ret;
	int nh_count = 0;
	struct rttbl_info rtInfo ={0};
	struct rtmsg *rtEntry=NULL;
	int payloadoff = 0;
#if 0
	struct npd_route_item_s dbItem;
	struct route_nexthop_brief_s nexthopItem;	
#endif
	rtEntry = (struct rtmsg *) NLMSG_DATA(nlp);
	payloadoff = RTM_PAYLOAD(nlp);

#if 0
	if(srcpid==0&&rtEntry->rtm_protocol!=RTPROT_KERNEL&&rtEntry->rtm_protocol!=RTPROT_BOOT&&rtEntry->rtm_protocol!=RTPROT_ZEBRA)
	{
		syslog_ax_route_dbg("Discard msg pid is %u rtEntry->rtm_protocol is %d",srcpid,rtEntry->rtm_protocol);
		return ROUTE_RETURN_CODE_ERROR;
	}	
	if(srcpid!=0&&rtEntry->rtm_protocol!=RTPROT_ZEBRA)
	{
		syslog_ax_route_dbg("Discard msg pid is %u rtEntry->rtm_protocol is %d",srcpid,rtEntry->rtm_protocol);
		return ROUTE_RETURN_CODE_ERROR;
	}	
#endif

	memset(&rtInfo,0,sizeof(rtInfo));
	rtInfo.ifindex = malloc(sizeof(unsigned int) * NPD_ROUTE_MAX_NH_NUM);
	rtInfo.nexthop = malloc(sizeof(unsigned int) * NPD_ROUTE_MAX_NH_NUM);
	rtInfo.nexthop_flag = malloc(sizeof(unsigned int) * NPD_ROUTE_MAX_NH_NUM);
#ifdef HAVE_NPD_IPV6	
	rtInfo.nexthop6 = malloc(sizeof(ip6_addr) * NPD_ROUTE_MAX_NH_NUM);
#endif //HAVE_NPD_IPV6	
	memset(rtInfo.ifindex,0,(sizeof(unsigned int) * NPD_ROUTE_MAX_NH_NUM));
	memset(rtInfo.nexthop,0,(sizeof(unsigned int) * NPD_ROUTE_MAX_NH_NUM));
	memset(rtInfo.nexthop_flag,0,(sizeof(unsigned int) * NPD_ROUTE_MAX_NH_NUM));
#ifdef HAVE_NPD_IPV6	
	memset(rtInfo.nexthop6,0,(sizeof(ip6_addr) * NPD_ROUTE_MAX_NH_NUM));
#endif //HAVE_NPD_IPV6	
	rtInfo.sifindex = -1;
	ret = get_route_info(&rtInfo,rtEntry,payloadoff);
	
	if(0 != ret)
	{
		syslog_ax_route_dbg("Can't get msg route info continue");
		free(rtInfo.ifindex);
		free(rtInfo.nexthop);
		free(rtInfo.nexthop_flag);
#ifdef HAVE_NPD_IPV6	
		free(rtInfo.nexthop6);
#endif //HAVE_NPD_IPV6	
		return ROUTE_RETURN_CODE_ERROR;
	}
    if(RTN_UNSPEC == rtInfo.rt_type)
    {
        ;
    }
#ifdef HAVE_NPD_IPV6    
    else if (((IPV6_ADDR_LINKLOCAL & ipv6_addr_type(&rtInfo.DIP6)) && 0x40==rtInfo.masklen)
        || ((IPV6_ADDR_MULTICAST & ipv6_addr_type(&rtInfo.DIP6)) && 0x08==rtInfo.masklen))
    {
        /*because linklocal address is same for all interface, so here we don't
                handle such address further. The address is set in HW at nam init phase.*/
        ;
    }
#endif    
    else if(rtInfo.rt_type == RTN_MULTICAST)
    {
#ifdef HAVE_IGMP_SNP		
        npd_mroute_set(nlp, &rtInfo);
#endif
    }
    else
    {
    	for(nh_count=0; nh_count<NPD_ROUTE_MAX_NH_NUM; nh_count++)
    	{
    		if( (0 != nh_count) && 
				(0 == rtInfo.nexthop[nh_count]) && 
#ifdef HAVE_NPD_IPV6				
				(in6_addr_is_zero(&rtInfo.nexthop6[nh_count])) &&
#endif //HAVE_NPD_IPV6				
				(0 == rtInfo.ifindex[nh_count])
                )
    			continue;
    
    		if(AF_INET == rtInfo.rtm_family){
    			ret = npd_route_set_route_v4(nlp, &rtInfo, nh_count);
    		}
#ifdef HAVE_NPD_IPV6
    		else if(AF_INET6 == rtInfo.rtm_family){
				if( in6_addr_cmp( &rtInfo.DIP6, &rtInfo.nexthop6[0])) //Prevent set ipv6 neigh host route
    				ret = npd_route_set_route_v6(nlp, &rtInfo, nh_count);
    		}
#endif //HAVE_NPD_IPV6
    		else{
    			syslog_ax_route_err("Unknow address family %d\r\n", rtInfo.rtm_family);
    		}
    
    	}
    }

	free(rtInfo.ifindex);
    free(rtInfo.nexthop);
	free(rtInfo.nexthop_flag);
#ifdef HAVE_NPD_IPV6
	free(rtInfo.nexthop6);
#endif //HAVE_NPD_IPV6	
	return ret;
}
#endif //HAVE_ROUTE

int npd_route_msg_handle(void *ptr,int totlemsglen,unsigned int srcpid)
{
	struct nlmsghdr *nlp = (struct nlmsghdr *)ptr;

	for(;NLMSG_OK(nlp, totlemsglen);nlp=NLMSG_NEXT(nlp, totlemsglen))
	{
		switch(nlp->nlmsg_type)
		{
#ifdef HAVE_ROUTE			
			case RTM_NEWROUTE:
			case RTM_DELROUTE:
			case RTM_GETROUTE:
				npd_route_read_route(nlp, srcpid);
				break;
#endif				
			case RTM_NEWADDR:
			case RTM_DELADDR:
				npd_route_read_ip(nlp, srcpid);
				break;
			case RTM_NEWLINK:
			case RTM_DELLINK:
				npd_route_read_link(nlp, srcpid);
				break;
			//case RTM_NEWNEIGH:
			//case RTM_DELNEIGH:
			//	syslog_ax_route_dbg("NPD read neighbour inform from kernel\r\n");
				break;
			default:
				break;
		}		
	}
	
	return ROUTE_RETURN_CODE_SUCCESS;
}

void syn_kernelRT_to_drv(int sockfd)
{
	int msglen=0;
	static char buf[BUFLENGTH];
	
	struct iovec iov = { buf, sizeof buf };
	struct sockaddr_nl snl ;
	struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
	
	char* p = buf;
	struct nlmsghdr *nlp = (struct nlmsghdr *)p,*tmp=NULL;
    int ret;
    int nRecvBuf;
  
    nRecvBuf = 512 * NPD_ROUTE_TABLE_SIZE;
    ret = setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
    if (0 != ret)
        syslog_ax_route_err("in func recv_msgbuf_to_drv setsockopt failed, retval:%d\n", ret);

	while(1)
	{
		msglen = recvmsg(sockfd, &msg, 0);
		
		if(!SYS_LOCAL_MODULE_ISMASTERACTIVE) {
	    	continue;
		}
		
		if(msglen <= 0)
		{
			syslog_ax_route_err("recv msg return error\n");
			continue;
		}
		
		tmp = (struct nlmsghdr *) p;
		if(tmp->nlmsg_flags == NLM_F_MULTI && tmp->nlmsg_type == NLMSG_DONE  ) 
		{
			 syslog_ax_route_err("in func recv_msgbuf_to_drv recv msg type is NLMSG_DONE\n");
			 continue;
		}
		
		syslog_ax_route_dbg("route rtnetlink rcv msg type %d len %d\n",nlp->nlmsg_type,msglen);

		npd_route_msg_handle(nlp,msglen,snl.nl_pid);
		//new_addr_gratuitous_arp_solicit(nlp,msglen);
	}
}

#ifdef HAVE_ROUTE
void form_Request_for_get_kRTs(struct netlink_req* req)
{
  	bzero(req, sizeof(req));

  	req->nl.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
  	req->nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
  	req->nl.nlmsg_type = RTM_GETROUTE;

  	req->rt.rtm_family = AF_INET;
  	req->rt.rtm_table = RT_TABLE_MAIN;
}

int send_Request_for_get_kRTs(int sockfd,struct netlink_req* req)
{
	struct msghdr msg;
	struct sockaddr_nl pa;
	struct iovec iov;

  	bzero(&pa, sizeof(pa));
  	pa.nl_family = AF_NETLINK;
  	bzero(&msg, sizeof(msg));
  	msg.msg_name = (void *) &pa;
  	msg.msg_namelen = sizeof(pa);

  	iov.iov_base = (void *) &req->nl;
  	iov.iov_len = req->nl.nlmsg_len;
  	msg.msg_iov = &iov;
  	msg.msg_iovlen = 1;
  	return sendmsg(sockfd, &msg, 0);
}

int write_msg_to_drv(void *ptr,int totlemsglen)
{
#if 0
	struct rttbl_info rtInfo ={0};
	struct rtmsg *rtEntry=NULL;

	int payloadoff = 0;
	struct npd_route_item_s dbItem;
	int status = 0;
	unsigned int ret = 0;
#endif
	struct nlmsghdr *nlp = (struct nlmsghdr *)ptr;

	for(;NLMSG_OK(nlp, totlemsglen);nlp=NLMSG_NEXT(nlp, totlemsglen))
	{
		npd_route_read_route(nlp, 0);
	
	}
	return NPD_SUCCESS;
}

int copy_fib2mvdrv()
{
	int ret = 0;
	int fd;
	int totlemsglen=BUFLENGTH;
	struct netlink_req req;
	struct sockaddr_nl la;
	static char buf[BUFLENGTH] = {0};

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
	if((ret = bind(fd, (struct sockaddr*) &la, sizeof(la))) != 0)
	{
		syslog_ax_route_err("Failed to bind socket when copy fib to asic. Return code: %d\n", ret);
		close(fd);
		return NPD_FAIL;
	}

	form_Request_for_get_kRTs(&req);
	send_Request_for_get_kRTs(fd,&req);
	if(recv_rtMSG_from_kernel(fd,(void*)buf,&totlemsglen))
	{
        syslog_ax_route_err("receive RT Netlink message from kernel error\n");
	    close(fd);
		return NPD_FAIL;
	}
	close(fd);
    init_routesock();

	write_msg_to_drv((void*)buf,totlemsglen);
	return NPD_SUCCESS;
}
#endif

unsigned char syn_fib2mvdrv()
{
	int ret = 0;
	int fd = -1;
	struct sockaddr_nl la;
	
	/* tell my thread id*/
	npd_init_tell_whoami("NpdRoute",0);

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(fd<0)
	{
		 syslog_ax_route_err("create socket error when sync fib to asic\n");
		return NPD_FAIL;
	}
	
	bzero(&la, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_pid = getpid();
	#ifdef CPU_ARM_XCAT
	la.nl_pid = syscall(SYS_gettid);
	#endif
	la.nl_groups = RTMGRP_IPV4_IFADDR|RTMGRP_LINK|RTMGRP_NOTIFY|RTMGRP_IPV6_IFADDR|RTMGRP_NEIGH;
#ifdef HAVE_ROUTE
	la.nl_groups |= RTMGRP_IPV4_ROUTE|RTMGRP_IPV4_MROUTE|RTMGRP_IPV6_ROUTE|RTMGRP_NEIGH;
#endif

	if((ret = bind(fd, (struct sockaddr*) &la, sizeof(la))) != 0)
	{
		syslog_ax_route_err("Failed to bind netlink socket for route, %s\n", strerror(errno));
		return NPD_FAIL;
	}
	syn_kernelRT_to_drv(fd);

	return NPD_SUCCESS;
}

#ifdef HAVE_ROUTE
/**********************************************************************************
 * npd_route_db_op_item
 *
 * route database add/delete or other operations.
 *
 *	INPUT:
 *		item - route DB items
 *		action - add or delete operation
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ROUTE_RETURN_CODE_SUCCESS - if no error occurred
 *		COMMON_RETURN_CODE_NULL_PTR - if input parameters have null pointer
 *		ROUTE_RETURN_CODE_NO_MORE   - the hash table is full
 *		ROUTE_RETURN_CODE_ALREADY_EXIST - static arp item already exists
 *		COMMON_RETURN_CODE_NO_RESOURCE - if no memory allocatable
 *		ROUTE_RETURN_CODE_NO_SUCH - if item not exists in DB
 *		
 *	NOTE:
 *		route database can only be modified by this API.
 *		Input arguments is viewed as temporary memory,when 
 *		add		- all hash backet data is newly allocated here.
 *		delete	- memory is returned back to system(or say FREE) here.
 *
 **********************************************************************************/ 

int npd_route_db_op_item
(
	enum NPD_ROUTE_ACTION action,
	void *data
)
{
	int status;
	struct npd_route_item_s* routePtr;
	struct npd_route_item_v6_s* routev6Ptr;
		
	if(NULL == data) {
		syslog_ax_route_err("npd %s route item null pointer error.",(ROUTE_ADD_ITEM==action) ? "add":"del");
		return COMMON_RETURN_CODE_NULL_PTR;
	}

	switch(action)
	{
		case ROUTE_ADD_ITEM:
			routePtr = (struct npd_route_item_s*)data;
			status = set_LPM_TBL_Entry(	\
						routePtr->DIP,routePtr->nexthop,routePtr->masklen,routePtr->ifindex,routePtr->rt_type,routePtr->flag);								
			if(status)
			{
				syslog_ax_route_err("add entry is wrong,status =%ld\n",status);
				return ROUTE_RETURN_CODE_SET_ERROR;
			}
			break;
		case ROUTE_DEL_ITEM:
			routePtr = (struct npd_route_item_s*)data;
			status = nam_del_route_info(routePtr->DIP,routePtr->nexthop,routePtr->masklen,routePtr->ifindex,routePtr->rt_type,routePtr->flag);		
			if(status)
			{
				 syslog_ax_route_err("del entry is wrong,status =%ld\n",status);
				 return ROUTE_RETURN_CODE_SET_ERROR;
			}
			break;
#ifdef HAVE_NPD_IPV6			
		case ROUTE_ADD_ITEM_V6:
			routev6Ptr = (struct npd_route_item_v6_s*)data;
			status = nam_set_route_v6(routev6Ptr->DIP,routev6Ptr->nexthop,routev6Ptr->masklen,routev6Ptr->ifindex,routev6Ptr->rt_type,routev6Ptr->flag);								
			if(status)
			{
				syslog_ax_route_err("add entry is wrong,status =%ld\n",status);
				return ROUTE_RETURN_CODE_SET_ERROR;
			}
			break;
		case ROUTE_DEL_ITEM_V6:	
			routev6Ptr = (struct npd_route_item_v6_s*)data;
			status = nam_del_route_v6_info(routev6Ptr->DIP,routev6Ptr->nexthop,routev6Ptr->masklen,routev6Ptr->ifindex,routev6Ptr->rt_type,routev6Ptr->flag);		
			if(status)
			{
				 syslog_ax_route_err("del entry is wrong,status =%ld\n",status);
				 return ROUTE_RETURN_CODE_SET_ERROR;
			}
			break;
#endif	//HAVE_NPD_IPV6		
		default:
			break;
	}
	
	return ROUTE_RETURN_CODE_SUCCESS;	
}

int npd_mroute_db_op_item
(
	enum NPD_ROUTE_ACTION action,
	struct npd_mroute_item_s* opItem
)
{
	int status;
	if(NULL == opItem) {
		syslog_ax_route_err("npd %s route item null pointer error.",(ROUTE_ADD_ITEM==action) ? "add":"del");
		return COMMON_RETURN_CODE_NULL_PTR;
	}

	if(ROUTE_ADD_ITEM==action) {	
		//The route get from IP kernel, here suppose No route update operation.
		//Otherwise,  the old route should be deleted before add new route.  --lizheng@autolan.com
		{
			status = nam_add_mroute_info(opItem);								
			if(status)
			{
				syslog_ax_route_err("add entry is wrong,status =%ld\n",status);
				return ROUTE_RETURN_CODE_SET_ERROR;
			}
		}
	}
	else if(ROUTE_DEL_ITEM==action) {	
		status = nam_del_mroute_info(opItem);
		if(status)
		{
			 syslog_ax_route_err("del entry is wrong,status =%ld\n",status);
			 return ROUTE_RETURN_CODE_SET_ERROR;
		}

        
	}
	return ROUTE_RETURN_CODE_SUCCESS;	
}

int npd_mroute_pim_mode_set(int mode)
{	
	struct npd_route_cfg_s cfg;

	dbtable_array_get(npd_route_seqcfg_index, 0, &cfg);
	cfg.multicast_pim_mode = mode;
	dbtable_array_update(npd_route_seqcfg_index, 0, NULL, &cfg);
    return 0;
}


/**********************************************************************************
 * npd_route_compare
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_route_compare
(
	void *data1,
	void *data2
)
{
	unsigned int equal = TRUE;
	struct npd_route_item_s *itemA = (struct npd_route_item_s *)data1;
	struct npd_route_item_s *itemB = (struct npd_route_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->DIP != itemB->DIP)
	{
		equal = FALSE;
        goto retcode;
	}
	else if(itemA->masklen != itemB->masklen) 
	{
		equal = FALSE;
        goto retcode;
	}
	else if(itemA->nexthop != itemB->nexthop)
	{ 
		equal = FALSE;
        goto retcode;
	}
    if(itemA->sifindex != itemB->sifindex)
    {
        equal = FALSE;
        goto retcode;
    }
    if(itemA->sip != itemB->sip)
    {
        equal = FALSE;
         goto retcode;
    }
    if(itemA->ifindex != itemB->ifindex)
    {
        equal = FALSE;
        goto retcode;
    }
    
retcode:
	return equal;

}

/**********************************************************************************
 * npd_route_compare_bynhp
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_route_compare_bynhp
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct npd_route_item_s *itemA = (struct npd_route_item_s *)data1;
	struct npd_route_item_s *itemB = (struct npd_route_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->nexthop != itemB->nexthop)
	{ 
		equal = FALSE;
	}

	return equal;
}

/**********************************************************************************
 * npd_route_cmp_by_ifindex
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_route_cmp_by_ifindex
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct npd_route_item_s *itemA = (struct npd_route_item_s *)data1;
	struct npd_route_item_s *itemB = (struct npd_route_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->ifindex != itemB->ifindex)
	{ 
		equal = FALSE;
	}

	return equal;

}

#ifdef HAVE_CAPWAP_ENGINE
/**********************************************************************************
 * npd_route_cmp_by_network
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_route_cmp_by_network
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct npd_route_item_s *itemA = (struct npd_route_item_s *)data1;
	struct npd_route_item_s *itemB = (struct npd_route_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->DIP != itemB->DIP)
	{ 
		equal = FALSE;
	}
	else if(itemA->masklen != itemB->masklen)
	{
		equal = FALSE;
	}

	return equal;

}

#endif //HAVE_CAPWAP_ENGINE
/**********************************************************************************
 * npd_route_cmp_by_network
 *
 * compare two of route database(Hash table) items
 *
 *	INPUT:
 *		itemA	- route database item
 *		itemB	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal.
 *
 **********************************************************************************/
unsigned int npd_route_cmp_by_nhnetwork
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct npd_route_item_s *itemA = (struct npd_route_item_s *)data1;
	unsigned int *ip_mask = (unsigned int *)data2;

	if((NULL==itemA)||(NULL==ip_mask)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	syslog_ax_route_dbg("npd route cmp: route nh %#x, ip %#x mask %#x\n", itemA->nexthop, ip_mask[0], ip_mask[1]);
	if((itemA->nexthop&ip_mask[1]) != (ip_mask[0]&ip_mask[1]) )
	{ 
		equal = FALSE;
	}

	return equal;

}



/**********************************************************************************
 * npd_route_key_generate
 *
 * route database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_route_key_ip_generate
(
	void *data	
)
{
	unsigned int key = 0;
	struct npd_route_item_s *item = (struct npd_route_item_s *)data;

	if(NULL == item) {
		syslog_ax_route_err("npd arp snooping items make key null pointers error.");
		return ~0UI;
	}
    key = jhash_1word(item->DIP, 0x35798642);
    key %= NPD_ROUTE_HASH_IP_SIZE;

	return key;
}


/**********************************************************************************
 * npd_route_key_nhp_generate
 *
 * route database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- route database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_route_key_nhp_generate
(
	void *data
)
{
	unsigned int key = 0;
	struct npd_route_item_s *item = (struct npd_route_item_s *)data;

	if(NULL == item) {
		syslog_ax_route_err("npd arp snooping items make key null pointers error.");
		return ~0UI;
	}

    key = jhash_1word(item->nexthop, 0x35798642);

	key %= (NPD_ROUTE_HASH_NHP_SIZE);
	
	return key;
}

long npd_route_dbtbl_handle_insert( void *newItem )
{
	int ret = ROUTE_RETURN_CODE_SUCCESS;
	struct npd_route_item_s *opItem = NULL;

	if( newItem == NULL )
		return ROUTE_RETURN_CODE_NULL_PTR;

	opItem = (struct npd_route_item_s *)(newItem);	

	syslog_ax_route_dbg("route table insert: add entry 0x%x, masklen %d, nhp 0x%x\n", opItem->DIP, opItem->masklen, opItem->nexthop);

	if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_ADD_ITEM, opItem) )
	{
		syslog_ax_route_err("Add entry 0x%x is wrong\n", opItem->DIP );
		ret =  ROUTE_RETURN_CODE_ERROR;
	}
		
	return ret;
}

long npd_route_dbtbl_handle_delete( void *delItem )
{
	int ret = ROUTE_RETURN_CODE_SUCCESS;
	struct npd_route_item_s *opItem = NULL;

	if( delItem == NULL )
		return ROUTE_RETURN_CODE_NULL_PTR;

	opItem = (struct npd_route_item_s *)(delItem);	

	syslog_ax_route_dbg("route table delete: del entry 0x%x, masklen %d, nhp 0x%x\n", opItem->DIP, opItem->masklen, opItem->nexthop);
	if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_DEL_ITEM, opItem) )
	{
		syslog_ax_route_err("Del entry 0x%x is wrong\n", opItem->DIP );
		ret =  ROUTE_RETURN_CODE_ERROR;
	}
	
	return ret;
}

long npd_route_dbtbl_handle_update(  void *newItem, void *oldItem )
{
	struct npd_route_item_s *newRoute = NULL;
	struct npd_route_item_s *oldRoute = NULL;

	if( (newItem == NULL) || (oldItem == NULL))
		return ROUTE_RETURN_CODE_NULL_PTR;

	newRoute = (struct npd_route_item_s *)(newItem);	
	oldRoute = (struct npd_route_item_s *)(oldItem);	
	
	syslog_ax_route_dbg("route table update: update entry 0x%x\n", newRoute->DIP);

	if(newRoute->flag != oldRoute->flag)
	{
		if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_DEL_ITEM, oldRoute) )
		{
			syslog_ax_route_err("Del entry 0x%x is wrong\n", oldRoute->DIP );
			return ROUTE_RETURN_CODE_ERROR;
		}	
		if( ROUTE_RETURN_CODE_SUCCESS != npd_route_db_op_item( ROUTE_ADD_ITEM, newRoute) )
		{
			syslog_ax_route_err("Add entry 0x%x is wrong\n", newRoute->DIP );
			return ROUTE_RETURN_CODE_ERROR;
		}	
	}
		
	return ROUTE_RETURN_CODE_SUCCESS;
}


int npd_route_cfgtbl_handle_ntoh(void *data)
{
	struct npd_route_cfg_s *routeCfg = (struct npd_route_cfg_s *)data;

	routeCfg->multicast_enable   = ntohl(routeCfg->multicast_enable);
    routeCfg->urpf_strict_enable = ntohl(routeCfg->urpf_strict_enable);
    routeCfg->ipv6_enable        = ntohl(routeCfg->ipv6_enable);

	return 0;
}

int npd_route_cfgtbl_handle_hton(void *data)
{
	struct npd_route_cfg_s *routeCfg = (struct npd_route_cfg_s *)data;

	routeCfg->multicast_enable	 = htonl(routeCfg->multicast_enable);
	routeCfg->urpf_strict_enable = htonl(routeCfg->urpf_strict_enable);
	routeCfg->ipv6_enable		 = htonl(routeCfg->ipv6_enable);

	return 0;
}


long npd_route_cfgtbl_handle_update(  void *newItem, void *oldItem )
{
	struct npd_route_cfg_s *newCfg = (struct npd_route_cfg_s *)newItem;
	struct npd_route_cfg_s *oldCfg = (struct npd_route_cfg_s *)oldItem;
	
	if(newCfg->multicast_pim_mode != oldCfg->multicast_pim_mode)
	{
		/*for Bi-direct PIM, the source vlan check need to be forbidden.*/
		//nam_ipmc_source_vlan_check(newCfg->multicast_source_check);
	}
	
    return 0;
}
long npd_route_cfgtbl_handle_insert(  void *newItem)
{
	//struct npd_route_cfg_s *newCfg = (struct npd_route_cfg_s *)newItem;
	
	/*for Bi-direct PIM, the source vlan check need to be forbidden.*/
	//nam_ipmc_source_vlan_check(newCfg->multicast_source_check);
	
    return 0;
}

int npd_route_update_urpf(hash_table_index_t *hash, void *data,unsigned int flag)
{
	struct npd_route_item_s *item = NULL;
	int ret = 0;

	if( data == NULL )
		return ROUTE_RETURN_CODE_ERROR;

	item = (struct npd_route_item_s *)data;
	
	syslog_ax_route_dbg("update Route urpf: ip 0x%x masklen %d nexthop 0x%x flag 0x%x current flag %d\n", item->DIP,\
													item->masklen, item->nexthop, item->flag, flag);

	if(URPF_STRICT_ALL == flag)
		item->flag |= NPD_ROUTE_URPF;
    else if(URPF_STRICT_EXCLUDE_DEFAULT == flag)
    {
        if(item->DIP != 0)
		    item->flag |= NPD_ROUTE_URPF;
    }
	else
		item->flag &= ~NPD_ROUTE_URPF;

	ret = dbtable_hash_update(npd_route_hashnhp_index, item, item);

 	return ret;
}

int npd_route_traversal_by_ifindex_update_flags(int flag)
{
	return dbtable_hash_traversal(npd_route_hashnhp_index, flag, NULL, NULL, npd_route_update_urpf);
}

#if 0
int npd_route_dbtble_app_handle_update(struct npd_route_ipaddr_s* newitem, struct npd_route_ipaddr_s* preitem)
{
    int i;
	int ret;

	syslog_ax_route_dbg("npd_route_dbtble_app_handle_update");
    for(i = 0; i < 8; i++)
    {
        syslog_ax_route_dbg("ifindex 0x%x ipaddr %d.%d.%d.%d mask %d.%d.%d.%d \n",\
    				  preitem->ifindex,
                		(preitem->ipaddr[i]>>24)&0xff,(preitem->ipaddr[i]>>16)&0xff,(preitem->ipaddr[i]>>8)&0xff,preitem->ipaddr[i]&0xff,\
                		(preitem->ipmask[i]>>24)&0xff,(preitem->ipmask[i]>>16)&0xff,(preitem->ipmask[i]>>8)&0xff,preitem->ipmask[i]&0xff);

        syslog_ax_route_dbg("ifindex 0x%x ipaddr %d.%d.%d.%d mask %d.%d.%d.%d \n",\
    				  newitem->ifindex,
                		(newitem->ipaddr[i]>>24)&0xff,(newitem->ipaddr[i]>>16)&0xff,(newitem->ipaddr[i]>>8)&0xff,newitem->ipaddr[i]&0xff,\
                		(newitem->ipmask[i]>>24)&0xff,(newitem->ipmask[i]>>16)&0xff,(newitem->ipmask[i]>>8)&0xff,newitem->ipmask[i]&0xff);
    	if((0 != newitem->ipaddr[i])&&(preitem->ipaddr[i] != newitem->ipaddr[i]))
    	{
    	    syslog_ax_route_dbg("npd_route_dbtble_app_handle_update: set ip ");
    		ret = npd_intf_set_ip_addr_kap_handler(newitem->ifindex, newitem->ipaddr[i],newitem->ipmask[i]);
			if(1 == ret)
			{
				return 0;
			}
			break;
    	}
    }
    return -1;
}

long npd_route_dbtble_handle_insert(struct npd_route_ipaddr_s* item)
{
    int ret = 0;
	
	syslog_ax_route_dbg("npd_route_dbtble_handle_insert");
    ret = npd_intf_set_ip_addr_kap_handler(item->ifindex, item->ipaddr[0],item->ipmask[0]);
	if(1 == ret)
	{
		return 0;
	}
    return -1;
}
#endif

int npd_route_dbtbl_handle_ntoh(void *data)
{	
	struct npd_route_item_s *routeEntry = (struct npd_route_item_s *)data;
	routeEntry->DIP = ntohl(routeEntry->DIP);
	routeEntry->nexthop = ntohl(routeEntry->nexthop);
	routeEntry->masklen = ntohl(routeEntry->masklen);
	routeEntry->ifindex = ntohl(routeEntry->ifindex);
	routeEntry->tblIndex = ntohl(routeEntry->tblIndex);
	routeEntry->rt_type = ntohl(routeEntry->rt_type);
	routeEntry->flag = ntohl(routeEntry->flag);

	return 0;
}
int npd_route_dbtbl_handle_hton(void *data)
{	
	struct npd_route_item_s *routeEntry = (struct npd_route_item_s *)data;
	routeEntry->DIP = htonl(routeEntry->DIP);
    routeEntry->sip = htonl(routeEntry->sip);
    routeEntry->sifindex = htonl(routeEntry->sifindex);
	routeEntry->nexthop = htonl(routeEntry->nexthop);
	routeEntry->masklen = htonl(routeEntry->masklen);
	routeEntry->ifindex = htonl(routeEntry->ifindex);
	routeEntry->tblIndex = htonl(routeEntry->tblIndex);
	routeEntry->rt_type = htonl(routeEntry->rt_type);
	routeEntry->flag = htonl(routeEntry->flag);

	return 0;
}

#endif
void npd_mroute_ipmap2mac(unsigned int family, unsigned char *dmip, char *mc_addr)
{
    if (family == AF_INET)
    {
        unsigned char mac[6] = {0x01,0x00,0x5e,0x7f,0xff,0xff};

        memcpy(mc_addr, mac, 6); 
    	
    	*(mc_addr+3) &= *(dmip+1);
    	*(mc_addr+4) &= *(dmip+2);
    	*(mc_addr+5) &= *(dmip+3);
    }
#ifdef HAVE_NPD_IPV6
    else if (family == AF_INET6)
    {
        unsigned char mac[6] = {0x33,0x33,0x00,0x00,0x00,0x00};

        memcpy(mc_addr, mac, 6); 

        *(mc_addr+2) = *(dmip+12);
    	*(mc_addr+3) = *(dmip+13);
    	*(mc_addr+4) = *(dmip+14);
    	*(mc_addr+5) = *(dmip+15);
    }
#endif
}


#ifdef HAVE_IGMP_SNP
unsigned int npd_l2mc_member_set
(
    char isset,
	unsigned int eth_g_index,
	unsigned short vidx,
	unsigned int group_ip,
	unsigned short vlanId
)
{
    int all_port;
    unsigned int port_index;
    int type;
    struct trunk_s node = {0};
    unsigned int trunk_id;
    
    type = npd_netif_type_get(eth_g_index);
    if(NPD_NETIF_TRUNK_TYPE == type)
    {
        if(isset)
        {
            nam_asic_l2mc_member_add(eth_g_index, vidx, group_ip, vlanId);
        }
        else
        {
            nam_asic_l2mc_member_del(eth_g_index, vidx, group_ip, vlanId);
        }
		
        trunk_id = npd_netif_trunk_get_tid(eth_g_index);
        npd_find_trunk(trunk_id, &node);

        NPD_PBMP_ITER(node.ports, all_port)
        {
            port_index = netif_array_index_to_ifindex(all_port);
            if(isset)
            {
                nam_asic_l2mc_member_add(port_index, vidx, group_ip, vlanId);
            }
            else
            {
                nam_asic_l2mc_member_del(port_index, vidx, group_ip, vlanId);
            }
        }
    }
    else 
    {
        if(isset)
        {
            nam_asic_l2mc_member_add(eth_g_index, vidx, group_ip, vlanId);
        }
        else
        {
            nam_asic_l2mc_member_del(eth_g_index, vidx, group_ip, vlanId);
        }
    }
    return 0;
}

#ifdef HAVE_NPD_IPV6
unsigned int npd_l2mc_v6member_set
(
    char isset,
	unsigned int eth_g_index,
	unsigned short vidx,
	ip6_addr group_ip,
	unsigned short vlanId
)
{
    int all_port;
    unsigned int port_index;
    int type;
    struct trunk_s node = {0};
    unsigned int trunk_id;
    
    type = npd_netif_type_get(eth_g_index);
    if(NPD_NETIF_TRUNK_TYPE == type)
    {
        trunk_id = npd_netif_trunk_get_tid(eth_g_index);
        npd_find_trunk(trunk_id, &node);

        NPD_PBMP_ITER(node.ports, all_port)
        {
            port_index = netif_array_index_to_ifindex(all_port);
            if(isset)
            {
                nam_asic_l2mc_v6member_add(port_index, vidx, group_ip, vlanId);
            }
            else
            {
                nam_asic_l2mc_v6member_del(port_index, vidx, group_ip, vlanId);
            }
        }
    }
    else 
    {
        if(isset)
        {
            nam_asic_l2mc_v6member_add(eth_g_index, vidx, group_ip, vlanId);
        }
        else
        {
            nam_asic_l2mc_v6member_del(eth_g_index, vidx, group_ip, vlanId);
        }
    }
    return 0;
}

#endif

unsigned int npd_mroute_compar_ip
(
	void *data1,
	void *data2
)
{
	unsigned int equal = TRUE;
	struct npd_mroute_item_s *itemA = (struct npd_mroute_item_s *)data1;
	struct npd_mroute_item_s *itemB = (struct npd_mroute_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->dip != itemB->dip)
	{
		equal = FALSE;
        goto retcode;
	}

	if(itemA->sip != itemB->sip)
	{
		equal = FALSE;
		goto retcode;
	}

#ifdef HAVE_NPD_IPV6
    if(!IPV6_ADDR_SAME(&itemA->dipv6, &itemB->dipv6))
    {
		equal = FALSE;
		goto retcode;
    }
    if(!IPV6_ADDR_SAME(&itemA->sipv6, &itemB->sipv6))
    {
		equal = FALSE;
		goto retcode;
    }
#endif
    
retcode:
	return equal;
    
}

unsigned int npd_mroute_compar_ip_vlan
(
	void *data1,
	void *data2
)
{
	int equal = TRUE;
	struct npd_mroute_item_s *itemA = (struct npd_mroute_item_s *)data1;
	struct npd_mroute_item_s *itemB = (struct npd_mroute_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->dip != itemB->dip)
	{
		equal = FALSE;
        goto retcode;
	}

    if(itemA->dstl3_netif_index != itemB->dstl3_netif_index)
    {
        equal = FALSE;
        goto retcode;
    }

#ifdef HAVE_NPD_IPV6
    if(!IPV6_ADDR_SAME(&itemA->dipv6, &itemB->dipv6))
    {
        equal = FALSE;
        goto retcode;
    }
#endif

retcode:
	return equal;
    
}

unsigned int npd_mroute_cmp_by_dstl3_ifindex(void *data1, void *data2)
{
	int equal = TRUE;
	struct npd_mroute_item_s *itemA = (struct npd_mroute_item_s *)data1;
	struct npd_mroute_item_s *itemB = (struct npd_mroute_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

    if(itemA->dstl3_netif_index != itemB->dstl3_netif_index)
    {
        equal = FALSE;
        goto retcode;
    }

    
retcode:
	return equal;
}

unsigned int npd_mroute_compare
(
	void *data1,
	void *data2
)
{
	unsigned int equal = TRUE;
	struct npd_mroute_item_s *itemA = (struct npd_mroute_item_s *)data1;
	struct npd_mroute_item_s *itemB = (struct npd_mroute_item_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		syslog_ax_route_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if(itemA->dip != itemB->dip)
	{
		equal = FALSE;
        goto retcode;
	}

    if(itemA->dstl3_netif_index != itemB->dstl3_netif_index)
    {
        equal = FALSE;
    }

    if(itemA->sip != itemB->sip)
    {
        equal = FALSE;
    }

#ifdef HAVE_NPD_IPV6
    if (!IPV6_ADDR_SAME(&itemA->dipv6, &itemB->dipv6))
    {
        equal = FALSE;
    }

    if (!IPV6_ADDR_SAME(&itemA->sipv6, &itemB->sipv6))
    {
        equal = FALSE;
    }    
#endif
    
retcode:
	return equal;

}
unsigned int npd_mroute_key_ip_generate
(
	void *data	
)
{
	unsigned int key = 0;
	struct npd_mroute_item_s *item = (struct npd_mroute_item_s *)data;

	if(NULL == item) {
		syslog_ax_route_err("npd arp snooping items make key null pointers error.");
		return ~0UI;
	}
    key = jhash_1word(item->dip, 0x35798642);
#ifdef HAVE_NPD_IPV6
    if (item->dip == 0)
        key = jhash_1word(item->dipv6.u6_addr32[3], 0x35798642);
#endif
    key %= NPD_ROUTE_HASH_IP_SIZE;

	return key;
}

/**********************************************************************************
 * npd_mroute_table_show
 *
 * show mroute database(Hash table) items
 *
 *	INPUT:
 *		entry: if entry == NULL, print header
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1: no space 
 *             0: ok, no output but need continued;
 *           >0: output length
 *
 **********************************************************************************/
int npd_mroute_table_show(void* entry, char *buf, int buf_len)
{
	struct npd_mroute_item_s *item = (struct npd_mroute_item_s*)entry;
	int len = 0;
	char tmp_buf[2048] = {0};
	int port;
	int num = 0;
	char *current = buf;
	int max_len = buf_len;

	if(NULL == entry)
	{
		len = sprintf(buf, "-------- --------------- ----------------- ---- ------ ------ ----\n");
		return len;
	}
	
	len = sprintf(tmp_buf, "dip %x sip %x dstl3if %x srcl3if %x srcl2if %x\r\n", 
		item->dip, item->sip, item->dstl3_netif_index, item->srcl3_netif_index,
		item->srcl2_netif_index);
	if(len < max_len)
	{
		sprintf(current, "%s",tmp_buf);
		current += len;
		max_len -= len;
	}
	else
		return -1;

	len = sprintf(tmp_buf, "ipmcindex %d mcindex %d rttype %x flag %x\r\nportlist:",
		    item->tbl_index, item->l2mc_index, item->rt_type, item->flag);
	if(len < max_len)
	{
		sprintf(current, "%s",tmp_buf);
		current += len;
		max_len -= len;
	}
	else
		return -1;

	
	len = 0;
	NPD_PBMP_ITER(item->dst_l2_ports, port)
	{
		char name[50];
		unsigned int netif_index;

		netif_index = netif_array_index_to_ifindex(port);
		npd_netif_index_to_name(netif_index, (char*)name);
		if(0 == (num % 8))
		{
		    len += sprintf(tmp_buf, "    %s", name);
		}
		else
			len += sprintf(tmp_buf + len, " %s", name);

		num++;
		if(0 == (num % 8))
		{
        	if(len < max_len)
        	{
        		sprintf(current, "%s\r\n",tmp_buf);
        		current += len;
		        max_len -= len;
        	}
			else
				return -1;
			len = 0;
		}
	}
	if (max_len > 2)
		strcat(current, "\r\n");
    return (buf_len - max_len);
	
}

int npd_mroute_dbtbl_handle_ntoh(void *data)
{	
	struct npd_mroute_item_s *routeEntry = (struct npd_mroute_item_s *)data;

	routeEntry->dip = ntohl(routeEntry->dip);
	routeEntry->sip = ntohl(routeEntry->sip);

	routeEntry->svid = ntohl(routeEntry->svid);
	routeEntry->srcl3_g_index = ntohl(routeEntry->srcl3_g_index);
	routeEntry->srcl3_netif_index = ntohl(routeEntry->srcl3_netif_index);
	routeEntry->srcl2_netif_index = ntohl(routeEntry->srcl2_netif_index);
	routeEntry->dst_vid = ntohl(routeEntry->dst_vid);
	routeEntry->dstl3_g_index = ntohl(routeEntry->dstl3_g_index);
	routeEntry->dstl3_netif_index = ntohl(routeEntry->dstl3_netif_index);
	NPD_PBMP_PORT_NTOH(routeEntry->dst_l2_ports);
	routeEntry->tbl_index = ntohl(routeEntry->tbl_index);
	routeEntry->rt_type = ntohl(routeEntry->rt_type);
	routeEntry->flag = ntohl(routeEntry->flag);		

	return 0;
}

int npd_mroute_dbtbl_handle_hton(void *data)
{	
	struct npd_mroute_item_s *routeEntry = (struct npd_mroute_item_s *)data;

	routeEntry->dip = htonl(routeEntry->dip);
	routeEntry->sip = htonl(routeEntry->sip);

	routeEntry->svid = htonl(routeEntry->svid);
	routeEntry->srcl3_g_index = htonl(routeEntry->srcl3_g_index);
	routeEntry->srcl3_netif_index = htonl(routeEntry->srcl3_netif_index);
	routeEntry->srcl2_netif_index = htonl(routeEntry->srcl2_netif_index);
	routeEntry->dst_vid = htonl(routeEntry->dst_vid);
	routeEntry->dstl3_g_index = htonl(routeEntry->dstl3_g_index);
	routeEntry->dstl3_netif_index = htonl(routeEntry->dstl3_netif_index);
	NPD_PBMP_PORT_HTON(routeEntry->dst_l2_ports);
	routeEntry->tbl_index = htonl(routeEntry->tbl_index);
	routeEntry->rt_type = htonl(routeEntry->rt_type);
	routeEntry->flag = htonl(routeEntry->flag); 	

	return 0;
}

long npd_mroute_dbtbl_handle_update(  void *newItem, void *oldItem )
{
	char isset = 0; 
	struct npd_mroute_item_s *newRoute = NULL;
	struct npd_mroute_item_s *oldRoute = NULL;

	if( (newItem == NULL) || (oldItem == NULL))
		return ROUTE_RETURN_CODE_NULL_PTR;

	newRoute = (struct npd_mroute_item_s *)(newItem);	
	oldRoute = (struct npd_mroute_item_s *)(oldItem);	
	
	syslog_ax_route_dbg("route table update: update entry 0x%x\n", newRoute->dip);
    if((newRoute->svid == newRoute->dst_vid) && (newRoute->rt_type == RTN_L2MULTICAST))
    {
        npd_pbmp_t pbmp;
        int array_port;
        if(NPD_PBMP_IS_NULL(newRoute->dst_l2_ports))
        {
			if(newRoute->family == AF_INET)
	            nam_static_fdb_entry_indirect_delete_for_igmp(newRoute->dip,
                        newRoute->svid);
#ifdef HAVE_NPD_IPV6			
			else if(newRoute->family == AF_INET6)
				nam_static_fdb_entry_indirect_delete_for_mld(newRoute->dipv6,
	    			newRoute->svid);
#endif			
        }
		else
		{
			if(NPD_PBMP_IS_NULL(oldRoute->dst_l2_ports))
			{
				if(newRoute->family == AF_INET)
	                nam_static_fdb_entry_indirect_set_for_igmp(newRoute->dip,
                            newRoute->l2mc_index, newRoute->svid, 0);
#ifdef HAVE_NPD_IPV6							
				else if(newRoute->family == AF_INET6)
					nam_static_fdb_entry_indirect_set_for_mld(newRoute->dipv6,
                   			 newRoute->l2mc_index, newRoute->svid, 0);
#endif				
			}
            NPD_PBMP_ASSIGN(pbmp, newRoute->dst_l2_ports);
            if(oldRoute->rt_type == RTN_L2MULTICAST)
                NPD_PBMP_XOR(pbmp, oldRoute->dst_l2_ports);
            NPD_PBMP_ITER(pbmp, array_port)
            {
                int netif_index = netif_array_index_to_ifindex(array_port);
                if(netif_index != -1)
                {
                    if(NPD_PBMP_MEMBER(newRoute->dst_l2_ports, array_port))
                        isset = 1;
                    else
                        isset = 0;

					if(newRoute->family == AF_INET)
						npd_l2mc_member_set(isset, netif_index, newRoute->l2mc_index, 
                              newRoute->dip, newRoute->svid);
#ifdef HAVE_NPD_IPV6												
					else if(newRoute->family == AF_INET6)
						npd_l2mc_v6member_set(isset, netif_index, newRoute->l2mc_index, 
                              newRoute->dipv6, newRoute->svid);
#endif
                }
            }
		}
    }
#ifdef HAVE_ROUTE    
    if((oldRoute->rt_type == RTN_L2MULTICAST)&&(newRoute->rt_type != RTN_L2MULTICAST))
    {
#if 0    
        nam_static_fdb_entry_indirect_set_for_igmp(newRoute->dip,
                    newRoute->l2mc_index, newRoute->svid, 0);
#endif
        nam_add_mroute_info(newRoute);
    }
    else if((oldRoute->rt_type != RTN_L2MULTICAST)&&(newRoute->rt_type == RTN_L2MULTICAST))
    {
		int array_port;
        nam_del_mroute_info(oldRoute);
		
        if(NPD_PBMP_IS_NULL(newRoute->dst_l2_ports))
        {
            nam_static_fdb_entry_indirect_delete_for_igmp(newRoute->dip,
                        newRoute->svid);
        }
		else
		{
            nam_static_fdb_entry_indirect_set_for_igmp(newRoute->dip,
                        newRoute->l2mc_index, newRoute->svid, 0);
            NPD_PBMP_ITER(newRoute->dst_l2_ports, array_port)
            {
                int netif_index = netif_array_index_to_ifindex(array_port);
				if(netif_index != -1)
				{
                    npd_l2mc_member_set(1, netif_index, newRoute->l2mc_index, 
                          newRoute->dip, newRoute->svid);
#if 0
                    nam_asic_l2mc_member_add(netif_index, newRoute->l2mc_index, 
                          newRoute->dip, newRoute->svid);
#endif
				}
            }
		}
    }
    else if(newRoute->rt_type != RTN_L2MULTICAST)
    {
        nam_update_mroute_info(newRoute, oldRoute);
    }
#endif		
	return ROUTE_RETURN_CODE_SUCCESS;
}

long npd_mroute_dbtbl_handle_insert( void *newItem )
{
	struct npd_mroute_item_s *opItem = NULL;
	int l3_exist = TRUE;
	unsigned int dst_l3ifindex;

	if( newItem == NULL )
		return ROUTE_RETURN_CODE_NULL_PTR;

	opItem = (struct npd_mroute_item_s *)(newItem);	

	l3_exist = npd_intf_exist_check(opItem->dstl3_netif_index, &dst_l3ifindex);

	syslog_ax_route_dbg("route table insert: add entry 0x%x\n", opItem->dip);

    if((opItem->rt_type == RTN_L2MULTICAST)||(l3_exist != TRUE))
    {
        int array_port;
		if(opItem->family == AF_INET)
		{
	        nam_static_fdb_entry_indirect_set_for_igmp(opItem->dip,
                    opItem->l2mc_index, opItem->svid, 0);
			NPD_PBMP_ITER(opItem->dst_l2_ports, array_port)
	        { 
	            int netif_index = netif_array_index_to_ifindex(array_port);
				if(netif_index != -1)
				{
	                npd_l2mc_member_set(1, netif_index, opItem->l2mc_index, 
	                              opItem->dip, opItem->svid);
				}
	        }
		}
#ifdef HAVE_NPD_IPV6		
		else if(opItem->family == AF_INET6)
		{
			nam_static_fdb_entry_indirect_set_for_mld(opItem->dipv6,
                    opItem->l2mc_index, opItem->svid, 0);
			NPD_PBMP_ITER(opItem->dst_l2_ports, array_port)
	        { 
	            int netif_index = netif_array_index_to_ifindex(array_port);
				if(netif_index != -1)
				{
	                npd_l2mc_v6member_set(1, netif_index, opItem->l2mc_index, 
	                              opItem->dipv6, opItem->svid);
				}
	        }
		}
#endif
        
    }
#ifdef HAVE_ROUTE    
    else
    {
        nam_add_mroute_info(opItem);
    }
#endif	
	
	return 0;
}

long npd_mroute_dbtbl_handle_delete( void *delItem )
{
	struct npd_mroute_item_s *opItem = NULL;
	int array_port;

	if( delItem == NULL )
		return ROUTE_RETURN_CODE_NULL_PTR;

	opItem = (struct npd_mroute_item_s *)(delItem);	

	if(opItem->family == AF_INET)
	{
		NPD_PBMP_ITER(opItem->dst_l2_ports, array_port)
        { 
            int netif_index = netif_array_index_to_ifindex(array_port);
			if(netif_index != -1)
			{
                npd_l2mc_member_set(0, netif_index, opItem->l2mc_index, 
                              opItem->dip, opItem->svid);
			}
        }
	    nam_static_fdb_entry_indirect_delete_for_igmp(opItem->dip,
                    opItem->svid);
	}
#ifdef HAVE_NPD_IPV6
	else if(opItem->family == AF_INET6)
	{
		NPD_PBMP_ITER(opItem->dst_l2_ports, array_port)
        { 
            int netif_index = netif_array_index_to_ifindex(array_port);
			if(netif_index != -1)
			{
                npd_l2mc_v6member_set(0, netif_index, opItem->l2mc_index, 
                              opItem->dipv6, opItem->svid);
			}
        }
	    nam_static_fdb_entry_indirect_delete_for_mld(opItem->dipv6,
	    			opItem->svid);
	}
#endif
	
#ifdef HAVE_ROUTE 
	if( opItem->rt_type != RTN_L2MULTICAST )
	    nam_del_mroute_info(opItem);
#endif	

	return 0;
}
#endif
#ifdef HAVE_ROUTE
unsigned int npd_route_hwid_nexthop_key_generate(void *data)
{
	unsigned int key = 0;
	struct route_nexthop_hwid_s *item = (struct route_nexthop_hwid_s *)data;

	if(NULL == item) {
		return ~0UI;
	}

    key = jhash_2words(item->ipAddr, item->srcIp, 0x35798642);
#ifdef HAVE_NPD_IPV6
    if (item->ipAddr == 0)
        key = jhash_2words(item->dipv6.u6_addr32[3], item->sipv6.u6_addr32[3], 0x35798642);    
#endif
    key %= NPD_MROUTE_TABLE_SIZE;
	
	return key;
	
}
	
unsigned int npd_route_hwid_nexthop_compare(void *data1, void *data2)
{
	unsigned int equal = TRUE;
	struct route_nexthop_hwid_s *itemA = (struct route_nexthop_hwid_s *)data1;
	struct route_nexthop_hwid_s *itemB = (struct route_nexthop_hwid_s *)data2;

	if((NULL==itemA)||(NULL==itemB)) {
		return FALSE;
	}
		
	if(itemA->ipAddr != itemB->ipAddr) {	/* ip address*/
		equal = FALSE;
	}

	if(itemA->srcIp != itemB->srcIp){
		equal = FALSE;
	}

#ifdef HAVE_NPD_IPV6
    if (!IPV6_ADDR_SAME(&itemA->dipv6, &itemB->dipv6))
    {
        equal = FALSE;
    }

    if (!IPV6_ADDR_SAME(&itemA->sipv6, &itemB->sipv6))
    {
        equal = FALSE;
    }    
#endif

	return equal;
	
}
#endif



int npd_route_table_init()
{
	int ret;

#ifdef HAVE_ROUTE	
	ret = create_dbtable( NPD_ROUTE_HASHTBL_NAME, NPD_ROUTE_TABLE_SIZE, sizeof(struct npd_route_item_s),\
					npd_route_dbtbl_handle_update, 
					NULL,
					npd_route_dbtbl_handle_insert, 
					npd_route_dbtbl_handle_delete,
					NULL,
					NULL, 
					NULL, 
					npd_route_dbtbl_handle_ntoh,
					npd_route_dbtbl_handle_hton,
					DB_SYNC_ALL,
					&(npd_route_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_route_err("create npd route database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("ipv4", npd_route_dbtbl,NPD_ROUTE_HASH_IP_SIZE, npd_route_key_ip_generate,\
													npd_route_compare, &npd_route_haship_index);
	if( 0  != ret )
	{
		syslog_ax_route_err("create npd route hash table fail\n");
		return NPD_FAIL;
	}
	
	ret = dbtable_create_hash_index("next_hopipv4", npd_route_dbtbl,NPD_ROUTE_HASH_NHP_SIZE, npd_route_key_nhp_generate,\
													npd_route_compare, &npd_route_hashnhp_index);
	if( 0  != ret )
	{
		syslog_ax_route_err("create npd route hash table fail\n");
		return NPD_FAIL;
	}	
    ret = create_dbtable( "mipv4hwindex", NPD_MROUTE_TABLE_SIZE, sizeof(struct route_nexthop_hwid_s),\
						NULL, NULL,	NULL, NULL,	NULL, NULL, NULL, NULL, NULL, DB_SYNC_ALL,&(npd_mroute_hwid_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_arpsnooping_err("create npd nexthop database fail\n");
		return NPD_FAIL;
	}
#endif
#ifdef HAVE_ROUTE
	ret = dbtable_create_hash_index("mipv4hwindex", npd_mroute_hwid_dbtbl,NPD_MROUTE_TABLE_SIZE, npd_route_hwid_nexthop_key_generate,\
													npd_route_hwid_nexthop_compare, &npd_mroute_hwid_hash_index);
	if( 0  != ret )
	{
		syslog_ax_arpsnooping_err("create npd nexthop hash table fail\n");
		return NPD_FAIL;
	}	
#endif
#ifdef HAVE_IGMP_SNP
	ret = create_dbtable( NPD_MROUTE_HASHTBL_NAME, (NPD_MROUTE_TABLE_SIZE?NPD_MROUTE_TABLE_SIZE:NPD_IGMP_SNP_TABLE_SIZE), sizeof(struct npd_mroute_item_s),\
					npd_mroute_dbtbl_handle_update, 
					NULL,
					npd_mroute_dbtbl_handle_insert, 
					npd_mroute_dbtbl_handle_delete,
					NULL,
					NULL, 
					NULL, 
					npd_mroute_dbtbl_handle_ntoh,
					npd_mroute_dbtbl_handle_hton,
					DB_SYNC_ALL,
					&(npd_mroute_dbtbl));
	if( 0 != ret )
	{
		syslog_ax_route_err("create npd route database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("mipv4", npd_mroute_dbtbl,NPD_ROUTE_HASH_IP_SIZE, npd_mroute_key_ip_generate,\
													npd_mroute_compare, &npd_mroute_haship_index);
	if( 0  != ret )
	{
		syslog_ax_route_err("create npd mroute hash table fail\n");
		return NPD_FAIL;
	}
	dbtable_table_show_func_install(npd_mroute_dbtbl, npd_mroute_table_show);
#endif

#ifdef HAVE_ROUTE	
#ifdef HAVE_NPD_IPV6
	ret = create_dbtable( "ipv6", NPD_ROUTE_V6_TABLE_SIZE, sizeof(struct npd_route_item_v6_s),\
					npd_route_v6_dbtbl_handle_update, 
					NULL,
					npd_route_v6_dbtbl_handle_insert, 
					npd_route_v6_dbtbl_handle_delete,
					NULL,
					NULL, 
					NULL, 
					NULL,
					NULL,
					DB_SYNC_ALL,
					&npd_route_v6_dbtbl);
	if( 0 != ret )
	{
		syslog_ax_route_err("create npd route database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("ipv6hash", npd_route_v6_dbtbl,NPD_ROUTE_HASH_IP_SIZE, npd_route_key_ip_v6_generate,\
													npd_route_v6_compare, &npd_route_haship6_index);
	if( 0  != ret )
	{
		syslog_ax_route_err("create npd route hash table fail\n");
		return NPD_FAIL;
	}
	
	ret = dbtable_create_hash_index("next_hopipv6", npd_route_v6_dbtbl,NPD_ROUTE_HASH_NHP_SIZE, npd_route_key_nhp_v6_generate,\
													npd_route_v6_compare, &npd_route_hashnhp6_index);
	if( 0  != ret )
	{
		syslog_ax_route_err("create npd route hash table fail\n");
		return NPD_FAIL;
	}

    ret = dbtable_create_hash_index("nh_ifindex_v6", npd_route_v6_dbtbl, NPD_ROUTE_HASH_NHP_SIZE, npd_route_key_ifi_v6_generate,
                                                    npd_route_v6_compare, &npd_route_hashifi6_index);
    if (0 != ret)
    {
        syslog_ax_route_err("Create npd route hash nh-ifindex-v6 index fail.\n");
        return NPD_FAIL;
    }
#endif //HAVE_NPD_IPV6

	ret = create_dbtable( NPD_ROUTE_CFGTBL_NAME, 1, sizeof(struct npd_route_cfg_s),\
					npd_route_cfgtbl_handle_update, 
					NULL,
					npd_route_cfgtbl_handle_insert, 
					NULL,
					NULL,
					NULL, 
					NULL, 
					npd_route_cfgtbl_handle_ntoh,
					npd_route_cfgtbl_handle_hton,
					DB_SYNC_ALL,
					&(npd_route_cfgtbl));

	ret = dbtable_create_array_index("routecfg", npd_route_cfgtbl,&npd_route_seqcfg_index);
	ret = nam_route_init();
	if(0 != ret )
	{
		syslog_ax_route_err("init nam route fail\n");
	}

    struct npd_route_cfg_s cfg = {1, 0, 1};
    dbtable_array_insert_byid(npd_route_seqcfg_index, 0, &cfg); 
#endif
	return NPD_OK;
}

#ifdef HAVE_ROUTE
#ifdef HAVE_IGMP_SNP
int mroute_flag_file_set(struct npd_mroute_item_s *mroute)
{
    char flag_file[50];
	char buf[4] = {0};
	int ret = -1;

    sprintf(flag_file, "/var/run/ipmc/%x.%x.flag", mroute->sip,mroute->dip);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "1"); 
	write_to_file(flag_file, buf, strlen(buf));

	return ret;        
}

/*检测IP 组播路由表项是否有包转发*/
unsigned int npd_mroute_hit(void* param)
{
	struct route_nexthop_hwid_s *nexthop 
                = (struct route_nexthop_hwid_s *)param;
    struct npd_mroute_item_s mroute_exist = {0};

    mroute_exist.dip = nexthop->ipAddr;
    mroute_exist.sip = nexthop->srcIp;
    mroute_flag_file_set(&mroute_exist);
    return 0;
}

#define MAX_HIT_MSG_PER_PACKET 25

typedef struct npd_hit_remote_event_msg
{
    netif_remote_event_hdr msg_hdr;
	struct route_nexthop_hwid_s nexthop;
}npd_hit_remote_event_msg_t;

npd_hit_remote_event_msg_t npd_hit_msg_buffer[MAX_HIT_MSG_PER_PACKET] = {{{{0}}}};
static int hit_msg_num = 0;

int npd_hit_msg_remote_event_send()
{
	int ret = 0;
	if(hit_msg_num > 0)
	{
	    ret = tipc_client_sync_send(PORT_EVENT_NOTIFIER, SYS_MASTER_ACTIVE_SLOT_INDEX + 1, (char *)npd_hit_msg_buffer, hit_msg_num*sizeof(npd_hit_remote_event_msg_t));
		hit_msg_num = 0;
	}
	return ret;
}

int npd_ipmc_hit_remote_event_buffer_insert(struct route_nexthop_hwid_s *nexthop, unsigned int netif_index, int event_code)
{
	npd_hit_remote_event_msg_t *hit_msg = NULL;
	hit_msg_num = hit_msg_num%MAX_HIT_MSG_PER_PACKET;
	hit_msg = &npd_hit_msg_buffer[hit_msg_num];
	
	hit_msg->msg_hdr.nlh.nlmsg_len = sizeof(npd_hit_remote_event_msg_t);
	hit_msg->msg_hdr.nlh.nlmsg_type = TIPC_APP_EVENT_SERVICE;
	hit_msg->msg_hdr.event_code = event_code;
	hit_msg->msg_hdr.netif_index = netif_index;
	hit_msg->msg_hdr.relate_netif_index = 0;
	netif_app_event_hton(&(hit_msg->msg_hdr));
	memcpy(&(hit_msg->nexthop), nexthop, sizeof(struct route_nexthop_hwid_s));
	hit_msg_num++;
	if(hit_msg_num >= MAX_HIT_MSG_PER_PACKET)
	{
		return npd_hit_msg_remote_event_send();
	}
	return 0;
}

int npd_hit_remote_msg_insert
(
    struct route_nexthop_hwid_s *nexthop, unsigned int netif_index
)
{
    return npd_ipmc_hit_remote_event_buffer_insert(nexthop, netif_index, PORT_NOTIFIER_IPMCHIT);
}

unsigned int mroute_detect_hit(void* param)
{
    int ret;
    struct npd_mroute_item_s mroute_exist = {0};
	struct route_nexthop_hwid_s nexthop = {0};

	/* tell my thread id*/
	npd_init_tell_whoami("MrouteDetectHit",0);

    mkdir("/var/run/ipmc", 0777);

    for(;;)
    {
        sleep(30);
        if(!npd_startup_end)
            continue;
        ret = dbtable_hash_head(npd_mroute_hwid_hash_index, NULL, &nexthop, NULL);
        while(0 == ret)
        {
            int mret;
            if(nexthop.srcIp == 0)
            {
                ret = dbtable_hash_next(npd_mroute_hwid_hash_index, &nexthop, &nexthop, NULL);
                continue;
            }
            mroute_exist.dip = nexthop.ipAddr;
            mroute_exist.sip = nexthop.srcIp;
            mret = dbtable_hash_head_key(npd_mroute_haship_index, &mroute_exist, 
                &mroute_exist, npd_mroute_compar_ip);
            if(0 == mret)
            {
                unsigned int src_eth_g_index = -1;
                unsigned char unit, port;
                int result = NPD_SUCCESS;
                int hit_flag = 0;
                
                if(mroute_exist.srcl2_netif_index != -1)
                    src_eth_g_index = mroute_exist.srcl2_netif_index;
                else if(mroute_exist.srcl3_netif_index != -1)
                    src_eth_g_index = mroute_exist.srcl3_netif_index;

                if(src_eth_g_index != -1)
                {
                    if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(src_eth_g_index))
                    {
                        result = npd_get_devport_by_global_index(src_eth_g_index, 
                            &unit, &port);
                    }
                }
                if(NPD_SUCCESS == result)
                {
                    nam_mroute_hit_getandclear(&mroute_exist, &hit_flag);
                    if(hit_flag)
                    {
                        nexthop.hit = 1;
                        if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
                        {
                            npd_hit_remote_msg_insert(&nexthop, mroute_exist.srcl3_netif_index);
                        }
                        else
                        {
                            mroute_flag_file_set(&mroute_exist);
                        }
                    }
                }

            }
            ret = dbtable_hash_next(npd_mroute_hwid_hash_index, &nexthop, &nexthop, NULL);
        }
		npd_hit_msg_remote_event_send();
    }
}

#endif
#endif
int npd_route_init()
{
#ifdef HAVE_ROUTE
	npd_route_table_init();
	copy_fib2mvdrv();
#endif
	nam_thread_create("NpdRoute",(void *)syn_fib2mvdrv,NULL,NPD_TRUE,NPD_FALSE);
#ifdef HAVE_ROUTE
#ifdef HAVE_IGMP_SNP
    nam_thread_create("MrouteDetectHit", (void*)mroute_detect_hit, NULL, NPD_TRUE, NPD_FALSE);
#endif
	register_netif_notifier(&route_netif_notifier);
#endif
	return 0;
}

#ifdef HAVE_ROUTE
#ifdef HAVE_CAPWAP_ENGINE
int npd_route_get_egrintf_by_dip(unsigned int dip, unsigned char *mac, unsigned int *netif_index, unsigned int *vlan_id)
{
	int tmpMask = 0;
	int ret = -1;
	unsigned int netaddr, netmask;
	unsigned int nexthop = dip;
	struct npd_route_item_s routeEntry;
	struct arp_snooping_item_s arpEntry;

	for(tmpMask = 32;tmpMask >= 0;tmpMask--){
		lib_get_mask_from_masklen(tmpMask, &netmask);
		netaddr = dip & netmask;
		ret = npd_route_find_route(netaddr, tmpMask, &routeEntry);
		if(0 == ret)
			break;
	}

	if( 0 != ret ) return ROUTE_RETURN_CODE_NOT_FOUND;
	
	if( routeEntry.nexthop != 0 )
		nexthop = routeEntry.nexthop;

	ret = npd_arp_snooping_find_item_byip(nexthop, &arpEntry);
	if( 0 != ret ) return ROUTE_RETURN_CODE_NOT_FOUND;

	if(mac)
		memcpy(mac, arpEntry.mac, MAC_ADDR_LEN);

	if(netif_index)
		*netif_index = arpEntry.ifIndex;

	if(vlan_id)
		*vlan_id = arpEntry.vid;

	return ROUTE_RETURN_CODE_SUCCESS;
}
#endif //HAVE_CAPWAP_ENGINE

DBusMessage * show_rtdrv_all(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	return NULL;
}


DBusMessage * show_rtdrv_entry(DBusConnection *conn, DBusMessage *msg, void *user_data)
{

	return NULL;
}

DBusMessage * config_ucrpf_enable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int	rpfen=0;
    struct npd_route_cfg_s cfg;
	DBusError 		err;
    int ret = 0;
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&rpfen,
		DBUS_TYPE_INVALID)))
		{		
			 syslog_ax_route_dbg("Unable to get input args\n ");
			if (dbus_error_is_set(&err)) 
			{
				 syslog_ax_route_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
	 syslog_ax_route_dbg("rpfen = %X\n",rpfen);
     dbtable_array_get(npd_route_seqcfg_index, 0, &cfg);
     cfg.urpf_strict_enable = rpfen;
     dbtable_array_update(npd_route_seqcfg_index, 0, NULL, &cfg);
     npd_route_traversal_by_ifindex_update_flags(rpfen);
     reply = dbus_message_new_method_return(msg);
		 
	 dbus_message_iter_init_append (reply, &iter);
		 
	 dbus_message_iter_append_basic (&iter,
							  DBUS_TYPE_UINT32, &ret);

	return reply;
}



DBusMessage * show_ucrpf_enable(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter  iter;
    int ret = 0;
    struct npd_route_cfg_s cfg;
    
     dbtable_array_get(npd_route_seqcfg_index, 0, &cfg);

	
	 reply = dbus_message_new_method_return(msg);
		 
	 dbus_message_iter_init_append (reply, &iter);
		 
	 dbus_message_iter_append_basic (&iter,
							  DBUS_TYPE_UINT32, &ret);
	 dbus_message_iter_append_basic (&iter,
							  DBUS_TYPE_UINT32, &cfg.urpf_strict_enable);

	return reply;
}

DBusMessage *npd_dbus_urpf_show_running(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter  iter;
	char *buffer = NULL;
	unsigned int buffsize = 3000;
    struct npd_route_cfg_s cfg = {0};
    
	buffer = (char*)malloc(3000);
	if(NULL == buffer) {
		return NULL;
	}
	memset(buffer,0,buffsize);

    dbtable_array_get(npd_route_seqcfg_index, 0, &cfg);

    switch(cfg.urpf_strict_enable)
    {
		/*
        case URPF_LOOSE:
            sprintf(buffer, "ip route urpf loose\n");
            break;
            */
        case URPF_STRICT_ALL:
            sprintf(buffer, "ip route urpf strict\n");
            break;
        case URPF_STRICT_EXCLUDE_DEFAULT:
            sprintf(buffer, "ip route urpf strict-exclude-default\n");
            break;
        default:
            break;
    }
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
                                     &buffer); 
								 
	free(buffer);
	buffer = NULL;

	return reply;
}

#endif //HAVE_ROUTE



#ifdef __cplusplus
}
#endif

