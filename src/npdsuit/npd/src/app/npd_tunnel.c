/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_tunnel.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		API used in NPD dbus process for TUNNEL module
*
* DATE:
*		05/01/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.16 $	
*******************************************************************************/
#ifdef HAVE_IP_TUNNEL
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_hash.h"
#include "npd_tunnel.h"

struct Hash  *tunnel_db_s = NULL;
unsigned int tt_table[TT_TABLE_LEN];

#define MAX_PAYLOAD 1024
#ifdef CPU_ARM_XCAT
#define RTNLGRP_IPV4_IFADDR 5 
#endif
#define GRP_ID  (1 << (RTNLGRP_IPV4_IFADDR-1)) 

#define TUNNEL_HASH_SIZE  256/*tunnel_hash_size*/
#define TUNNEL_HASH(addr) ((addr^(addr>>8))&0xFF)/*tunnel_hash*/

extern unsigned int nam_show_fdb_one
(
	NPD_FDB			*fdb,
	unsigned char		macAddr[6],
	unsigned short	vlanId
);

/**********************************************************************************
 * npd_tunnel_compare_byip
 *
 * compare two of tunnel database(Hash table) items, by dstip add srcip
 *
 *	INPUT:
 *		itemA	- tunnel database item
 *		itemB	- tunnel database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 	- if two items are not equal.
 *		1 	- if two items are equal.
 *
 **********************************************************************************/
int npd_tunnel_compare_byip
(
	struct tunnel_item_s *itemA,
	struct tunnel_item_s *itemB
)
{
	if ((NULL== itemA)||(NULL== itemB)) {
		npd_syslog_err("npd tunnel items compare null pointers error.");
		return TUNNEL_RETURN_CODE_NULLPOINTER_2;
	}

	if ((itemA->kmsg.dstip == itemB->kmsg.dstip) &&
		(itemA->kmsg.srcip == itemB->kmsg.srcip)) { 
		return NPD_TRUE;
	}

	return NPD_FALSE;
}

/**********************************************************************************
 * npd_tunnel_compare_byip
 *
 * compare two of tunnel database(Hash table) items, by kernel msg
 *
 *	INPUT:
 *		itemA	- tunnel database item
 *		itemB	- tunnel database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 	- if two items are not equal.
 *		1 	- if two items are equal.
 *
 **********************************************************************************/
 int npd_tunnel_compare_kmsg
(
	struct tunnel_item_s *itemA,
	struct tunnel_item_s *itemB
)
{
	if ((NULL== itemA)||(NULL== itemB)) {
		npd_syslog_err("npd tunnel items compare null pointers error.");
		return TUNNEL_RETURN_CODE_NULLPOINTER_2;
	}

	if (!memcmp(&itemA->kmsg, &itemB->kmsg, sizeof(tunnel_kernel_msg_t))) { 
		return NPD_TRUE;
	}

	return NPD_FALSE;
}

/**********************************************************************************
 * npd_tunnel_compare_byip
 *
 * compare two of tunnel database(Hash table) items, by host ip
 *
 *	INPUT:
 *		itemA	- tunnel database item
 *		itemB	- tunnel database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 	- if two items are not equal.
 *		1 	- if two items are equal.
 *
 **********************************************************************************/
int npd_tunnel_compare_byhostip
(
	struct tunnel_item_s *itemA,
	struct tunnel_item_s *itemB
)
{
	struct tunnel_host_s *hostA = NULL;
	struct tunnel_host_s *hostB = NULL;
	struct list_head         *ptr = NULL;
	if ((NULL == itemA)||(NULL == itemB)) {
		npd_syslog_err("npd tunnel items compare null pointers error.");
		return TUNNEL_RETURN_CODE_NULLPOINTER_2;
	}
	/* find itemB's hostdip and hdiplen*/
	__list_for_each(ptr, &(itemB->list1)) {
		hostB = list_entry(ptr, struct tunnel_host_s, list);
		if (NULL != hostB) {
			break;
		}
	}
	if (NULL == hostB) {
		npd_syslog_dbg("bad itemB, hostB not exist !!\n");
		return NPD_FALSE;
	}
	__list_for_each(ptr, &(itemA->list1)) {
		hostA = list_entry(ptr, struct tunnel_host_s, list);
		if (NULL == hostA) {
			if ((hostA->hostdip == hostB->hostdip) &&
				(hostA->hdiplen == hostB->hdiplen)) {
				return NPD_TRUE;
			}
		}
	}

	return NPD_FALSE;
}

/**********************************************************************************
 * npd_tunnel_key_generate
 *
 * tunnel database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- tunnel database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur
 *
 **********************************************************************************/
 unsigned int npd_tunnel_key_generate
(
	struct tunnel_item_s *item
)
{
	unsigned int key = 0;

	if (NULL == item) {
		npd_syslog_err("npd tunnel items make key null pointers error.");
		return TUNNEL_RETURN_CODE_ERROR;
	}
/*dstip>>16*/
	key = TUNNEL_HASH((item->kmsg.dstip)>>16);
	key += TUNNEL_HASH(item->kmsg.srcip); /* change this beatiful later*/
	key %= (TUNNEL_HASH_SIZE);
	npd_syslog_dbg("get hash tunnel key is %d \n", key);

	return key;
}

/**********************************************************************************
 * npd_tunnel_init
 *
 * tunnel database(Hash table) initialization
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
void npd_tunnel_init
(
	void
)
{
	tunnel_db_s = hash_new(TUNNEL_HASH_SIZE);

	tunnel_db_s->hash_cmp = npd_tunnel_compare_byip;
	tunnel_db_s->hash_key = npd_tunnel_key_generate;
	memset(tt_table, 0, sizeof(unsigned int)*TT_TABLE_LEN);
}

/**********************************************************************************
 * npd_tunnel_release
 *
 *  	release about tunnel start add tunnel terminal res
 *	INPUT:
 *		item:	msg about tunnel start add tunnel terminal
 *		tstten:	release ts tt
 *		rten:	release rt
 *
 *	OUTPUT:
 *		NONE
 *
 * 	RETURN:
 *		NONE
 *
 *	NOTE:
 *
 **********************************************************************************/
void npd_tunnel_release
(
	struct tunnel_item_s *item,
	unsigned int tstten,
	unsigned int rten
)
{
	int ret = TUNNEL_RETURN_CODE_SUCCESS;
	struct tunnel_host_s *host = NULL;
	struct list_head		 *ptr = NULL;
	struct tunnel_item_s *data = item;
	if (NULL == data) {
		npd_syslog_err("item is NULL \n");
	}
	if (tstten) {
		if (data->state) {
			if (TUNNEL_TS_SW_EN & data->state) {
				ret = npd_tunnel_del_ts_tab(data->tsindex);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("del ts index at tunnel release!! \n");
				}
			}
			if (TUNNEL_TS_HW_EN & data->state) {
				ret = nam_tunnel_start_del_entry(data->kmsg.devnum, data->tsindex, TUNNEL_IPV4_OVER_IPV4_E, data->nhindex);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("del ts fail at tunnel release!! \n");
				}
			}
			if (TUNNEL_TT_SW_EN & data->state) {
				ret = npd_tunnel_del_tt_tab(tt_table, sizeof(unsigned int)*TT_TABLE_LEN, (data->ttindex - TUNNEL_TERM_FRIST_NUM));
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("del tt fail at tunnel release!! \n");				
				}
			}
			if (TUNNEL_TT_HW_EN & data->state) {
				ret = nam_tunnel_term_entry_del(data->kmsg.devnum, data->ttindex, TUNNEL_IPV4_OVER_IPV4_E);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("del tt fail at tunnel release!! \n");				
				}
			}			
			if (TUNNEL_NH_SW_EN & data->state) {
				ret = npd_tunnel_del_nexthop_tab(data->nhindex);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("tunnel nh del fail ! \n");
				}
			}
			if (TUNNEL_NH_HW_EN & data->state) {
				ret = nam_tunnel_nh_del(data->kmsg.devnum, data->nhindex);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("tunnel nh del fail ! \n");
				}
			}				
		}
		else {
			npd_syslog_dbg("tunnel ts tt nh is empty \n");
		}
	}	

	if (rten) {
		__list_for_each(ptr, &(data->list1)) {
			host = list_entry(ptr, struct tunnel_host_s, list);
			if (NULL != host) {
				ret = nam_tunnel_del_tcam_ipltt(host->hostdip, host->hdiplen);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("tunnel del tcam and ipltt fail at tunnel release!\n");
				}
				data->hostnum -= 1;
				__list_del((host->list).prev, (host->list).next);
			}
		}
	}
	
	if (tstten & rten) {
		if (!data->hostnum) {
			hash_pull(tunnel_db_s, data);
		}
		else {
			npd_syslog_dbg("tunnel route host num is not");
		}
	}
	
}

/* Improve error handling */
/* !!!!!!! dmac in ts same as arp next hop mac*/
/**********************************************************************************
 * npd_tunnel_handle_tstt_msg
 *
 *  	handle the tunnel msg about tunnel start add tunnel terminal
 *
 *	INPUT:
 *		item:	msg about tunnel start add tunnel terminal
 *	
 *	OUTPUT:
 *		NONE
 *
 * 	RETURN:
 *		TUNNEL_RETURN_CODE_SUCCESS
 *		TUNNEL_RETURN_CODE_ERROR
 *	NOTE:
 *
 **********************************************************************************/
int npd_tunnel_handle_tstt_msg
(
	struct tunnel_kernel_msg_s *item
)
{
	unsigned int ret = TUNNEL_RETURN_CODE_SUCCESS, i = 0;
	unsigned int ts = ~0UI; /* tunnel start table index, valid range [0,1023] */
	unsigned int tt = ~0UI; /* tunnel termination table index, valid range [0,127] */
	unsigned int np = ~0UI; /* next hop table index, valid range [0,4095] */
	struct tunnel_item_s *data = NULL, *checkptr = NULL;
	ETHERADDRS sysmac ;
	
	if (NULL == item) {
		npd_syslog_err("tunnel handle ts tt msg item is NULL \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}

	memset(&sysmac, 0, sizeof(ETHERADDRS));
	data = malloc(sizeof(struct tunnel_item_s));
	if (NULL == data) {
		npd_syslog_err("tunnel item data malloc fail \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}

	memset(data, 0, sizeof(struct tunnel_item_s));
	data->kmsg.dstip = item->dstip;
	data->kmsg.srcip = item->srcip;
	data->kmsg.vid = item->vid;
	data->kmsg.devnum = item->devnum;
	data->kmsg.portnum = item->portnum;
	for (i = 0; i < 6; i++) {
		data->kmsg.mac[i] = item->mac[i];
	}
	
	/* first check item info is in tunnel_db_s yes or not */
	checkptr = hash_search(tunnel_db_s, data, NULL, 0);
	if (NULL != checkptr) {
		if (!memcmp(&checkptr->kmsg, &data->kmsg, sizeof(tunnel_kernel_msg_t))) {
			npd_syslog_dbg("the same netlink msg which recv from kernel, do nothing \n");
		}
		/* update tunnel */
		else {
			npd_syslog_dbg("portnum or some info changed, update tunnel \n");
			npd_tunnel_release(checkptr, 1, 1);
			goto handle_kmsg;
		}		
	}
	else {
	handle_kmsg:	
		INIT_LIST_HEAD(&(data->list1));
		/* 1 get ts index */
		ret = npd_tunnel_get_ts_tab(&ts);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_err("get ts index fail \n");
			goto errRet;
		}

		/* 2 get tt index*/
		ret = npd_tunnel_get_tt_tab(tt_table, sizeof(unsigned int)*TT_TABLE_LEN, &tt);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_err("tt table is full can not get a index \n");
			goto errRet;
		}
		/* used the same space for rt tcam so +=TUNNEL_TERM_FRIST_NUM*/
		tt += TUNNEL_TERM_FRIST_NUM;
		ret = npd_tunnel_get_nexthop_tab(&np);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_err("TUNNEL_RETURN_CODE_NPFULL_8 \n");
			goto errRet;
		}
		
		data->tsindex = ts;
		data->ttindex = tt;
		data->nhindex = np;
		data->state |= TUNNEL_TS_SW_EN;
		data->state |= TUNNEL_NH_SW_EN;
		data->state |= TUNNEL_TT_SW_EN;

		/* 4 hw*/
		ret = nam_tunnel_start_set_entry(data->kmsg.devnum, ts, TUNNEL_IPV4_OVER_IPV4_E, data);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_dbg("ts set entry fail \n");
			/*npd_tunnel_op_item(data, TUNNEL_DEL_ITEM);*/
			goto errRet;
		}
		else {
			data->state |= TUNNEL_TS_HW_EN;
		}

		ret = nam_tunnel_nh_set(data->kmsg.devnum, data);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_dbg("nh set fail \n");
			/*npd_tunnel_op_item(data, TUNNEL_DEL_ITEM);*/
			goto errRet;
		}
		else {
			data->state |= TUNNEL_NH_HW_EN;
		}
		
		npd_tunnel_get_sys_mac(&sysmac);
		memcpy(data->sysmac, sysmac.arEther, 6);
		ret = nam_tunnel_term_entry_set(data->kmsg.devnum, tt, TUNNEL_IPV4_OVER_IPV4_E, data);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_dbg("tt set entry fail \n");
			/*npd_tunnel_op_item(data, TUNNEL_DEL_ITEM);*/
			goto errRet;
		}
		else {
			data->state |= TUNNEL_TT_HW_EN;
		}
		/* add when route enable*/
		ret = nam_tunnel_ipv4_tunnel_term_port_set(data->kmsg.devnum, data->kmsg.portnum, 1);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_dbg("ipv4 tunnel term port set set  fail \n");
			goto errRet;
		}	
		
		hash_push(tunnel_db_s, data); /* wait for perfect  use this api first */  /*insert into hash table*/	
	}
	
	return TUNNEL_RETURN_CODE_SUCCESS;
	
	errRet:
		if (data->state) {
			npd_tunnel_release(data, 1, 0);
		}
		else {
			if (~0UI != ts) {
				/* release tunnel start table index */
				npd_tunnel_del_ts_tab(ts);
			}
			if (~0UI != tt) {
				npd_tunnel_del_tt_tab(tt_table, sizeof(unsigned int)*TT_TABLE_LEN, (tt - TUNNEL_TERM_FRIST_NUM));
			}
			if (~0UI != np) {
				npd_tunnel_del_nexthop_tab(np);
			}
		}
		free(data);
	return TUNNEL_RETURN_CODE_ERROR;
}

/**********************************************************************************
 * npd_tunnel_handle_rt_msg
 *
 *  	handle the tunnel msg about route info
 *
 *	INPUT:
 *		item:	msg about route
 *		dip:		destination ip
 *	
 *	OUTPUT:
 *		NONE
 *
 * 	RETURN:
 *		TUNNEL_RETURN_CODE_SUCCESS
 *		TUNNEL_RETURN_CODE_ERROR
 *	NOTE:
 *
 **********************************************************************************/
int npd_tunnel_handle_rt_msg
(
	struct tunnel_kernel_msg_s *item,
	unsigned int dip
)
{
	int ret = TUNNEL_RETURN_CODE_SUCCESS;
	struct tunnel_item_s *data = NULL, *tmpdata = NULL;
	struct tunnel_host_s *host = NULL, *hosttmp = NULL;
	struct list_head         *ptr = NULL;

	if (NULL == item) {
		npd_syslog_err("tunnel handle rt msg item is NULL \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
	tmpdata = malloc(sizeof(struct tunnel_item_s));
	if (NULL == tmpdata) {
		npd_syslog_err("tmpItem malloc fail \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
	memset(tmpdata, 0, sizeof(struct tunnel_item_s));
	
	host = malloc(sizeof(struct tunnel_host_s));
	if (NULL == host) {
		npd_syslog_err("host malloc fail \n");
		free(tmpdata);
		return TUNNEL_RETURN_CODE_ERROR;
	}
	memset(host, 0, sizeof(struct tunnel_host_s));
	/*test*/
	tmpdata->kmsg.srcip = item->srcip;
	tmpdata->kmsg.dstip = item->dstip;

	data = hash_search(tunnel_db_s, tmpdata, NULL, 0);
	free(tmpdata);
	if(NULL != data) {
		npd_syslog_dbg("nh is %d! \n", data->nhindex);
		__list_for_each(ptr, &(data->list1)) {
			hosttmp = list_entry(ptr, struct tunnel_host_s, list);
			if (hosttmp->hostdip == dip) {
				npd_syslog_dbg("host route exist !! \n");
				free(host);
				return TUNNEL_RETURN_CODE_RT_HOST_EXISTS_9;
			}
		}
		host->hostdip = dip;
		host->hdiplen = 32;
		ret = nam_tunnel_set_tcam_ipltt(data->kmsg.devnum, host->hostdip, host->hdiplen, data);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_err("tunnel set tcam and ipltt fail !\n");
			free(host);
			return TUNNEL_RETURN_CODE_ERROR;
		}
		/*data->state |= TUNNEL_STATES_RT;*/   /*data->hostnum  do the same work*/
		data->hostnum += 1;
		list_add_tail(&(host->list), &(data->list1));
		/*npd_tunnel_update_hashtable(data, tmpItem);*/
	}
	else {
		npd_syslog_dbg("tunnel not exist  !!!\n");
		free(host);
		return TUNNEL_RETURN_CODE_ERROR;
	}
/*
	else {
		INIT_LIST_HEAD(&(tmpItem->list1));
		// add information about tunnel route
//		host->hostdip = item->hostdip;
//		host->hdiplen = item->hdiplen;
		ret = nam_tunnel_set_tcam_ipltt(0, host->hostdip, host->hdiplen, tmpItem);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_err("tunnel set tcam and ipltt fail !\n");
			return TUNNEL_RETURN_CODE_ERROR;
		}
		//data->state |= TUNNEL_STATES_RT;
		data->hostnum += 1;
		list_add_tail(&(host->list), &(tmpItem->list1));
		hash_push(tunnel_db_s, tmpItem);
		npd_syslog_dbg("add new item in tunnel_db_s  for add tunnel route information\n");
		return TUNNEL_RETURN_CODE_SUCCESS;
	}
*/
	return TUNNEL_RETURN_CODE_SUCCESS;

}

/**********************************************************************************
 * npd_tunnel_del_rt_host
 *
 *  	del the tunnel msg about route info
 *
 *	INPUT:
 *		item:	msg about route
 *		dip:		destination ip
 *	
 *	OUTPUT:
 *		NONE
 *
 * 	RETURN:
 *		TUNNEL_RETURN_CODE_SUCCESS
 *		TUNNEL_RETURN_CODE_ERROR
 *	NOTE:
 *
 **********************************************************************************/
int npd_tunnel_del_rt_host
(
	struct tunnel_kernel_msg_s *item,
	unsigned int dip
)
{
	int ret = TUNNEL_RETURN_CODE_SUCCESS;
	struct tunnel_item_s *data = NULL, *tmpdata = NULL;
	struct list_head		 *ptr = NULL;
	struct tunnel_host_s *host = NULL;
	
	if (NULL == item) {
		npd_syslog_err("tunnel del rt msg item is NULL \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}	
	tmpdata = malloc(sizeof(struct tunnel_item_s));
	if (NULL == tmpdata) {
		npd_syslog_err("tmpItem malloc fail \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
	memset(tmpdata,0,sizeof(struct tunnel_item_s));
	
	tmpdata->kmsg.srcip = item->srcip;
	tmpdata->kmsg.dstip = item->dstip;	
	data = hash_search(tunnel_db_s, tmpdata, NULL, 0);
	free(tmpdata);
	if(NULL != data) {
		__list_for_each(ptr, &(data->list1)) {
			host = list_entry(ptr, struct tunnel_host_s, list);
			if ((NULL != host) &&
				(host->hostdip == dip)) {
				ret = nam_tunnel_del_tcam_ipltt(host->hostdip, host->hdiplen);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_err("tunnel set tcam and ipltt fail !\n");
					return TUNNEL_RETURN_CODE_ERROR;
				}
				data->hostnum -= 1;
				__list_del((host->list).prev, (host->list).next);
			}
			else {
				npd_syslog_dbg("tunnel host del fail host ip is 0x%x \n", host->hostdip);
			}
		}
	}
	else {
		npd_syslog_dbg("tunnel not exist host ip dip sip !! \n");
		return TUNNEL_RETURN_CODE_ERROR;
/* what happen ??*/
	}

	return TUNNEL_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_tunnel_del_tsttnp
 *
 *  	del the tunnel msg about tunnel start add tunnel terminal
 *
 *	INPUT:
 *		item:	msg about tunnel start add tunnel terminal
 *	
 *	OUTPUT:
 *		NONE
 *
 * 	RETURN:
 *		TUNNEL_RETURN_CODE_SUCCESS
 *		TUNNEL_RETURN_CODE_ERROR
 *	NOTE:
 *
 **********************************************************************************/
int npd_tunnel_del_tsttnp
(
	struct tunnel_kernel_msg_s *item
)
{
	int ret = TUNNEL_RETURN_CODE_SUCCESS;
	struct tunnel_item_s *data = NULL, *tmpdata = NULL;
	struct tunnel_host_s *host = NULL;
	struct list_head		 *ptr = NULL;
	
	if (NULL == item) {
		npd_syslog_err("tunnel del ts tt msg item is NULL \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}	
	tmpdata = malloc(sizeof(struct tunnel_item_s));
	if (NULL == tmpdata) {
		npd_syslog_err("tmpItem malloc fail \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
	memset(tmpdata,0,sizeof(struct tunnel_item_s));
/*	del it in anther func 	
	host = malloc(sizeof(struct tunnel_host_s));
	if (NULL == host) {
		npd_syslog_err("host malloc fail \n");
		free(checkdata);
		return TUNNEL_RETURN_CODE_ERROR;
	}
	memset(host, 0, sizeof(struct tunnel_host_s));
*/
	
	tmpdata->kmsg.dstip = item->dstip;
	tmpdata->kmsg.srcip = item->srcip;

	data = hash_search(tunnel_db_s, tmpdata, NULL, 0);
	free(tmpdata);/*no use later*/
	if (NULL != data) {
		/*frist del host , del tunnel later*/
		if (0 == data->hostnum) {
			ret = nam_tunnel_start_del_entry(0, data->tsindex, 0);
			if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
				npd_syslog_err("tunnel start del fail ! \n");
			}
			/*nam_tunnel_term_entry_invalidate(0, data->ttindex);*/
			ret = nam_tunnel_term_entry_del(0, data->ttindex, 0);
			if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
				npd_syslog_err("tunnel term del fail ! \n");
			}
			ret = nam_tunnel_nh_del(0, data->nhindex);
			if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
				npd_syslog_err("tunnel nh del fail ! \n");
			}
			/* route disable it */
			nam_tunnel_ipv4_tunnel_term_port_set(0, data->kmsg.portnum, 0);
			if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
				npd_syslog_err("tunnel port disable fail ! \n");
			}
			ret = npd_tunnel_del_ts_tab(data->tsindex);
			if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
				npd_syslog_err("tunnel port disable fail ! \n");
			}
			ret = npd_tunnel_del_tt_tab(tt_table, sizeof(unsigned int)*TT_TABLE_LEN, (data->ttindex - TUNNEL_TERM_FRIST_NUM));
			if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
				npd_syslog_err("tunnel port disable fail ! \n");
			}
			ret = npd_tunnel_del_nexthop_tab(data->nhindex);
			if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
				npd_syslog_err("tunnel port disable fail ! \n");
			}

			/*hash_pull(tunnel_db_s, data);*/
			hash_pull(tunnel_db_s, data);
		}
		else {
			npd_syslog_dbg("host ip exist can not remove tunnel information ! \n");
			return TUNNEL_RETURN_CODE_ERROR;
		}
	}
	else {
		npd_syslog_err("not find dstip %dsrcip %d\n");
/* what happen ??*/
	}

	return TUNNEL_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 * npd_tunnel_show_table
 *
 * show tunnel database(Hash table) item detailed info
 *
 *	INPUT:
 *		hash:		tunnel hash item
 *		strbuff:		the buffer of show string
 *		
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *
 **********************************************************************************/
void npd_tunnel_show_table
(
	struct Hash *hash,
	char *strbuff
)
{
	int i = 0,count = 0;
	unsigned char withTitle = 0;
	struct hash_bucket *backet = NULL, *next = NULL;
	struct tunnel_item_s *item = NULL;
	struct tunnel_host_s *host = NULL, *hosttmp = NULL;
	struct list_head         *ptr = NULL;
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;

	showStr = (char*)malloc(102400);
	if(NULL == showStr) {
		npd_syslog_dbg("Malloc failed!\n");
		return NULL;
	}
	memset(showStr, 0, 102400);	
	cursor = showStr;
	
	for(i = 0; i < hash->size; i++) {
		for(backet = hash->index[i]; backet; backet = next) {
			next = backet->next;
			if(count==0) {
				withTitle = 1;
			}
			else {
				withTitle = 0;
			}
			
			item =  backet->data;

			totalLen += sprintf(cursor, "\n---------------------------------------------------------------\n");
			cursor = showStr + totalLen;
			totalLen += sprintf(cursor, "show one tunnel:\n");
			cursor = showStr + totalLen;
			totalLen += sprintf(cursor, "tunnel termination source IP address is %d.%d.%d.%d \n", 
				(item->kmsg.srcip>>24) & 0xFF, (item->kmsg.srcip>>16) & 0xFF,(item->kmsg.srcip>>8) & 0xFF,item->kmsg.srcip & 0xFF);
			cursor = showStr + totalLen;
			totalLen += sprintf(cursor, "tunnel termination destination IP address is %d.%d.%d.%d \n",
				(item->kmsg.dstip>>24) & 0xFF, (item->kmsg.dstip>>16) & 0xFF,(item->kmsg.dstip>>8) & 0xFF,item->kmsg.dstip & 0xFF);
			cursor = showStr + totalLen;
			totalLen += sprintf(cursor, "tunnel termination vlan id is %d \n", item->kmsg.vid);
			cursor = showStr + totalLen;

			__list_for_each(ptr, &(item->list1)) {
				hosttmp = list_entry(ptr, struct tunnel_host_s, list);
				totalLen += sprintf(cursor, "tunnel start host destination IP address is %d.%d.%d.%d \n",
					(hosttmp->hostdip>>24) & 0xFF, (hosttmp->hostdip>>16) & 0xFF,(hosttmp->hostdip>>8) & 0xFF,hosttmp->hostdip & 0xFF);
				cursor = showStr + totalLen;
			}
			totalLen += sprintf(cursor, "\n---------------------------------------------------------------\n");
			cursor = showStr + totalLen;
			count++;
		}
	}
	if(count != hash->count) {
		npd_syslog_dbg("actual count %d not match with hash count %d\n", count, hash->count);
	}
	
	strncpy(strbuff, showStr, strlen(showStr));
	free(showStr);	
}

int npd_tunnel_update_hashtable
(
	struct tunnel_item_s *oldItem,
	struct tunnel_item_s *newItem
)
{
#if 0
	oldItem ->dstIntf.intf.port.devNum  = newItem ->dstIntf.intf.port.devNum;
	oldItem ->dstIntf.intf.port.portNum  = newItem ->dstIntf.intf.port.portNum;
	oldItem->isStatic = newItem->isStatic;
	oldItem->isTagged = newItem->isTagged;
	oldItem->vid = newItem->vid;
	oldItem->vidx = newItem->vidx;
	memcpy(oldItem->mac,newItem->mac,MAC_ADDRESS_LEN);
#endif
	memcpy(oldItem, newItem, (sizeof(struct tunnel_item_s )));

	return TUNNEL_RETURN_CODE_SUCCESS;
}

int npd_tunnel_get_sys_mac
(
	ETHERADDRS *sysMac
)
{
	ETHERADDRS sysmac;

	memset(&sysmac, 0, sizeof(ETHERADDRS));

	parse_mac_addr(PRODUCT_MAC_ADDRESS, &sysmac);
	memcpy(sysMac, &sysmac, sizeof(ETHERADDRS));
	npd_syslog_dbg("%-15s:%02x:%02x:%02x:%02x:%02x:%02x\n","system mac",	\
		sysMac->arEther[0], sysMac->arEther[1], sysMac->arEther[2],	\
		sysMac->arEther[3], sysMac->arEther[4], sysMac->arEther[5]);

	return TUNNEL_RETURN_CODE_SUCCESS;
}

int npd_tunnel_get_ts_tab
(
	unsigned int *index
)
{
	unsigned int Index = 0, ret = TUNNEL_RETURN_CODE_SUCCESS;
	unsigned int i = 0, j = 0;
	for (j = 0; j < TS_MAX_TABLE_NUM; j++) { /* when failed   release the Index*/
		ret = nam_arp_mactbl_index_get(Index);
		if (TUNNEL_RETURN_CODE_SUCCESS == ret) {
			/*magic 3 add new index*/
			for (i = 0; i < 3; i++) {
				Index += 1;
				ret = nam_arp_mactbl_index_get(Index);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					break;
				}
			}
			if ((3 == i) && (TUNNEL_RETURN_CODE_SUCCESS == ret)) {
				break;
			}
			else {
				while(Index%4) {
					Index -= 1;
					nam_arp_free_mactbl_index(Index);
				}
				Index += 4;
			}
		}
		else {
			Index += 4;
		}
	}
	if ((TS_MAX_TABLE_NUM == j) && (TUNNEL_RETURN_CODE_SUCCESS != ret)) {
		npd_syslog_dbg("there is no ts table index to get \n");
		return TUNNEL_RETURN_CODE_TSFULL_6;
	}
	/*Index = Index /4;*/
	Index = (Index - 3)/4;
	npd_syslog_dbg("get ts index is %d \n", Index);
	*index = Index;
	
	return TUNNEL_RETURN_CODE_SUCCESS;
}

int npd_tunnel_del_ts_tab
(
	unsigned int index
)
{
	unsigned int i = 0, ret = TUNNEL_RETURN_CODE_SUCCESS, arpindex = 0;

	arpindex = index * 4;
	for (i = 0; i < 4; i++) {
		ret = nam_arp_free_mactbl_index(arpindex);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_err("free arp mactal index fail index is %d, ret is %d \n", arpindex, ret);
		}
		arpindex += 1;
	}

	return TUNNEL_RETURN_CODE_SUCCESS;
}

/* len is byte of table*/
int npd_tunnel_get_tt_tab
(
	void *table,
	unsigned int len,
	unsigned int *index
)
{
	unsigned int Index = 0,  i = 0, j = 0;
	unsigned char *data = NULL;
	if ((NULL == table) || (!index)){
		
	}
	data = (unsigned char *)table;
	for (i = 0; i < len; i++) {
		if (~(*(data + i))) {
			for (j = 0; j < 8; j++) {
				if (0 == (*(data + i) & (1<<j))) {
					Index = 8*i + j;
					*(data + i) |= 1<<j;
					npd_syslog_dbg("get tt index is %d \n", Index);
					*index = Index;
					return TUNNEL_RETURN_CODE_SUCCESS;
				}
			}
		}
	}
	
	return TUNNEL_RETURN_CODE_TTFULL_7;
}

int npd_tunnel_del_tt_tab
(
	void *table,
	unsigned int len,
	unsigned int index
)
{
	unsigned int i = 0, j = 0;
	unsigned char *data = NULL;

	data = (unsigned char *)table;
	i = index / 8;
	j = index % 8;

	if (*(data + i) & (1<<j)) {
		*(data + i) &= ~(1<<j);
	}
	else {
		npd_syslog_err("tt table %d empty\n", index);
		return TUNNEL_RETURN_CODE_ERROR;
	}

	return TUNNEL_RETURN_CODE_SUCCESS;
}

int npd_tunnel_get_nexthop_tab
(
	unsigned int *index
)
{
	unsigned int Index = 0, ret = TUNNEL_RETURN_CODE_SUCCESS;
	unsigned int i = 0;

	ret = nam_arp_get_nexthop_tbl_index(&Index);
	if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
		npd_syslog_err("there is no nexthop index to get \n");
		return TUNNEL_RETURN_CODE_NHFULL_8;
	}			

	npd_syslog_dbg("get nh index is %d \n", Index);
	*index = Index;
	
	return TUNNEL_RETURN_CODE_SUCCESS;
}

int npd_tunnel_del_nexthop_tab
(
	unsigned int index
)
{
	unsigned int ret = 0;

	ret = nam_arp_free_nexthop_tbl_index(index);
	if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
		npd_syslog_err("free nexthop index fail index is %d, ret is %d \n", index, ret);
		return TUNNEL_RETURN_CODE_ERROR;
	}

	return TUNNEL_RETURN_CODE_SUCCESS;
}

int npd_tunnel_eth_get_vid
(
	unsigned int eth_g_index,
	unsigned short *vlanid
)
{
	struct eth_port_s *portInfo = NULL;
	struct port_based_vlan_s  *pvid = NULL;
	int ret = TUNNEL_RETURN_CODE_SUCCESS;
#ifdef CORRECT_DB	
	portInfo = npd_get_port_by_index(eth_g_index);
	if(NULL == portInfo) {
		npd_syslog_err("npd eth port %#0x clear pvid port null", eth_g_index);
		return TUNNEL_RETURN_CODE_ERROR;
	}

	pvid = portInfo->funcs.func_data[ETH_PORT_FUNC_PORT_BASED_VLAN];
	if(NULL != pvid) {
		*vlanid = pvid->vid;
	}
	else {
		npd_syslog_err("The vlan set NA msg failed \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
#endif
	return TUNNEL_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_tunnel_recv_netlink_msg
 *
 *  	receive kernel netlink msg for tunnel 
 *
 *	INPUT:
 *		NONE
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NONE
 *		
 *	NOTE:
 *
 **********************************************************************************/
void *npd_tunnel_recv_netlink_msg
(
	void
)
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd = -1, ret = TUNNEL_RETURN_CODE_SUCCESS;
	struct msghdr msg;
	
	/* tell my thread id */
	npd_init_tell_whoami("TunnelNetlink", 0);	
	
	/* Initialize data field */
	memset(&src_addr, 0, sizeof(src_addr));
	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));
	/* Create netlink socket use NETLINK_SELINUX(7) */
	if ((sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
		npd_syslog_err("create netlink socket for tunnel error\n");
		return TUNNEL_RETURN_CODE_ERROR;
	}

	/* Fill in src_addr */
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* Thread method */
#ifdef CPU_ARM_XCAT
	src_addr.nl_pid = syscall(SYS_gettid);
#endif

	src_addr.nl_groups = GRP_ID;
	
	if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
		npd_syslog_err("bind tunnel socket error\n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
	
	/* Fill in dest_addr */
	dest_addr.nl_pid = 0; /* From kernel */
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_groups = GRP_ID;

	/* Initialize buffer */
	if((nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL) {
		npd_syslog_err("malloc tunnel socket buffer error\n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
	
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	while (1) {

		memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

		/* Recv message from kernel */
		recvmsg(sock_fd, &msg, 0);

		/*npd_syslog_dbg("Recv message from kernel: %s\n", NLMSG_DATA(nlh));*/
		ret = npd_tunnel_handle_netlink_msg((struct wifi_nl_msg *)NLMSG_DATA(nlh));
		if (0 != ret) {
			npd_syslog_dbg("npd tunnel handle netlink kernel msg fail \n");
			/* npd_syslog_dbg("kernel msg is: %s \n", NLMSG_DATA(nlh)); */
		} 
	}

	close(sock_fd);
	free(nlh);
	return TUNNEL_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_tunnel_handle_netlink_msg
 *
 *  	handle the netlink msg which for kernel
 *
 *	INPUT:
 *		nl_msg:	struct of wifi_nl_msg
 *	
 *	OUTPUT:
 *		NONE
 *
 * 	RETURN:
 *		TUNNEL_RETURN_CODE_SUCCESS
 *		TUNNEL_RETURN_CODE_ERROR
 *		TUNNEL_RETURN_CODE_DSTIP_NOT_EXISTS_10: can not get arp info from dstip
 *	NOTE:
 *
 **********************************************************************************/
int npd_tunnel_handle_netlink_msg
(
	struct wifi_nl_msg * nl_msg
)
{
	unsigned int ret = TUNNEL_RETURN_CODE_SUCCESS, eth_g_index = 0;
	NPD_FDB	fdb;
	struct tunnel_kernel_msg_s *store = NULL;
	struct arp_snooping_item_s data;

	memset(&fdb, 0, sizeof(NPD_FDB));	
	
	store=(struct tunnel_kernel_msg_s *)malloc(sizeof(struct tunnel_kernel_msg_s));
	if(NULL == store)  {
		npd_syslog_err("npd tunnel handle netlink msg malloc store fail \n");
		return TUNNEL_RETURN_CODE_ERROR;
	}
	memset(store, 0, sizeof(struct tunnel_kernel_msg_s));	

	switch(nl_msg->op) {
		case IP_ADD:
			/* INNER_IP is 1 EXT_IP is 0 */
			if (EXT_IP == nl_msg->type) {
				store->srcip = nl_msg->u.extMsg.sip;
				store->dstip = nl_msg->u.extMsg.dip;
				
				if( 0 == npd_arp_snooping_find_item_byip(store->dstip, &data))
				{
					/**/
					memcpy(store->mac, data.mac, 6);
					store->vid = data.vid;
					npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", store->dstip, store->srcip);
					npd_syslog_dbg("%-15s:%02x:%02x:%02x:%02x:%02x:%02x\n","port mac",	\
									store->mac[0], store->mac[1], store->mac[2],	\
									store->mac[3], store->mac[4], store->mac[5]);
					npd_syslog_dbg("port vid is %d\n", store->vid);
					
					ret = nam_show_fdb_one(&fdb, store->mac, store->vid);
					if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
						npd_syslog_err("use mac add vid get fdb fail !!!\n");
						free(store);
						return TUNNEL_RETURN_CODE_ERROR;
					}
					eth_g_index = fdb.value;
					ret = npd_get_devport_by_global_index(eth_g_index, &(store->devnum), &(store->portnum));
					if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
						npd_syslog_err("npd tunnel handle tstt msg fail !\n");
						free(store);
						return TUNNEL_RETURN_CODE_ERROR;
					}
					npd_syslog_dbg("devnum is %d, portnum is %d \n", store->devnum, store->portnum);
					ret = npd_tunnel_handle_tstt_msg(store);
					if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
						npd_syslog_err("npd tunnel handle tstt msg fail !\n");
						free(store);
						return TUNNEL_RETURN_CODE_ERROR;
					}
				}
				else {
					npd_syslog_dbg("from arp table, check by dstip, data is null \n");
					free(store);
					return TUNNEL_RETURN_CODE_DSTIP_NOT_EXISTS_10;
				}
			}
			else if (INNER_IP == nl_msg->type) {
				store->dstip = nl_msg->u.innerMsg.ext_dip;
				store->srcip = nl_msg->u.innerMsg.ext_sip;
				npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", store->dstip, store->srcip);		
				ret = npd_tunnel_handle_rt_msg(store, nl_msg->u.innerMsg.inner_IP);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_dbg("npd tunnel handle rt msg fail !\n");
					free(store);
					return TUNNEL_RETURN_CODE_ERROR;
				}
			}
			else {
				npd_syslog_dbg("bad netlink msg nl_msg->type is %d \n", nl_msg->type);
				free(store);
				return TUNNEL_RETURN_CODE_ERROR;
			}
			break;
		case IP_DEL:
			/* INNER_IP is 1 EXT_IP is 0  and more*/
			if (EXT_IP == nl_msg->type) {
				store->srcip = nl_msg->u.extMsg.sip;
				store->dstip = nl_msg->u.extMsg.dip;

				npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", store->dstip, store->srcip);

				ret = npd_tunnel_del_tsttnp(store);
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_dbg("npd tunnel del ts tt np fail !\n");
					free(store);
					return TUNNEL_RETURN_CODE_ERROR;
				}
			}
			else if (INNER_IP == nl_msg->type) {
				store->dstip = nl_msg->u.innerMsg.ext_dip;
				store->srcip = nl_msg->u.innerMsg.ext_sip;
				npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", store->dstip, store->srcip);		
				npd_tunnel_del_rt_host(store, nl_msg->u.innerMsg.inner_IP);	
				if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
					npd_syslog_dbg("npd tunnel del rt host fail !\n");
					free(store);
					return TUNNEL_RETURN_CODE_ERROR;
				}
			}
			else {
				npd_syslog_dbg("bad netlink msg nl_msg->type is %d \n", nl_msg->type);
				free(store);
				return TUNNEL_RETURN_CODE_ERROR;
			}
			break;
		default:
			npd_syslog_dbg("bad netlink msg nl_msg->op is %d \n", nl_msg->op);
			break;
	}

	free(store);
	return TUNNEL_RETURN_CODE_SUCCESS;	
}

DBusMessage * npd_dbus_config_ip_tunnel_add
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage*	reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned long	dipaddr = 0, sipaddr = 0;
	unsigned int	sipmaskLen = 0, dipmaskLen = 0, ret = TUNNEL_RETURN_CODE_SUCCESS, eth_g_index = 0;
	unsigned char   mslot = 0, mport = 0;
	struct tunnel_kernel_msg_s *store = NULL;
	struct arp_snooping_item_s data;

	store=(struct tunnel_kernel_msg_s *)malloc(sizeof(struct tunnel_kernel_msg_s));
	if(store == NULL)  {
		ret = TUNNEL_RETURN_CODE_ERROR;
	}
	else {
		memset(store, 0, sizeof(struct tunnel_kernel_msg_s));
		dbus_error_init(&err);

		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &sipmaskLen,
									DBUS_TYPE_UINT32, &dipmaskLen,
									DBUS_TYPE_UINT32, &dipaddr,
									DBUS_TYPE_UINT32, &sipaddr,
									DBUS_TYPE_BYTE,  &mslot,
									DBUS_TYPE_BYTE,  &mport,
									DBUS_TYPE_INVALID))) {
			syslog_ax_acl_err("Unable to get input args ");
			if (dbus_error_is_set(&err)) {
				syslog_ax_acl_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
	
		store->srcip = sipaddr;
		store->dstip = dipaddr;

		eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(mslot, mport);
		npd_get_devport_by_global_index(eth_g_index, &(store->devnum), &(store->portnum));
		npd_syslog_dbg("devnum is %d, portnum is %d \n", store->devnum, store->portnum);
		if( 0 == npd_arp_snooping_find_item_byip(store->dstip,&data))
		{
			memcpy(store->mac, data.mac, 6);
			store->vid = data.vid;
			npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", store->dstip, store->srcip);
			npd_syslog_dbg("%-15s:%02x:%02x:%02x:%02x:%02x:%02x\n","port mac",	\
				store->mac[0], store->mac[1], store->mac[2],	\
				store->mac[3], store->mac[4], store->mac[5]);
			npd_syslog_dbg("port vid is %d\n", store->vid);
			npd_tunnel_handle_tstt_msg(store);
		}
		else {
			ret = 2;
		}
	}	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}

DBusMessage * npd_dbus_config_ip_tunnel_delete
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage*	reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned long	dipaddr = 0, sipaddr = 0;
	unsigned int	sipmaskLen = 0, dipmaskLen = 0, ret = TUNNEL_RETURN_CODE_SUCCESS, eth_g_index = 0;
	unsigned char   mslot = 0, mport = 0;
	struct tunnel_kernel_msg_s *store = NULL;
	struct arp_snooping_item_s *data = NULL;

	store=(struct tunnel_kernel_msg_s *)malloc(sizeof(struct tunnel_kernel_msg_s));
	if(store == NULL)  {
		ret = TUNNEL_RETURN_CODE_ERROR;
	}
	else {
		memset(store, 0, sizeof(struct tunnel_kernel_msg_s));
		dbus_error_init(&err);

		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &sipmaskLen,
									DBUS_TYPE_UINT32, &dipmaskLen,
									DBUS_TYPE_UINT32, &dipaddr,
									DBUS_TYPE_UINT32, &sipaddr,
									DBUS_TYPE_BYTE,  &mslot,
									DBUS_TYPE_BYTE,  &mport,
									DBUS_TYPE_INVALID))) {
			syslog_ax_acl_err("Unable to get input args ");
			if (dbus_error_is_set(&err)) {
				syslog_ax_acl_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}
	
		store->srcip = sipaddr;
		store->dstip = dipaddr;

		npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", store->dstip, store->srcip);

		ret = npd_tunnel_del_tsttnp(store);
		if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
			npd_syslog_dbg("npd tunnel del ts tt np fail !\n");
		}
	
	}	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}

DBusMessage * npd_dbus_config_ip_tunnel_host_add
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage*	reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned long	ripaddr = 0, lipaddr = 0, dipaddr = 0;
	unsigned int	ripmaskLen = 0, lipmaskLen = 0, dipmaskLen = 0, ret = TUNNEL_RETURN_CODE_SUCCESS;
	struct tunnel_kernel_msg_s *item = NULL; /*warning memery not free */

	item=(struct tunnel_kernel_msg_s *)malloc(sizeof(struct tunnel_kernel_msg_s));
	if(item == NULL)  {
		ret = TUNNEL_RETURN_CODE_ERROR;
	}
	else {
		memset(item, 0, sizeof(struct tunnel_kernel_msg_s));
		dbus_error_init(&err);

		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &ripmaskLen,
									DBUS_TYPE_UINT32, &lipmaskLen,
									DBUS_TYPE_UINT32, &dipmaskLen,
									DBUS_TYPE_UINT32, &ripaddr,
									DBUS_TYPE_UINT32, &lipaddr,							
									DBUS_TYPE_UINT32, &dipaddr,
									DBUS_TYPE_INVALID))) {
			syslog_ax_acl_err("Unable to get input args ");
			if (dbus_error_is_set(&err)) {
				syslog_ax_acl_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		item->dstip = ripaddr;
		item->srcip = lipaddr;
		npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", item->dstip, item->srcip);		
		npd_tunnel_handle_rt_msg(item, dipaddr);		
	}	

	free(item);
	item = NULL;
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}

DBusMessage * npd_dbus_config_ip_tunnel_host_delete
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
) 
{
	DBusMessage*	reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned long	ripaddr = 0, lipaddr = 0, dipaddr = 0;
	unsigned int	ripmaskLen = 0, lipmaskLen = 0, dipmaskLen = 0, ret = 0;
	struct tunnel_kernel_msg_s *item = NULL; /*warning memery not free */

	item=(struct tunnel_kernel_msg_s *)malloc(sizeof(struct tunnel_kernel_msg_s));
	if(item == NULL) {
		ret = TUNNEL_RETURN_CODE_ERROR;
	}
	else {
		memset(item, 0, sizeof(struct tunnel_kernel_msg_s));
		dbus_error_init(&err);

		if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT32, &ripmaskLen,
									DBUS_TYPE_UINT32, &lipmaskLen,
									DBUS_TYPE_UINT32, &dipmaskLen,
									DBUS_TYPE_UINT32, &ripaddr,
									DBUS_TYPE_UINT32, &lipaddr,							
									DBUS_TYPE_UINT32, &dipaddr,
									DBUS_TYPE_INVALID))) {
			syslog_ax_acl_err("Unable to get input args ");
			if (dbus_error_is_set(&err)) {
				syslog_ax_acl_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			return NULL;
		}

		item->dstip = ripaddr;
		item->srcip = lipaddr;
		npd_syslog_dbg("TS dstip is 0x%x, srcip is 0x%x \n", item->dstip, item->srcip);		
		npd_tunnel_del_rt_host(item, dipaddr);		
	}	

	free(item);
	item = NULL;
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
	
	return reply;	
}

DBusMessage * npd_dbus_ip_tunnel_show_tab(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	DBusMessage* reply;
	DBusMessageIter  iter;
	unsigned int ret = TUNNEL_RETURN_CODE_SUCCESS;
	unsigned int opDevice = 0;
	unsigned int portnum = 0;
	char *showStr = NULL;
				   
	showStr = (char*)malloc(102400);
	if(NULL == showStr) {
		npd_syslog_dbg("Malloc failed!\n");
		return NULL;
	}
	memset(showStr, 0, 102400);

	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,									
									DBUS_TYPE_UINT32, &opDevice,
									DBUS_TYPE_UINT32, &portnum,
									DBUS_TYPE_INVALID))) {
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
				npd_syslog_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		ret = TUNNEL_RETURN_CODE_ERROR;
	}
	npd_tunnel_show_table(tunnel_db_s, showStr);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &showStr); 
	
	free(showStr);
	showStr = NULL;
	return reply;
}
#ifdef __cplusplus
}
#endif
#endif

#ifdef HAVE_M4_TUNNEL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <linux/if_tunnel.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>

#include <dbus/dbus.h>
#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"

#include "lib/netif_index.h"
#include "util/npd_list.h"
#include "lib/npd_bitop.h"
#include "lib/npd_database.h"
#include "npd/npd_netif_event.h"
#include "npd/nam/npd_amapi.h"
#include "npd/npd_route.h"
#include "npd/npd_tunnel.h"

void npd_tunnel_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char* data,
    int datalen
);

unsigned int npd_interface_tunnel(struct npd_tunnel_s* );


db_table_t* npd_tunnel_db = NULL;
array_table_index_t *npd_tunnel_index = NULL;

netif_event_notifier_t npd_tunnel_notifier =
{
    .netif_event_handle_f = NULL,
    .netif_relate_handle_f = &npd_tunnel_relate_event
};



int npd_tunnel_kernel_dev_op
(
    unsigned int cmd,
    struct npd_tunnel_s* tunnel,
    unsigned char mac[],
    unsigned int* index
)
{
    int fd = -1;
    int err = 0;
    struct ip_tunnel_parm tnl;
    struct ifreq ifr;

    if ((ip_tunnel_none == tunnel->mode)
        || (0 == tunnel->tnl_m_6in4_sip)
        || (0 == tunnel->tnl_m_6in4_dip))
    {
        return -1;
    }

    memset(&tnl, 0, sizeof(tnl));

    tnl.iph.version = 4;
    tnl.iph.ihl = 5;
    tnl.iph.frag_off = htons(IP_DF);
    tnl.iph.tos = 128;
    tnl.iph.protocol = IPPROTO_IPV6;
    tnl.iph.daddr = tunnel->tnl_m_6in4_dip;
    tnl.iph.saddr = tunnel->tnl_m_6in4_sip;
    sprintf(tnl.name, "tunnel%.3d", tunnel->id);

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "sit0");
    ifr.ifr_ifru.ifru_data = (void*)&tnl;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd != -1)
    {
        err = ioctl(fd, cmd, &ifr);
        if ((0 == err) && (SIOCADDTUNNEL == cmd))
        {
            memset(&ifr, 0, sizeof(ifr));
            sprintf(ifr.ifr_name, "tunnel%.3d", tunnel->id);
            err = ioctl(fd, SIOCGIFFLAGS, &ifr);
            if (0 == err)
            {
                if (0 == err)
                {
                    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
                    err = ioctl(fd, SIOCSIFFLAGS, &ifr);
                    if (0 == err)
                    {
                        err = ioctl(fd, SIOCGIFINDEX, &ifr);
                        if ((NULL != index) || (0 == err))
                        {
                            *index = ifr.ifr_ifindex;
                        }
                    }
                }
            }
        }
        close(fd);
    }
    else
    {
        return -1;
    }
    
    return err;
}

int npd_tunnel_operate_by_netif_index
(
    unsigned int cmd,
    unsigned int netif_index,
    unsigned char mac[],
    unsigned int* index
)
{
    struct npd_tunnel_s tunnel;
    int npd_tunnel_search(struct npd_tunnel_s* );

    memset(&tunnel, 0, sizeof(tunnel));

    tunnel.id = npd_netif_tunnel_get_tunnelid(netif_index);
    if (0 == npd_tunnel_search(&tunnel))
    {
        return npd_tunnel_kernel_dev_op(cmd, &tunnel, mac, index);
    }
    else
    {
        return -1;
    }

    return 0;
}

int npd_tnl_create_by_netif_index(unsigned int netif_index, char mac[], unsigned int* local_index)
{
    return npd_tunnel_operate_by_netif_index(SIOCADDTUNNEL, netif_index, mac, local_index);
}

int npd_tnl_delete_by_netif_index(unsigned int netif_index)
{
    return npd_tunnel_operate_by_netif_index(SIOCDELTUNNEL, netif_index, NULL, NULL);
}

int npd_tunnel_valid_nh_count(struct npd_tunnel_s* entry)
{
    int ni = 0;
    int count = 0;
    
    for (ni = 0; ni < NPD_ROUTE_MAX_NH_NUM; ni++)
    {
        if ((0 != entry->nh[ni].nh)
            && (!(entry->nh[ni].flag & TUNNEL_HOST_INVALID)))
        {
            count++;
        }
    }

    return count;
}

int npd_tunnel_handle_update_mode(struct npd_tunnel_s* new_entry, struct npd_tunnel_s* old_entry)
{
    if (NPD_TUNNEL_VALID(old_entry))
    {
        (void)nam_tunnel_delete(old_entry);
    }

    if (ip_6in4_tunnel == new_entry->mode)
    {
        if (NPD_TUNNEL_VALID(new_entry))
        {
            return nam_tunnel_add(new_entry);
        }
    }

    return 0;
}

int npd_tunnel_handle_update_nh(struct npd_tunnel_s* new_entry, struct npd_tunnel_s* old_entry)
{
    int ni = 0;
    int new_valid = 0;
    int old_valid = 0;

    if ((ip_tunnel_none != new_entry->mode)
        && (0 != new_entry->tnl_m_6in4_sip)
        && (0 != new_entry->tnl_m_6in4_dip))
    {
        new_valid = npd_tunnel_valid_nh_count(new_entry);
        old_valid = npd_tunnel_valid_nh_count(old_entry);

        if ((1 == new_valid) && (old_valid > 1))
        {
            npd_route6_update_by_tunnel(old_entry->g_ifindex, 0);
            nam_tunnel_ecmp_op(old_entry->id, 0);    /* destroy multipath */
        }
        else if ((new_valid > 1) && (1 == old_valid))
        {
            npd_route6_update_by_tunnel(new_entry->g_ifindex, 0);
        }
        else if ((0 == new_valid) && (1 == old_valid))
        {
            npd_route6_update_by_tunnel(old_entry->g_ifindex, 0);
        }
        
        for (ni = 0; ni < NPD_ROUTE_MAX_NH_NUM; ni++)
        {
            if ((new_entry->nh[ni].nh != old_entry->nh[ni].nh)
                || (new_entry->nh[ni].flag != old_entry->nh[ni].flag))
            {
                if ((0 != old_entry->nh[ni].nh)
                    && (!(old_entry->nh[ni].flag & TUNNEL_HOST_INVALID)))
                {
                    nam_tunnel_nh_destroy(old_entry->nh[ni].nh, old_entry);
                }
                if ((0 != new_entry->nh[ni].nh)
                    && (!(new_entry->nh[ni].flag & TUNNEL_HOST_INVALID)))
                {
                    nam_tunnel_nh_create(new_entry->nh[ni].nh, new_entry);
                } 
            }
        }

        if ((old_valid > 1) && (1 == new_valid))
        {
            npd_route6_update_by_tunnel(old_entry->g_ifindex, 1);
        }
        else if ((1 == old_valid) && (new_valid > 1))
        {
            nam_tunnel_ecmp_op(new_entry->id, 1);    /* create multipath */
            npd_route6_update_by_tunnel(new_entry->g_ifindex, 1);
        }
        else if ((1 == new_valid) && (0 == old_valid))
        {
            npd_route6_update_by_tunnel(new_entry->g_ifindex, 1);
        }
    }

    return 0;
}

long npd_tunnel_handle_insert(void* data)
{
    struct npd_tunnel_s* entry = (struct npd_tunnel_s*)data;

    if (NPD_TUNNEL_VALID(entry) && NPD_TUNNEL_IF_VALID(entry))
    {
        if (ip_6in4_tunnel == entry->mode)
        {
            nam_tunnel_add(entry);
            nam_tunnel_nh_add(entry);
            if (npd_tunnel_valid_nh_count(entry) > 1)
            {
                nam_tunnel_ecmp_op(entry->id, 1);
            }
            npd_route6_update_by_tunnel(entry->g_ifindex, 1);
            nam_tunnel_record_ifindex(entry->id, entry->g_ifindex);
        }
    }
    
    return 0;
}

long npd_tunnel_handle_delete(void* data)
{
    struct npd_tunnel_s* entry = (struct npd_tunnel_s*)data;

    if (NPD_TUNNEL_VALID(entry) && NPD_TUNNEL_IF_VALID(entry))
    {
        if (ip_6in4_tunnel == entry->mode)
        {
            if (npd_tunnel_valid_nh_count(entry) > 1)
            {
                nam_tunnel_ecmp_op(entry->id, 0);
            }
            nam_tunnel_nh_delete(entry);
            nam_tunnel_delete(entry);
            npd_route6_update_by_tunnel(entry->g_ifindex, 0);
            nam_tunnel_record_ifindex(entry->id, 0);
        }
    }
    
    return 0;
}

long npd_tunnel_handle_update(void* new_data, void* old_data)
{
    int ret = 0;
    struct npd_tunnel_s* new_entry = (struct npd_tunnel_s*)new_data;
    struct npd_tunnel_s* old_entry = (struct npd_tunnel_s*)old_data;

    if (NPD_TUNNEL_IF_VALID(new_entry)
        && (new_entry->g_ifindex == old_entry->g_ifindex))
    {
        if (new_entry->mode != old_entry->mode)
        {
            ret = npd_tunnel_handle_update_mode(new_entry, old_entry);
        }
        else if ((new_entry->tnl_m_6in4_dip != old_entry->tnl_m_6in4_dip)
                || (new_entry->tnl_m_6in4_sip != old_entry->tnl_m_6in4_sip))
        {
            ret = npd_tunnel_handle_delete(old_data);
            ret |= npd_tunnel_handle_insert(new_data);
        }
        else if ((new_entry->nh_count != old_entry->nh_count)
            || (0 != memcmp(new_entry->nh, old_entry->nh, sizeof(new_entry->nh))))
        {
            nam_tunnel_terminator_delete(old_entry);
            ret = npd_tunnel_handle_update_nh(new_entry, old_entry);
            nam_tunnel_terminator_add(new_entry);
        }

        return ret;
    }
    else if (NPD_TUNNEL_IF_VALID(new_entry) && !NPD_TUNNEL_IF_VALID(old_entry))
    {
        return npd_tunnel_handle_insert(new_data);
    }
    else if (!NPD_TUNNEL_IF_VALID(new_entry) && NPD_TUNNEL_IF_VALID(old_entry))
    {
        return npd_tunnel_handle_delete(old_data);
    }
    else
    {
        return 0;
    }

    return 0;
}

int npd_ip_tunnel_init()
{
    int ret;

    ret = create_dbtable("npd_tunnel_db",
                NPD_IP_TUNNEL_SIZE, 
                sizeof(struct npd_tunnel_s),
                npd_tunnel_handle_update, 
                NULL,
                npd_tunnel_handle_insert, 
                npd_tunnel_handle_delete, 
                NULL,
                NULL,
                NULL, 
                NULL,
                NULL,
                DB_SYNC_ALL,
                &(npd_tunnel_db));
    if (0 != ret)
    {
        npd_syslog_err("Create npd ip tunnel db-table failed.\n");
        return NPD_FAIL;
    }

    ret = dbtable_create_array_index("npd_tunnel_index", 
                npd_tunnel_db,  
                &npd_tunnel_index);


    if (0  != ret)
    {
        npd_syslog_err("Create npd ip tunnel index failed.\n");
        return NPD_FAIL;
    }

    nam_tunnel_index_init();
    register_netif_notifier(&npd_tunnel_notifier);

    return 0;
}

int npd_tunnel_search(struct npd_tunnel_s* entry)
{
    return (0 == dbtable_array_get(npd_tunnel_index, entry->id, entry)) ? 0 : -1;
}

int npd_tunnel_insert(struct npd_tunnel_s* entry)
{
    return (0 == dbtable_array_insert_byid(npd_tunnel_index, entry->id, entry)) ? 0 : -1;
}

int npd_tunnel_update(struct npd_tunnel_s* entry)
{
    return (0 == dbtable_array_update(npd_tunnel_index, entry->id, NULL, entry)) ? 0 : -1;
}

int npd_tunnel_delete(struct npd_tunnel_s* entry)
{
    return (0 == dbtable_array_delete(npd_tunnel_index, entry->id, entry)) ? 0 : -1;
}

int npd_tunnel_search_next(struct npd_tunnel_s* entry)
{
    do
    {
        entry->id++;
    } while ((entry->id <= NPD_IP_TUNNEL_SIZE)
        && (0 != npd_tunnel_search(entry)));

    if (entry->id > NPD_IP_TUNNEL_SIZE)
    {
        return -1;
    }

    return 0;
}

unsigned short npd_tunnel_s_vlan_get_by_netif_index(unsigned int netif_index)
{
    struct npd_tunnel_s entry;

    memset(&entry, 0, sizeof(entry));
    
    entry.id = npd_netif_tunnel_get_tunnelid(netif_index);
    if (0 == npd_tunnel_search(&entry))
    {
        return entry.tnl_m_6in4_vlan;
    }
    else
    {
        return -1;
    }

    return -1;
}

void npd_tunnel_get_next_hop_in_arp(struct npd_tunnel_s* tunnel)
{
    struct arp_snooping_item_s arp;

    if (npd_intf_in4_fallin_network(tunnel->tnl_m_6in4_dip) > 0)
    {
        if (0 != npd_arp_snooping_find_item_byip(tunnel->tnl_m_6in4_dip, &arp))
        {
            tunnel->nh[0].flag |= TUNNEL_HOST_INVALID;
        }
        tunnel->nh[0].nh = tunnel->tnl_m_6in4_dip;
        tunnel->nh_count = 1;
        tunnel->nh_is_direct = nh_direct;
        return ;
    }
    
    return ;
}

void npd_tunnel_get_next_hop_in_route(struct npd_tunnel_s* tunnel)
{
    int ni = 0;
    struct arp_snooping_item_s arp;
    unsigned int tmp_nh[NPD_ROUTE_MAX_NH_NUM];
    
    for (ni = 32; ni > 0; ni--)
    {
        tunnel->nh_count = npd_route_next_hop_get_by_network(tunnel->tnl_m_6in4_dip, ni, tmp_nh);
        if (0 != tunnel->nh_count)
        {
            tunnel->lpm = ni;
            for (ni = 0; ni < tunnel->nh_count; ni++)
            {
                if (0 != npd_arp_snooping_find_item_byip(tmp_nh[ni], &arp))
                {
                    tunnel->nh[ni].flag |= TUNNEL_HOST_INVALID;
                }
                tunnel->nh[ni].nh = tmp_nh[ni];
            }
            tunnel->nh_is_direct = nh_network;
            return ;
        }
    }

    return ;
}

void npd_tunnel_get_next_hop(struct npd_tunnel_s* tunnel)
{
    if (nh_network != tunnel->nh_is_direct)
    {
        npd_tunnel_get_next_hop_in_arp(tunnel);
    }

    if (nh_direct != tunnel->nh_is_direct)
    {
        npd_tunnel_get_next_hop_in_route(tunnel);
    }
    
    return ;
}

void npd_tunnel_update_by_arp(unsigned int in4, unsigned int is_add)
{
    int ni = 0;
    int match = 0;
    struct npd_tunnel_s tunnel;

    memset(&tunnel, 0, sizeof(tunnel));
    while (0 == npd_tunnel_search_next(&tunnel))
    {
        if (0 == tunnel.tnl_m_6in4_dip)
        {
            continue;
        }
        
        if (in4 == tunnel.tnl_m_6in4_dip)
        {
            tunnel.nh_count = 1;
            tunnel.nh_is_direct = nh_direct;
            memset(tunnel.nh, 0, sizeof(tunnel.nh));

            tunnel.nh[0].nh = in4;
            if (is_add)
            {
                tunnel.nh[0].flag &= ~TUNNEL_HOST_INVALID;
            }
            else
            {
                tunnel.nh[0].flag |= TUNNEL_HOST_INVALID;
            }

            match = 1;
        }
        else
        {
            for (ni = 0; ni < tunnel.nh_count; ni++)
            {
                if (in4 == tunnel.nh[ni].nh)
                {
                    if (is_add)
                    {
                        tunnel.nh[ni].flag &= ~TUNNEL_HOST_INVALID;
                    }
                    else
                    {
                        tunnel.nh[ni].flag |= TUNNEL_HOST_INVALID;
                    }

                    match = 1;
                }
            }
        }

        if (0 != match)
        {
            if (0 != npd_tunnel_update(&tunnel))
            {
                npd_syslog_err("Tunnel index(%d) update failed.\n", tunnel.id);
            }
        }
    }

    return ;
}

void npd_tunnel_update_by_route_add
(
    unsigned int dst_ip,
    unsigned int mask_len,
    unsigned int nh
)
{
    int ni = 0;
    int match = 0;
    unsigned int mask = 0;
    struct npd_tunnel_s tunnel;
    struct arp_snooping_item_s arp;

    if (0 == nh)
    {
        return ;
    }

    memset(&tunnel, 0, sizeof(tunnel));
    while (0 == npd_tunnel_search_next(&tunnel))
    {
        if (0 == tunnel.tnl_m_6in4_dip)
        {
            continue;
        }

        lib_get_mask_from_masklen(mask_len, &mask);
        if ((tunnel.tnl_m_6in4_dip & mask) == (dst_ip & mask))
        {
            if (mask_len > tunnel.lpm)
            {
                tunnel.lpm = mask_len;
                
                memset(tunnel.nh, 0, sizeof(tunnel.nh));
                tunnel.nh[0].nh = nh;
                if (0 != npd_arp_snooping_find_item_byip(nh, &arp))
                {
                    tunnel.nh[0].flag |= TUNNEL_HOST_INVALID;
                }
                tunnel.nh_count = 1;
                tunnel.nh_is_direct = nh_network;
                match = 1;
            }
            else if (mask_len == tunnel.lpm)
            {
                for (ni = 0; ni < NPD_ROUTE_MAX_NH_NUM; ni++)
                {
                    if (nh == tunnel.nh[ni].nh)
                    {
                        return ;
                    }
                }
                
                for (ni = 0; ni < NPD_ROUTE_MAX_NH_NUM; ni++)
                {
                    if (0 == tunnel.nh[ni].nh)
                    {
                        tunnel.nh[ni].nh = nh;
                        if (0 != npd_arp_snooping_find_item_byip(nh, &arp))
                        {
                            tunnel.nh[ni].flag |= TUNNEL_HOST_INVALID;
                        }

                        tunnel.nh_count++;
                        tunnel.nh_is_direct = nh_network;
                        match = 1;
                        break;
                    }
                }
            }
            else
            {
                continue;
            }

            if (0 != match)
            {
                if (0 != npd_tunnel_update(&tunnel))
                {
                    npd_syslog_err("Tunnel index(%d) update failed.\n", tunnel.id);
                }
            }
        }
    }

    return ;
}

void npd_tunnel_update_by_route_delete
(
    unsigned int dst_ip,
    unsigned int mask_len,
    unsigned int nh
)
{
    int ni = 0;
    unsigned int mask = 0;
    struct npd_tunnel_s tunnel;

    if (0 == nh)
    {
        return ;
    }

    memset(&tunnel, 0, sizeof(tunnel));
    while (0 == npd_tunnel_search_next(&tunnel))
    {
        if (0 == tunnel.tnl_m_6in4_dip)
        {
            continue;
        }

        lib_get_mask_from_masklen(mask_len, &mask);

        if ((tunnel.nh_count > 0)
            && (tunnel.nh_is_direct = nh_network)
            && (tunnel.lpm == mask_len)
            && ((tunnel.tnl_m_6in4_dip & mask) == (dst_ip & mask)))
        {
            for (ni = 0; ni < NPD_ROUTE_MAX_NH_NUM; ni++)
            {
                if (nh == tunnel.nh[ni].nh)
                {
                    tunnel.nh_count--;
                    if (0 == tunnel.nh_count)
                    {
                        tunnel.nh_is_direct = nh_invalid;
                        memset(tunnel.nh, 0, sizeof(tunnel.nh));

                        tunnel.lpm = 0;
                        npd_tunnel_get_next_hop_in_route(&tunnel);
                    }
                    else
                    {
                        tunnel.nh[ni].nh = 0;
                        tunnel.nh[ni].flag = 0;
                    }

                    if (0 != npd_tunnel_update(&tunnel))
                    {
                        npd_syslog_err("Tunnel index(%d) update failed.\n", tunnel.id);
                    }
                    
                    break;
                }
            }
        }
    }

    return ;
}

void npd_tunnel_event_address_add(unsigned int ipaddr)
{
    struct npd_tunnel_s tunnel;

    memset(&tunnel, 0, sizeof(tunnel));
    while (0 == npd_tunnel_search_next(&tunnel))
    {
        if (tunnel.tnl_m_6in4_sip == ipaddr)
        {
            if (0 == tunnel.tnl_sip_valid)
            {
                tunnel.tnl_sip_valid = 1;
                if (0 != npd_tunnel_update(&tunnel))
                {
                    return ;
                }

                if (NPD_TUNNEL_VALID(&tunnel))
                {
                    npd_interface_tunnel(&tunnel);
                }
                
                if (0 != npd_tunnel_update(&tunnel))
                {
                    return ;
                }
            }
        }
    }

    return ;
}

void npd_tunnel_event_address_delete(unsigned int ipaddr)
{
    struct npd_tunnel_s tunnel;
    unsigned int npd_no_interface_tunnel(struct npd_tunnel_s* );

    memset(&tunnel, 0, sizeof(tunnel));
    while (0 == npd_tunnel_search_next(&tunnel))
    {
        if (tunnel.tnl_m_6in4_sip == ipaddr)
        {
            if (0 != tunnel.tnl_sip_valid)
            {
                if (NPD_TUNNEL_VALID(&tunnel) && NPD_TUNNEL_IF_VALID(&tunnel))
                {
                    npd_no_interface_tunnel(&tunnel);
                }

                if (0 != npd_tunnel_update(&tunnel))
                {
                    return ;
                }
                
                tunnel.tnl_sip_valid = 0;
                if (0 != npd_tunnel_update(&tunnel))
                {
                    return ;
                }
            }
        }
    }

    return ;
}

void npd_tunnel_relate_event
(
    unsigned int father_index,
    unsigned int son_ifindex,
    enum PORT_RELATE_ENT event,
    char* data,
    int datalen
)
{
    unsigned int ipaddr = 0;


    switch(event)
    {		
        case PORT_NOTIFIER_ADDR_ADD:
        {
            if (NULL != data)
            {
                ipaddr = *(int*)data;
                npd_tunnel_event_address_add(ipaddr);
            }
            break;
        }
        case PORT_NOTIFIER_ADDR_DEL:
        {
            if (NULL != data)
            {
                ipaddr = *(int*)data;
                npd_tunnel_event_address_delete(ipaddr);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return;
}

int npd_tunnel_get_by_ifindex(struct npd_tunnel_s* tunnel, unsigned int ifindex)
{
    memset(tunnel, 0, sizeof(*tunnel));
    while (0 == npd_tunnel_search_next(tunnel))
    {
        if (tunnel->g_ifindex == ifindex)
        {
            return 0;
        }
    }

    return -1;
}

void npd_tunnel_nh(struct npd_tunnel_s* entry)
{
    if (0 != entry->tnl_m_6in4_dip)
    {
        npd_tunnel_get_next_hop(entry);
    }
    else
    {
        entry->nh_is_direct = nh_invalid;
        entry->nh_count = 0;
        memset(entry->nh, 0, sizeof(entry->nh));
    }

    return ;
}


unsigned int npd_tunnel_confilct_check(struct npd_tunnel_s* entry)
{
    struct npd_tunnel_s tunnel;

    if ((0 == entry->tnl_m_6in4_dip)
        || (0 == entry->tnl_m_6in4_sip)
        || (ip_tunnel_none == entry->mode))
    {
        return 0;
    }

    memset(&tunnel, 0, sizeof(tunnel));
    while (0 == npd_tunnel_search_next(&tunnel))
    {
        if ((tunnel.tnl_m_6in4_dip == entry->tnl_m_6in4_dip)
            && (tunnel.tnl_m_6in4_sip == entry->tnl_m_6in4_sip)
            && (tunnel.mode == entry->mode))
        {
            return 1;
        }
    }

    return 0;
}

unsigned int npd_interface_tunnel(struct npd_tunnel_s* entry)
{
    if (!NPD_TUNNEL_IF_VALID(entry))
    {
        if (0 != npd_l3_intf_create(entry->netif_index, &entry->g_ifindex))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_BASE;
        }
    }

    return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
}

unsigned int npd_no_interface_tunnel(struct npd_tunnel_s* entry)
{
    entry->g_ifindex = NPD_TNL_IF_INVALID;

    if (0 != npd_l3_intf_delete(entry->netif_index))
    {
        return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
    }

    return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
}

unsigned int npd_ip_tunnel(unsigned int id)
{
    struct npd_tunnel_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    if (0 != npd_tunnel_search(&entry))
    {
        entry.id = id;
        entry.netif_index = npd_netif_tunnel_get_index(id);
        entry.g_ifindex = NPD_TNL_IF_INVALID;
        entry.mode = ip_tunnel_none;
        entry.nh_is_direct = nh_invalid;

        if (0 != npd_tunnel_insert(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }
    }

    return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
}

unsigned int npd_no_ip_tunnel(unsigned int id)
{
    struct npd_tunnel_s entry;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    if (0 == npd_tunnel_search(&entry))
    {
        if (NPD_TUNNEL_VALID(&entry) && NPD_TUNNEL_IF_VALID(&entry))
        {
            npd_no_interface_tunnel(&entry);
        }

        if (0 != npd_tunnel_update(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }
        
        if (0 != npd_tunnel_delete(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }
    }

    return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
}

unsigned int npd_tunnel_source(unsigned int id, unsigned int in4)
{
    unsigned int l3_index = 0;
    unsigned int netif_index = 0;
    struct npd_tunnel_s entry;
    struct npd_tunnel_s temp;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    if (0 == npd_tunnel_search(&entry))
    {
        if (in4 == entry.tnl_m_6in4_sip)
        {
            return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
        }
        
        if (0 != in4)
        {
            memcpy(&temp, &entry, sizeof(entry));
            temp.tnl_m_6in4_sip = in4;
            if (0 != npd_tunnel_confilct_check(&temp))
            {
                return NPD_IP_TUNNEL_RETURN_CODE_CONFLICT;
            }
        }
        
        if ((0 == in4) || (0 != entry.tnl_m_6in4_sip))
        {
            if (NPD_TUNNEL_VALID(&entry) && NPD_TUNNEL_IF_VALID(&entry))
            {
                npd_no_interface_tunnel(&entry);
            }

            entry.tnl_m_6in4_vlan = 0;
            entry.tnl_sip_valid = 0;
        }

        if (0 != in4)
        {
            do 
            {
                if (NPD_TRUE != npd_intf_netif_get_by_ip(&l3_index, in4))
                {
                    break;
                }
                
                if (NPD_TRUE != npd_intf_netif_get_by_ifindex(l3_index, &netif_index))
                {
                    break;
                }
                entry.tnl_m_6in4_vlan = (unsigned short)npd_netif_vlan_get_vid(netif_index);
                entry.tnl_sip_valid = 1;
            } while (0);
        }
        
        entry.tnl_m_6in4_sip = in4;
        if (0 != npd_tunnel_update(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }

        if (NPD_TUNNEL_VALID(&entry))
        {
            npd_interface_tunnel(&entry);
        }
        
        if (0 != npd_tunnel_update(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }
    }
    else
    {
        return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
    }

    return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
}

unsigned int npd_tunnel_destination(unsigned int id, unsigned int in4)
{
    struct npd_tunnel_s entry;
    struct npd_tunnel_s temp;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    if (0 == npd_tunnel_search(&entry))
    {
        if (in4 == entry.tnl_m_6in4_dip)
        {
            return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
        }
        
        if (0 != in4)
        {
            memcpy(&temp, &entry, sizeof(entry));
            temp.tnl_m_6in4_dip = in4;
            if (0 != npd_tunnel_confilct_check(&temp))
            {
                return NPD_IP_TUNNEL_RETURN_CODE_CONFLICT;
            }
        }

        if ((0 == in4) || (0 != entry.tnl_m_6in4_dip))
        {
            if (NPD_TUNNEL_VALID(&entry) && NPD_TUNNEL_IF_VALID(&entry))
            {
                npd_no_interface_tunnel(&entry);
            }
            
            if (0 != entry.tnl_m_6in4_dip)
            {
                entry.tnl_m_6in4_dip = 0;
                npd_tunnel_nh(&entry);
            }
        }
        entry.tnl_m_6in4_dip = in4;
        npd_tunnel_nh(&entry);

        if (0 != npd_tunnel_update(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }

        if (NPD_TUNNEL_VALID(&entry))
        {
            npd_interface_tunnel(&entry);
        }
        
        if (0 != npd_tunnel_update(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }
    }
    else
    {
        return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
    }

    return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
}

unsigned int npd_tunnel_mode(unsigned int id, unsigned int mode)
{
    struct npd_tunnel_s entry;
    struct npd_tunnel_s temp;

    memset(&entry, 0, sizeof(entry));
    entry.id = id;
    if (0 == npd_tunnel_search(&entry))
    {
        if (mode == entry.mode)
        {
            return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
        }

        if (ip_tunnel_none != mode)
        {
            memcpy(&temp, &entry, sizeof(entry));
            temp.mode = mode;
            if (0 != npd_tunnel_confilct_check(&temp))
            {
                return NPD_IP_TUNNEL_RETURN_CODE_CONFLICT;
            }
        }

        if ((ip_tunnel_none == mode)
            || (ip_tunnel_none != entry.mode))
        {
            if (NPD_TUNNEL_VALID(&entry) && NPD_TUNNEL_IF_VALID(&entry))
            {
                npd_no_interface_tunnel(&entry);
            }
        }

        entry.mode = mode;
        if (0 != npd_tunnel_update(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }
        
        if (NPD_TUNNEL_VALID(&entry))
        {
            npd_interface_tunnel(&entry);
        }

        if (0 != npd_tunnel_update(&entry))
        {
            return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
        }
    }
    else
    {
        return NPD_IP_TUNNEL_RETURN_CODE_ERROR;
    }

    return NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
}

#define NPD_DBUS_FOR_ERROR(err)                     \
do {                                                \
    npd_syslog_err("Unable to get input args\n");   \
    if (dbus_error_is_set(&err))                \
    {                                           \
        npd_syslog_err("%s raised: %s\n", err.name, err.message);        \
        dbus_error_free(&err);                  \
    }                                           \
    return NULL;                                \
} while (0)

#define NPD_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret)                \
do {                                                                \
    DBusMessageIter iter;                                           \
    reply = dbus_message_new_method_return(msg);                    \
    dbus_message_iter_init_append(reply, &iter);                    \
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);  \
} while (0)


DBusMessage* npd_dbus_uint32_processor(DBusMessage* msg, unsigned int (*processor)(unsigned int))
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int value = 0;
    unsigned int ret = NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &value,
                    DBUS_TYPE_INVALID)))
    {
        NPD_DBUS_FOR_ERROR(err);
    }

    ret = processor(value);

    NPD_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* npd_dbus_uint32_in6_processor(DBusMessage* msg, unsigned int (*processor)(unsigned int, struct in6_addr*))
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int value = 0;
    unsigned int ret = NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
    struct in6_addr in6;

    memset(&in6, 0, sizeof(in6));
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &value,
                    DBUS_TYPE_UINT32, &in6.s6_addr32[0],
                    DBUS_TYPE_UINT32, &in6.s6_addr32[1],
                    DBUS_TYPE_UINT32, &in6.s6_addr32[2],
                    DBUS_TYPE_UINT32, &in6.s6_addr32[3],
                    DBUS_TYPE_INVALID)))
    {
        NPD_DBUS_FOR_ERROR(err);
    }

    ret = processor(value, &in6);

    NPD_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* npd_dbus_2_uint32_processor(DBusMessage* msg, unsigned int (*processor)(unsigned int , unsigned int))
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int value_1 = 0;
    unsigned int value_2 = 0;
    unsigned int ret = NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &value_1,
                    DBUS_TYPE_UINT32, &value_2,
                    DBUS_TYPE_INVALID)))
    {
        NPD_DBUS_FOR_ERROR(err);
    }

    ret = processor(value_1, value_2);

    NPD_DBUS_FOR_EETURN_ONE_ARG(reply, msg, ret);

    return reply;
}

DBusMessage* npd_dbus_show_interface_tunnel_processor(DBusMessage* msg, unsigned int (*processor)(struct npd_tunnel_s*))
{
    DBusMessage*    reply = NULL;
    DBusError       err;
    unsigned int ret = NPD_IP_TUNNEL_RETURN_CODE_SUCCESS;
    unsigned char* p_str = 0;
    struct npd_tunnel_s entry;
    
    memset(&entry, 0, sizeof(entry));
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                    DBUS_TYPE_UINT32, &entry.id,
                    DBUS_TYPE_INVALID)))
    {
        NPD_DBUS_FOR_ERROR(err);
    }

    ret = processor(&entry);
    p_str = (unsigned char*)&entry;

    reply = dbus_message_new_method_return(msg);

    dbus_message_append_args(reply,
                        DBUS_TYPE_UINT32,
                        &ret,

                        DBUS_TYPE_ARRAY,
                        DBUS_TYPE_BYTE,
                        &p_str,
                        sizeof(struct npd_tunnel_s),

                        DBUS_TYPE_INVALID);

    return reply;
}

DBusMessage * npd_dbus_ip_tunnel(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_uint32_processor(msg, npd_ip_tunnel);
}

DBusMessage * npd_dbus_no_ip_tunnel(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_uint32_processor(msg, npd_no_ip_tunnel);
}

DBusMessage * npd_dbus_tunnel_source(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_2_uint32_processor(msg, npd_tunnel_source);
}

DBusMessage * npd_dbus_tunnel_destination(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_2_uint32_processor(msg, npd_tunnel_destination);
}

DBusMessage * npd_dbus_tunnel_mode(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_2_uint32_processor(msg, npd_tunnel_mode);
}

DBusMessage * npd_dbus_show_interface_tunnel_id(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_show_interface_tunnel_processor(msg, npd_tunnel_search);
}

DBusMessage * npd_dbus_show_interface_tunnel_next(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    return npd_dbus_show_interface_tunnel_processor(msg, npd_tunnel_search_next);
}

DBusMessage * npd_dbus_tunnel_show_running(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    DBusMessage*    reply = NULL;
    DBusMessageIter iter;
    int cur_len = 0;
    char address[64];
    struct npd_tunnel_s entry;
    char* pbuf = NULL;
    char* cur_str = NULL;
    char* tunnel_type[ip_tunnel_max] = {[ip_tunnel_none] = "none",
                                        [ip_6in4_tunnel] = "ipv6ip"};

    pbuf = malloc(512 * NPD_IP_TUNNEL_SIZE);
    cur_str = pbuf;

    memset(&entry, 0, sizeof(entry));
    while (0 == npd_tunnel_search_next(&entry))
    {
        cur_len += sprintf(cur_str, "interface tunnel %d\n", entry.id);
        cur_str = pbuf + cur_len;

        if (0 != entry.tnl_m_6in4_sip)
        {
            memset(address, 0, sizeof(address));
            inet_ntop(AF_INET, &entry.tnl_m_6in4_sip, address, NPD_INET_ADDRSTRLEN);
            cur_len += sprintf(cur_str, " tunnel source %s\n", address);
            cur_str = pbuf + cur_len;
        }

        if (0 != entry.tnl_m_6in4_dip)
        {
            memset(address, 0, sizeof(address));
            inet_ntop(AF_INET, &entry.tnl_m_6in4_dip, address, NPD_INET_ADDRSTRLEN);
            cur_len += sprintf(cur_str, " tunnel destination %s\n", address);
            cur_str = pbuf + cur_len;
        }

        if (ip_tunnel_none != entry.mode)
        {
            cur_len += sprintf(cur_str, " tunnel mode %s\n", tunnel_type[entry.mode]);
            cur_str = pbuf + cur_len;
        }

        cur_len += sprintf(cur_str, " exit\n");
        cur_str = pbuf + cur_len;
    }
    
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &pbuf);

    if (NULL != pbuf)
    {
        free(pbuf);
    }

    return reply;
}

#endif


