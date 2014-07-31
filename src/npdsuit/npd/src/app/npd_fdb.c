/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_fdb.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for FDB module.
*
* DATE:
*		02/21/2008
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.90 $
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_fdb.h"


unsigned int npd_fdb_dynamic_entry_del
(
    unsigned char* mac,
    unsigned short vid
);
int npd_fdb_dynamic_entry_sticky
(
    hash_table_index_t *hash,
    void *in,
    unsigned int flag
);
int npd_fdb_swshadow_enable = FALSE;

hash_table_index_t *npd_fdb_hashmac_index = NULL;

array_table_index_t *npd_fdb_cfg_index = NULL;

db_table_t         *npd_fdb_dbtbl = NULL;
db_table_t         *npd_fdb_cfgtbl = NULL;
unsigned int fdb_cfg_global_no = 0;
unsigned int g_entry_fdb_count = 0;

static unsigned int g_agingtime = 300;

static char *npd_fdb_showStr = NULL;
static int   npd_fdb_showStr_len = 0;


void npd_fdb_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
);

void npd_fdb_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
);

netif_event_notifier_t npd_fdb_netif_notifier =
{
    .netif_event_handle_f = &npd_fdb_notify_event,
    .netif_relate_handle_f = &npd_fdb_relate_event
};


/**********************************************************************************
 * npd_fdb_key_mac_generate
 *
 * FDB database(Hash table) hash key generation method
 *
 *	INPUT:
 *		item	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		hash key calculated if no error occur,otherwise all 1s(0xFFFFFFFF).
 *
 **********************************************************************************/
unsigned int npd_fdb_key_mac_generate
(
    void * in
)
{
    struct fdb_entry_item_s *item = (struct fdb_entry_item_s *)in;
    unsigned int key = 0; /*for hash key calculate*/
#if 0
    unsigned int p = 16777619;
    unsigned int vid = 0;
    unsigned char mac[MAC_ADDR_LEN] = {0};
    unsigned int tmpData = 0;
    int i = 0;
    key = jhash(item->mac, MAC_ADDR_LEN, 0);

    if (NULL == item)
    {
        syslog_ax_arpsnooping_err("npd arp snooping items make key null pointers error.");
        return ~0UI;
    }

    vid = item->vlanid;
    memcpy(mac,item->mac,MAC_ADDR_LEN);

    for (i = 0; i<8; i++)
    {
        tmpData = ((i == 0)?((vid>>8)&0xff)\
                   :((i == 1)?(vid&0xff):(mac[i-2])));
        key = (key^tmpData)*p;
        key += key<<13;
        key ^= key>>7;
        key += key<<3;
        key ^= key>>17;
        key += key<<5;
    }

#endif
    key = ((item->mac[4] << 8) | item->mac[5]);
	key %= NPD_FDB_TABLE_SIZE;
	
	return key;
}


/**********************************************************************************
 * npd_fdb_compare_by_vlan
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_compare_by_vlan
(
    void *in1,
    void *in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (itemA->vlanid != itemB->vlanid)
    {
        equal = FALSE;
    }

    return equal;
}

/**********************************************************************************
 * npd_fdb_compare_by_port
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_compare_by_port
(
    void *in1,
    void *in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (itemA->ifIndex != itemB->ifIndex)
    {
        equal = FALSE;
    }

    return equal;
}

/**********************************************************************************
 * npd_fdb_compare
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_compare
(
    void * in1,
    void * in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    unsigned int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (itemA->vlanid != itemB->vlanid)
    {
        equal = FALSE;
    }
    else if (0 != memcmp((char*)itemA->mac,(char*)itemB->mac,MAC_ADDRESS_LEN))  /* MAC*/
    {
        equal = FALSE;
    }

    return equal;
}

/**********************************************************************************
 * npd_fdb_filter_by_vlan_port
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_static_filter_by_vlan_port
(
    void * in1,
    void * in2
)
{
	struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
	struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (itemA->ifIndex != itemB->ifIndex)
    {
        equal = FALSE;
    }
    else if (itemA->vlanid != itemB->vlanid)
    {
        equal = FALSE;
    }

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

    return equal;
}

/**********************************************************************************
 * npd_static_blacklist_fdb_filter
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_blacklist_filter
(
    void * in1,
    void * in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (!itemA->isBlock || !itemB->isBlock)
    {
        equal = FALSE;
    }

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

    return equal;
}


unsigned int npd_fdb_dynamic_filter
(
	void * in1,
	void * in2
)
{
	struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
	struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;		
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB))
	{
		syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if( itemA->isStatic || itemB->isStatic )
	{
		equal = FALSE;
	}

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}


	return equal;
}

unsigned int npd_fdb_netif_index_filter
(
	void * in1,
	void * in2
)
{
	struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
	struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;		
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB))
	{
		syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if( itemA->ifIndex != itemB->ifIndex )
	{
		equal = FALSE;
	}

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

	return equal;
}

unsigned int npd_fdb_vlan_filter
(
	void * in1,
	void * in2
)
{
	struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
	struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;		
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB))
	{
		syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	if( itemA->vlanid != itemB->vlanid )
	{
		equal = FALSE;
	}

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

	return equal;
}

unsigned int npd_fdb_mac_filter
(
	void * in1,
	void * in2
)
{
	struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
	struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;		
	int equal = TRUE;

	if((NULL==itemA)||(NULL==itemB))
	{
		syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
		return FALSE;
	}

	/* */
	if( memcmp(itemA->mac, itemB->mac, 6) )
	{
		equal = FALSE;
	}

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

	return equal;
}

/**********************************************************************************
 * npd_static_fdb_filter
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_static_filter
(
    void * in1,
    void * in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (!itemA->isStatic || !itemB->isStatic)
    {
        equal = FALSE;
    }
	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

	

    return equal;
}

/**********************************************************************************
 * npd_static_fdb_filter_by_vlan
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_static_filter_by_vlan
(
    void * in1,
    void * in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (itemA->vlanid != itemB->vlanid)
    {
        equal = FALSE;
    }

    if (!itemA->isStatic || !itemB->isStatic)
    {
        equal = FALSE;
    }

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

    return equal;
}

/**********************************************************************************
 * npd_static_blacklist_fdb_filter_by_vlan
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_blacklist_filter_by_vlan
(
    void * in1,
    void * in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (itemA->vlanid != itemB->vlanid)
    {
        equal = FALSE;
    }

    if (!itemA->isBlock || !itemB->isBlock)
    {
        equal = FALSE;
    }

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

    return equal;
}

/**********************************************************************************
 * npd_static_blacklist_fdb_filter_by_vlan
 *
 * compare two of FDB database(Hash table) items
 *
 *	INPUT:
 *		itemA	- FDB database item
 *		itemB	- FDB database item
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FALSE 	- if two items are not equal.
 *		TRUE 	- if two items are equal to each other.
 *
 **********************************************************************************/
unsigned int npd_fdb_static_filter_by_mirror_profile
(
    void * in1,
    void * in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }

    if (!itemA->isMirror || !itemB->isMirror)
    {
        equal = FALSE;
    }

    if (itemA->mirrorProfile != itemB->mirrorProfile)
    {
        equal = FALSE;
    }

	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

    return equal;
}

unsigned int npd_fdb_all_filter
(
    void * in1,
    void * in2
)
{
    struct fdb_entry_item_s *itemA = (struct fdb_entry_item_s *)in1;
    struct fdb_entry_item_s *itemB = (struct fdb_entry_item_s *)in2;
    int equal = TRUE;

    if ((NULL==itemA)||(NULL==itemB))
    {
        syslog_ax_arpsnooping_err("npd arp snooping items compare null pointers error.");
        return FALSE;
    }
	
	if (itemA->flagApp || itemB->flagApp)
	{
		equal = FALSE;
	}

    return equal;
}

long npd_fdb_dbtbl_handle_update(void *newItem, void *oldItem)
{
    int ret = FDB_RETURN_CODE_SUCCESS;
    struct fdb_entry_item_s *origItem = NULL, *updateItem = NULL;

    if (NULL == newItem || NULL == oldItem)
        return FDB_RETURN_CODE_NULL_PTR;

    origItem = (struct fdb_entry_item_s *)oldItem;
    updateItem = (struct fdb_entry_item_s *)newItem;
    syslog_ax_fdb_dbg("FDB table update: update entry %.2x:%.2x:%.2x:%.2x:%.2x:%.2x Vlan %d Port_index 0x%x\n",\
                      updateItem->mac[0], updateItem->mac[1], updateItem->mac[2], updateItem->mac[3],
                      updateItem->mac[4], updateItem->mac[5], updateItem->vlanid, updateItem->ifIndex);

    if (!origItem->isMirror && updateItem->isMirror)   //change from static fdb to static mirror fdb
    {
        ret = nam_mirror_fdb_entry_set(updateItem->vlanid, updateItem->ifIndex,(ETHERADDR *)updateItem->mac,1);
    }
    else if (origItem->blockMode != updateItem->blockMode)  //change blacklist mac mode
    {
        if (updateItem->blockMode & NPD_FDB_BLACKLIST_SMAC_MODE)
        {
            if (!(origItem->blockMode & NPD_FDB_BLACKLIST_SMAC_MODE))
            {
                ret = nam_fdb_entry_mac_vlan_drop(updateItem->mac,updateItem->vlanid,0, updateItem->isStatic);
            }
        }
        else
        {
            if (origItem->blockMode & NPD_FDB_BLACKLIST_SMAC_MODE)
            {
                ret = nam_fdb_entry_mac_vlan_no_drop(origItem->mac,origItem->vlanid,0);
            }
        }

        if (updateItem->blockMode & NPD_FDB_BLACKLIST_DMAC_MODE)
        {
            if (!(origItem->blockMode & NPD_FDB_BLACKLIST_DMAC_MODE))
            {
                ret = nam_fdb_entry_mac_vlan_drop(updateItem->mac,updateItem->vlanid,1, updateItem->isStatic);
            }
        }
        else
        {
            if (origItem->blockMode & NPD_FDB_BLACKLIST_DMAC_MODE)
            {
                ret = nam_fdb_entry_mac_vlan_no_drop(origItem->mac,origItem->vlanid,1);
            }
        }
    }
	else if(!origItem->flagApp && updateItem->flagApp)
	{
        ret = nam_fdb_app_entry_add(updateItem->vlanid, (ETHERADDR *)updateItem->mac,updateItem->ifIndex, updateItem->flagApp);
	}
	else if(updateItem->isStatic != origItem->isStatic)
	{
        port_driver_t *driver;
		if(updateItem->isStatic)
		{
            driver = port_driver_get(updateItem->ifIndex);
    
            if (NULL == driver)
                return -1;
    
            ret = (*driver->fdb_add)(updateItem->mac,updateItem->vlanid,updateItem->ifIndex);
		}
	}
    else
    {
        ret = nam_fdb_entry_mac_vlan_port_set(updateItem->mac, updateItem->vlanid, updateItem->ifIndex);
    }

    return 0;
}

long npd_fdb_dbtbl_handle_insert(void *newItem)
{
    struct fdb_entry_item_s *opItem = NULL;
    int status;

    if (newItem == NULL)
        return FDB_RETURN_CODE_NULL_PTR;

    opItem = (struct fdb_entry_item_s *)newItem;
    syslog_ax_fdb_dbg("FDB table insert: insert entry %.2x:%.2x:%.2x:%.2x:%.2x:%.2x Vlan %d Port_index 0x%x\n",\
                      opItem->mac[0], opItem->mac[1], opItem->mac[2], opItem->mac[3],
                      opItem->mac[4], opItem->mac[5], opItem->vlanid, opItem->ifIndex);

    if (opItem->isMirror == TRUE)
    {
        status = nam_mirror_fdb_entry_set(opItem->vlanid, opItem->ifIndex,(ETHERADDR *)opItem->mac, opItem->mirrorProfile);
    }
    else if (opItem->isBlock)
    {
        if (opItem->blockMode & NPD_FDB_BLACKLIST_SMAC_MODE)
        {
            status = nam_fdb_entry_mac_vlan_drop(opItem->mac,opItem->vlanid,0, opItem->isStatic);
        }

        if (opItem->blockMode & NPD_FDB_BLACKLIST_DMAC_MODE)
        {
            status = nam_fdb_entry_mac_vlan_drop(opItem->mac,opItem->vlanid,1, opItem->isStatic);
        }
    }
    else if (opItem->flagApp)
    {
        status = nam_fdb_app_entry_add(opItem->vlanid, (ETHERADDR *)opItem->mac, opItem->ifIndex, opItem->flagApp);
    }	
    else if (opItem->isStatic)
    {
        port_driver_t *driver;
        driver = port_driver_get(opItem->ifIndex);

        if (NULL == driver)
            return -1;

        status = (*driver->fdb_add)(opItem->mac,opItem->vlanid,opItem->ifIndex);
    }
    else
    {
        status = nam_fdb_entry_mac_vlan_port_set(opItem->mac,opItem->vlanid,opItem->ifIndex);
        syslog_ax_fdb_dbg("For dynamic fdb insert\n");
    }

    return 0;
}

long npd_fdb_dbtbl_handle_delete(void *delItem)
{
    struct fdb_entry_item_s *opItem = NULL;
    int status;

    if (delItem == NULL)
        return FDB_RETURN_CODE_NULL_PTR;

    opItem = (struct fdb_entry_item_s *)delItem;
    syslog_ax_fdb_dbg("FDB table delete: delete entry %.2x:%.2x:%.2x:%.2x:%.2x:%.2x Vlan %d Port_index 0x%x\n",\
                      opItem->mac[0], opItem->mac[1], opItem->mac[2], opItem->mac[3],
                      opItem->mac[4], opItem->mac[5], opItem->vlanid, opItem->ifIndex);

    if (opItem->isMirror == TRUE)
    {
        status = nam_mirror_fdb_entry_set(opItem->vlanid, opItem->ifIndex,(ETHERADDR *)opItem->mac,0);
    }
    else if (opItem->isBlock)
    {
        if (opItem->blockMode & NPD_FDB_BLACKLIST_SMAC_MODE)
        {
            status = nam_fdb_entry_mac_vlan_no_drop(opItem->mac,opItem->vlanid, 0);
        }

        if (opItem->blockMode & NPD_FDB_BLACKLIST_DMAC_MODE)
        {
            status = nam_fdb_entry_mac_vlan_no_drop(opItem->mac,opItem->vlanid, 1);
        }
    }
    else if (opItem->flagApp)
    {
        status = nam_fdb_app_entry_del(opItem->vlanid, (ETHERADDR *)opItem->mac, opItem->flagApp);
    }	
    else if (opItem->isStatic)
    {
        status = nam_no_static_fdb_entry_mac_vlan_set(opItem->mac,opItem->vlanid, opItem->ifIndex);
    }
    else
    {
        status = nam_fdb_entry_mac_vlan_port_delete(opItem->mac,opItem->vlanid, opItem->ifIndex);
    }

    return 0;
}

long npd_fdb_cfgtbl_handle_update(void *newItem, void *oldItem)
{
    int ret = FDB_RETURN_CODE_SUCCESS;
    struct npd_fdb_cfg_s* npd_fdb_cfg_new = (struct npd_fdb_cfg_s*)newItem;
    struct npd_fdb_cfg_s* npd_fdb_cfg_old = (struct npd_fdb_cfg_s*)oldItem;

    if (npd_fdb_cfg_new->agingtime != npd_fdb_cfg_old->agingtime)
    {
        ret = nam_fdb_table_agingtime_set(npd_fdb_cfg_new->agingtime);

        if (ret != FDB_RETURN_CODE_SUCCESS)
        {
            syslog_ax_fdb_err("npd_dbus_fdb_config_agingtime:: fdb agingtime %d set ERROR. \n",npd_fdb_cfg_new->agingtime);
            ret = FDB_RETURN_CODE_OCCUR_HW;
        }
        else
        {
    		g_agingtime = npd_fdb_cfg_new->agingtime;
        }
    }
		
    if ((npd_fdb_cfg_new->del_netif_index)
            &&(npd_fdb_cfg_new->del_vlanid))
    {
        npd_fdb_entry_del_by_vlan_port(npd_fdb_cfg_new->del_vlanid,
                                       npd_fdb_cfg_new->del_netif_index);
    }
    else if (npd_fdb_cfg_new->del_netif_index)
    {
        npd_fdb_entry_del_by_port(npd_fdb_cfg_new->del_netif_index);
    }
    else if (npd_fdb_cfg_new->del_vlanid)
    {
        npd_fdb_entry_del_by_vlan(npd_fdb_cfg_new->del_vlanid);
    }

    return 0;
}

long npd_fdb_cfgtbl_handle_insert(void *newItem)
{
    int ret = FDB_RETURN_CODE_SUCCESS;
    struct npd_fdb_cfg_s* npd_fdb_cfg_new = (struct npd_fdb_cfg_s*)newItem;
    ret = nam_fdb_table_agingtime_set(npd_fdb_cfg_new->agingtime);

    if (ret != FDB_RETURN_CODE_SUCCESS)
    {
        syslog_ax_fdb_err("npd_dbus_fdb_config_agingtime:: fdb agingtime %d set ERROR. \n",npd_fdb_cfg_new->agingtime);
        ret = FDB_RETURN_CODE_OCCUR_HW;
    }
    else
    {
		g_agingtime = npd_fdb_cfg_new->agingtime;
    }
    return ret;
}
#ifdef HAVE_FDB_SW_SYNC
#define NPD_FDB_AGE_INTERVAL 5

int npd_fdb_timer(void)
{
    int curtime = 0;
    static struct fdb_entry_item_s item;
    int ret;
    nam_l2_addr_t l2_addr;
    npd_init_tell_whoami("npdFdbTimer",0);

    while (1)
    {
        sleep(NPD_FDB_AGE_INTERVAL);

        if (!SYS_LOCAL_MODULE_ISMASTERACTIVE)
            continue;

        if (npd_fdb_swshadow_enable)
        {
            curtime = time(NULL);
            ret = dbtable_hash_head(npd_fdb_hashmac_index, NULL, &item, NULL);

            while (0 == ret)
            {
                if (item.time - curtime >= g_agingtime)
                {
                    ret = nam_l2_addr_get(item.mac, item.vlanid, &l2_addr);

                    if (0 != ret)
                    {
                        dbtable_hash_delete(npd_fdb_hashmac_index, &item, &item);
                        ret = dbtable_hash_head(npd_fdb_hashmac_index, NULL, &item, NULL);
                    }
                    else
                    {
                        if (l2_addr.hit)
                            item.time = curtime;

                        ret = dbtable_hash_update(npd_fdb_hashmac_index, NULL, &item);
                        ret = dbtable_hash_next(npd_fdb_hashmac_index, &item, &item, NULL);
                    }
                }
            }
        }
    }

    return ARP_RETURN_CODE_SUCCESS;
}
#endif
int npd_fdb_dbtbl_handle_ntoh(void *data)
{
    struct fdb_entry_item_s *fdbEntry = (struct fdb_entry_item_s *)data;
    fdbEntry->vlanid = ntohs(fdbEntry->vlanid);
    fdbEntry->ifIndex = ntohl(fdbEntry->ifIndex);
    fdbEntry->time = ntohl(fdbEntry->time);
    return 0;
}

int npd_fdb_dbtbl_handle_hton(void *data)
{
    struct fdb_entry_item_s *fdbEntry = (struct fdb_entry_item_s *)data;
    fdbEntry->vlanid = htons(fdbEntry->vlanid);
    fdbEntry->ifIndex = htonl(fdbEntry->ifIndex);
    fdbEntry->time = ntohl(fdbEntry->time);
    return 0;
}

int npd_fdb_cfgtbl_handle_ntoh(void *data)
{
    struct npd_fdb_cfg_s *fdbCfg = (struct npd_fdb_cfg_s *)data;
    fdbCfg->agingtime = ntohl(fdbCfg->agingtime);
    fdbCfg->del_netif_index = ntohl(fdbCfg->del_netif_index);
    fdbCfg->del_vlanid = ntohl(fdbCfg->del_vlanid);
    fdbCfg->del_all = ntohl(fdbCfg->del_all);
    return 0;
}

int npd_fdb_cfgtbl_handle_hton(void *data)
{
    struct npd_fdb_cfg_s *fdbCfg = (struct npd_fdb_cfg_s *)data;
    fdbCfg->agingtime = htonl(fdbCfg->agingtime);
    fdbCfg->del_netif_index = htonl(fdbCfg->del_netif_index);
    fdbCfg->del_vlanid = htonl(fdbCfg->del_vlanid);
    fdbCfg->del_all = htonl(fdbCfg->del_all);
    return 0;
}


int npd_fdb_table_init()
{
    int ret;
    struct npd_fdb_cfg_s npd_fdb_cfg_default;
    ret = create_dbtable(NPD_FDB_HASHTBL_NAME, NPD_FDB_TABLE_SIZE, sizeof(struct fdb_entry_item_s),\
                         npd_fdb_dbtbl_handle_update,
                         NULL,
                         npd_fdb_dbtbl_handle_insert,
                         npd_fdb_dbtbl_handle_delete,
                         NULL,
                         NULL,
                         NULL,
                         npd_fdb_dbtbl_handle_ntoh,
                         npd_fdb_dbtbl_handle_hton,
                         (DB_SYNC_ALL|DB_SYNC_ASYNC),
                         &(npd_fdb_dbtbl));

    if (0 != ret)
    {
        syslog_ax_fdb_err("create npd fdb database fail\n");
        return NPD_FAIL;
    }

    ret = dbtable_create_hash_index("mac", npd_fdb_dbtbl,NPD_FDB_TABLE_SIZE, npd_fdb_key_mac_generate,\
                                    npd_fdb_compare, &npd_fdb_hashmac_index);

    if (0 != ret)
    {
        syslog_ax_fdb_err("create npd mac based hash index fail\n");
        return NPD_FAIL;
    }

    register_netif_notifier(&npd_fdb_netif_notifier);
    ret = create_dbtable(NPD_FDB_CFGTBL_NAME, 1, sizeof(struct npd_fdb_cfg_s),\
                         npd_fdb_cfgtbl_handle_update,
                         NULL,
                         npd_fdb_cfgtbl_handle_insert,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         npd_fdb_cfgtbl_handle_ntoh,
                         npd_fdb_cfgtbl_handle_hton,
                         DB_SYNC_ALL,
                         &npd_fdb_cfgtbl);

    if (0 != ret)
    {
        syslog_ax_fdb_err("create npd fdb configuration table fail\n");
        return NPD_FAIL;
    }

    ret = dbtable_create_array_index("fdb_cfg", npd_fdb_cfgtbl, &npd_fdb_cfg_index);

    if (0 != ret)
    {
        syslog_ax_fdb_err("create npd fdb configuration table index fail\n");
        return NPD_FAIL;
    }

    npd_fdb_cfg_default.agingtime = g_agingtime;
    ret = dbtable_array_insert(npd_fdb_cfg_index, &fdb_cfg_global_no, &npd_fdb_cfg_default);

    if (ret != 0)
    {
        syslog_ax_fdb_err("Insert fdb default configuration failed.\n");
        return NPD_FAIL;
    }

    /*
    	nam_thread_create("npdFDBSync",(void *)npd_fdb_timer,NULL,TRUE,FALSE);
    */
    return NPD_OK;
}

unsigned int npd_fdb_fdbCount_by_port
(
    unsigned int netif_index,
    unsigned int *fdbCount
)
{
    struct fdb_entry_item_s fdbItem;
    memset(&fdbItem, 0, sizeof(struct fdb_entry_item_s));
    fdbItem.ifIndex = netif_index;
    *fdbCount = dbtable_hash_traversal(npd_fdb_hashmac_index, TRUE, &fdbItem, npd_fdb_compare_by_port, NULL);
    return NPD_OK;
}

unsigned int npd_fdb_fdbCount_by_vlan
(
    unsigned short vlanId,
    unsigned int *fdbCount
)
{
    struct fdb_entry_item_s fdbItem;
    memset(&fdbItem, 0, sizeof(struct fdb_entry_item_s));
    fdbItem.vlanid = vlanId;
    *fdbCount = dbtable_hash_traversal(npd_fdb_hashmac_index, TRUE, &fdbItem, npd_fdb_compare_by_vlan, NULL);
    return NPD_OK;
}

int npd_fdb_set_netif_learn_status(
    unsigned int netif_index,
    unsigned int  status
)
{
    int ret = FDB_RETURN_CODE_SUCCESS;
    int netif_type = -1;
    int port_index = -1;
    int array_index = -1;
    int trunk_id = -1;
    struct trunk_s trunk;
    netif_type = npd_netif_type_get(netif_index);

    switch (netif_type)
    {
        case NPD_NETIF_ETH_TYPE:
        {
            ret = npd_port_fdb_learning_mode(netif_index, status);
            break;
        }
        case NPD_NETIF_TRUNK_TYPE:
        {
            trunk_id = npd_netif_trunk_get_tid(netif_index);

            if (npd_find_trunk(trunk_id, &trunk) != 0)
            {
                return FDB_RETURN_CODE_NODE_NOT_EXIST;
            }

            NPD_PBMP_ITER(trunk.ports, array_index)
            {
                port_index = eth_port_array_index_to_ifindex(array_index);

                if (npd_port_fdb_learning_mode(port_index, status))
                {
                    return FDB_RETURN_CODE_ERR_DBUS;
                }
            }
            break;
        }
        case NPD_NETIF_VLAN_TYPE:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    return ret;
}

int npd_fdb_check_limit_exist
(
    unsigned int dev
)
{
    unsigned char slot_no =0;
    unsigned int slot_index = 0;
    unsigned char local_port_no =0;
    unsigned int fdblimit=0;
    unsigned int ret =0;
    int i = 0;

    /*for the vlan-based*/
    for (i = 1; i<4095; i++)
    {
        ret = npd_check_vlan_exist(i);

        if (NPD_TRUE == ret)
        {
            ret = npd_fdb_number_vlan_set_check(i,&fdblimit);

            if (1 == ret)
            {
                return NPD_TRUE;
            }
        }
        else
            continue;
    }

    /*for the port-base*/
    for (slot_no = CHASSIS_SLOT_START_NO; slot_no < (CHASSIS_SLOT_COUNT+CHASSIS_SLOT_START_NO); slot_no++)
    {
        if (CHASSIS_SLOTNO_ISLEGAL(slot_no))
        {
            slot_index = CHASSIS_SLOT_NO2INDEX(slot_no);

            for (local_port_no = ETH_LOCAL_PORT_START_NO(slot_index); local_port_no <(ETH_LOCAL_PORT_COUNT(slot_index)+ETH_LOCAL_PORT_START_NO(slot_index)); local_port_no++)
            {
                if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no,local_port_no))
                {
                    unsigned int eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_INDEX(slot_index, ETH_LOCAL_NO2INDEX(slot_index, local_port_no));
                    ret = npd_fdb_number_port_set_check(eth_g_index,&fdblimit);

                    if (1 == ret)
                    {
                        return NPD_TRUE;
                    }
                }
            }
        }
    }

    return NPD_FALSE;
}

/**********************************************************************************
 * npd_fdb_entry_del
 *
 * del FDB entry from global FDB database
 *
 *	INPUT:
 *		item - struct fdb_entry_item_s
 *		flag  - operation flag
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ret - NPD_FDB_ERR_NONE
 *                 FDB_RETURN_CODE_HASH_OP_FAIL
 *
 **********************************************************************************/

int npd_fdb_entry_del(
    hash_table_index_t *hash,
    void *in,
    unsigned int flag)
{
    struct fdb_entry_item_s *item = (struct fdb_entry_item_s *)in;
    int status = 0;
    status = npd_system_verify_basemac((char *)item->mac);

    if (1 == status)
    {
        return FDB_RETURN_CODE_SUCCESS;
    }

    if (item->isMirror && flag == 2)
    {
        status = npd_fdb_static_mirror_entry_del(item->mac, item->vlanid);
    }
    else if (item->isBlock  && flag == 3)
    {
        status = npd_fdb_static_blacklist_entry_del((char *)item->mac, item->vlanid, 0);
    }
    else if (item->isStatic && flag == 1)
    {
        status = npd_fdb_static_entry_del(item->mac, item->vlanid);
    }
    else if (flag == 0)
    {
        if (item->isMirror || item->isBlock || item->isStatic)
        {
            return NPD_OK;
        }

        status = npd_fdb_dynamic_entry_del(item->mac, item->vlanid);
    }

    if (0 != status)
    {
        syslog_ax_fdb_err("FDB del entry %02x:%02x:%02x:%02x:%02x:%02x vlan %d fail, err %d\n",\
                          item->mac[0],item->mac[1],item->mac[2],item->mac[3],
                          item->mac[4],item->mac[5],item->vlanid, status);
    }

    return NPD_OK;
}


/**********************************************************************************
 * npd_fdb_entry_del_by_vlan
 *
 * del FDB entry from global FDB database
 *
 *	INPUT:
 *		item - struct fdb_entry_item_s
 *		flag  - operation flag
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ret - NPD_FDB_ERR_NONE
 *                 FDB_RETURN_CODE_HASH_OP_FAIL
 *
 **********************************************************************************/

int npd_fdb_entry_del_by_vlan(int vlanId)
{
    int ret;
    struct fdb_entry_item_s dbItem;
    int status = 0;
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE && npd_startup_end)
	{
        memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
        dbItem.vlanid = vlanId;
        dbItem.isStatic = 0;
        status = dbtable_hash_traversal(npd_fdb_hashmac_index, 0, &dbItem, \
                                        npd_fdb_compare_by_vlan, npd_fdb_entry_del);
	}
	if(npd_startup_end)
	{
        ret =  nam_fdb_table_delete_entry_with_vlan(vlanId);
    
        if (ret != NPD_FDB_ERR_NONE)
        {
            syslog_ax_fdb_err("nam_fdb_table_delete_entry_with_vlan:erro !\n");
            ret = FDB_RETURN_CODE_OCCUR_HW;
        }
	}
    ret = FDB_RETURN_CODE_SUCCESS;
    return ret;
}

/**********************************************************************************
 * npd_fdb_entry_del_by_port
 *
 * del FDB entry from global FDB database
 *
 *	INPUT:
 *		item - struct fdb_entry_item_s
 *		flag  - operation flag
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ret - NPD_FDB_ERR_NONE
 *                 FDB_RETURN_CODE_HASH_OP_FAIL
 *
 **********************************************************************************/

int npd_fdb_entry_del_by_port(int ifIndex)
{
    int ret;
    port_driver_t *driver;
    driver = port_driver_get(ifIndex);
    struct fdb_entry_item_s dbItem;
    int status = 0;
	if(SYS_LOCAL_MODULE_ISMASTERACTIVE && npd_startup_end)
	{
        memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
        dbItem.ifIndex = ifIndex;
        status = dbtable_hash_traversal(npd_fdb_hashmac_index, 0, &dbItem, \
                                        npd_fdb_compare_by_port, npd_fdb_entry_del);
	}
	if(npd_startup_end)
	{
        if (NULL == driver)
            return -1;
    
        ret = (*driver->fdb_delete_by_port)(ifIndex);
	}
    return 0;
}

/**********************************************************************************
 * npd_fdb_entry_del_by_vlan_port
 *
 * del FDB entry from global FDB database
 *
 *	INPUT:
 *		item - struct fdb_entry_item_s
 *		flag  - operation flag
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ret - NPD_FDB_ERR_NONE
 *                 FDB_RETURN_CODE_HASH_OP_FAIL
 *
 **********************************************************************************/

int npd_fdb_entry_del_by_vlan_port(unsigned vlanid, int ifIndex)
{
    int ret = FDB_RETURN_CODE_SUCCESS;
    port_driver_t *driver;
    driver = port_driver_get(ifIndex);
    if(npd_startup_end)
    {
        if (NULL == driver)
            return -1;
    
        ret = (*driver->fdb_delete_by_vlan_port)(ifIndex, vlanid);
    }
    return 0;
}

/**********************************************************************************
 * npd_fdb_dynamic_entry_del_by_vlan
 *
 * delete dynamic FDB entry to global FDB database
 *
 *	INPUT:
 *		vid -- vlan ID
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - dynamic fdb entry info stucture
 *
 **********************************************************************************/

unsigned int npd_fdb_dynamic_entry_del_by_vlan
(
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    dbItem.isStatic = 0;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 0, &dbItem, \
                                    npd_fdb_compare_by_vlan, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("FDB delete by vlan %d: delete %d entries\n", vid, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_static_entry_del_by_vlan
 *
 * delete static FDB entry to global FDB database
 *
 *	INPUT:
 *		vid - vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_static_entry_del_by_vlan
(
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    dbItem.isStatic = 1;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, TRUE, &dbItem, \
                                    npd_fdb_static_filter_by_vlan, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("FDB delete by vlan %d: delete %d entries\n", vid, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_blacklist_entry_del_by_vlan
 *
 * delete blacklist FDB entry to global FDB database
 *
 *	INPUT:
 *		vid - vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_blacklist_entry_del_by_vlan
(
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    dbItem.isBlock = 1;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 3, &dbItem, \
                                    npd_fdb_blacklist_filter_by_vlan, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("FDB delete by vlan %d: delete %d entries\n", vid, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_dynamic_entry_del_by_port
 *
 * delete dynamic FDB entry to global FDB database
 *
 *	INPUT:
 *		slot_no --slot number
 *           port_no--port number
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - dynamic fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_dynamic_entry_del_by_port
(
    unsigned int netif_index
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.ifIndex = netif_index;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 0, &dbItem, \
                                    npd_fdb_compare_by_port, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("FDB delete by port index 0x%x: delete %d entries\n", netif_index, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_static_entry_del_by_port
 *
 * delete static FDB entry to global FDB database
 *
 *	INPUT:
 *		slot_no --slot number
 *           port_no--port number
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_static_entry_del_by_port
(
    unsigned int netif_index
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.ifIndex = netif_index;
    dbItem.isStatic = 1;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 1, &dbItem, \
                                    npd_fdb_compare_by_port, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("FDB delete by port index 0x%x: delete %d entries\n", netif_index, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_mirror_entry_del_by_port
 *
 * delete mirror FDB entry to global FDB database
 *
 *	INPUT:
 *		slot_no --slot number
 *           port_no--port number
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - mirror fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_mirror_entry_del_by_port
(
    unsigned int netif_index
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.ifIndex = netif_index;
    dbItem.isMirror = 1;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 2, &dbItem, \
                                    npd_fdb_compare_by_port, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("Mirror-based FDB entry(S) deleted by port index 0x%x: delete %d entries\n", netif_index, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_block_entry_del_by_port
 *
 * delete block FDB entry to global FDB database
 *
 *	INPUT:
 *		slot_no --slot number
 *           port_no--port number
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - block fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_block_entry_del_by_port
(
    unsigned int netif_index
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.ifIndex = netif_index;
    dbItem.isBlock = 1;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 3, &dbItem, \
                                    npd_fdb_compare_by_port, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("Blocked FDB entry(S) deleted by port index 0x%x: delete %d entries\n", netif_index, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_static_entry_del_by_vlan_port
 *
 * delete static FDB entry to global FDB database
 *
 *	INPUT:
 *		slot_no --slot number
 *           port_no--port number
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_static_entry_del_by_vlan_port
(
    unsigned int vlanid,
    unsigned int netif_index
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.ifIndex = netif_index;
    dbItem.vlanid = vlanid;
    dbItem.isStatic = 1;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, TRUE, &dbItem, \
                                    npd_fdb_static_filter_by_vlan_port, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("Static FDB entry(s) deleted by vlan %d port index 0x%x: delete %d entries\n", vlanid, netif_index, status);
    return FDB_RETURN_CODE_SUCCESS;
}

unsigned int npd_fdb_dynamic_entry_del_by_vlan_port
(
    unsigned int vlanid,
    unsigned int netif_index
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.ifIndex = netif_index;
    dbItem.vlanid = vlanid;
    dbItem.isStatic = 0;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 0, &dbItem, \
                                    npd_fdb_static_filter_by_vlan_port, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("Dynamic FDB entry(s) deleted by vlan %d port index 0x%x: delete %d entries\n", vlanid, netif_index, status);
    return FDB_RETURN_CODE_SUCCESS;
}

unsigned int npd_fdb_dynamic_entry_sticky_by_netif
(
    unsigned int netif_index
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.ifIndex = netif_index;
    dbItem.isStatic = 0;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 0, &dbItem, \
                                    npd_fdb_compare_by_port, npd_fdb_dynamic_entry_sticky);
    syslog_ax_fdb_dbg("Dynamic FDB entry(s) stickied by netif 0x%x: %d entries\n", netif_index, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_static_entry_add
 *
 * add static FDB entry to global FDB database
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *		eth_index - global eth port index
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/

unsigned int npd_fdb_static_entry_add
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int eth_g_index,
    unsigned int  isMirror
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    int fdb_count = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    dbItem.ifIndex = eth_g_index;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    dbItem.isMirror = isMirror;
    fdb_count = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, NULL, 0, npd_fdb_static_filter);

    if (fdb_count >= AX_STATIC_FDB_ENTRY_SIZE)
    {
        syslog_ax_fdb_event("npd static fdb database overrun.\n");
        return FDB_RETURN_CODE_MAX;
    }

    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isBlock)
        {
            syslog_ax_fdb_err("The item has exist as one blacklist item. Please delete it first \n");
            return FDB_RETURN_CODE_NODE_EXIST;
        }

        if (dupItem.isStatic)
        {
            if (isMirror && !dupItem.isMirror)
            {
                dupItem.isMirror = isMirror;
                status = dbtable_hash_update(npd_fdb_hashmac_index, &dupItem, &dupItem);
            }
            else
            {
                syslog_ax_fdb_err("The item has exist as one static item. Please delete it first \n");
                return FDB_RETURN_CODE_NODE_EXIST;
            }
        }
    }

    status = dbtable_hash_insert(npd_fdb_hashmac_index, &dbItem);

    if (0 != status)
        return FDB_RETURN_CODE_HASH_OP_FAIL;

    return FDB_RETURN_CODE_SUCCESS;
}

unsigned int npd_fdb_check_entry_exist
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int * eth_g_index
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        *eth_g_index = dupItem.ifIndex;
        return FDB_RETURN_CODE_SUCCESS;
    }

    return FDB_RETURN_CODE_NODE_NOT_EXIST;
}


unsigned int npd_fdb_check_static_entry_exist
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int * eth_g_index
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isStatic == 1)
            *eth_g_index = dupItem.ifIndex;
        else
            return FDB_RETURN_CODE_NODE_NOT_EXIST;

        return NPD_FDB_ERR_NODE_EXIST;
    }

    return FDB_RETURN_CODE_NODE_NOT_EXIST;
}

/**********************************************************************************
 * npd_fdb_dynamic_entry_del
 *
 * delete dynamic FDB entry to global FDB database
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - dynamic fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_dynamic_entry_del
(
    unsigned char* mac,
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    status = dbtable_hash_delete(npd_fdb_hashmac_index, &dbItem, &dbItem);

    if (0 != status)
        return FDB_RETURN_CODE_HASH_OP_FAIL;

    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_dynamic_entry_sticky
 *
 * Conver a dynamic FDB entry to be a sticky entry
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		FDB_RETURN_CODE_SUCCESS - OK
 *
 **********************************************************************************/
int npd_fdb_dynamic_entry_sticky
(
    hash_table_index_t *hash,
    void *in,
    unsigned int flag
)
{
    struct fdb_entry_item_s *dbItem = (struct fdb_entry_item_s *)in;
    int status = 0;
	
	if(dbItem->isStatic || dbItem->isBlock || dbItem->isMirror)
	{
		return FDB_RETURN_CODE_SUCCESS;
	}
	
	dbItem->isStatic = 1;
	
    status = dbtable_hash_update(npd_fdb_hashmac_index, dbItem, dbItem);

    if (0 != status)
        return FDB_RETURN_CODE_HASH_OP_FAIL;

    return FDB_RETURN_CODE_SUCCESS;
}
/**********************************************************************************
 * npd_fdb_static_entry_del
 *
 * delete static FDB entry to global FDB database
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_static_entry_del
(
    unsigned char* mac,
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isMirror)
        {
            syslog_ax_fdb_err("The item is mirror , can not delete directly!\n");
            return FDB_RETURN_CODE_ITEM_ISMIRROR;
        }
        else if (dupItem.isBlock)
        {
            syslog_ax_fdb_err("The item is block , can not delete directly!\n");
            return FDB_RETURN_CODE_GENERAL;
        }
        else if (dupItem.isStatic)
        {
            status = dbtable_hash_delete(npd_fdb_hashmac_index, &dbItem, &dbItem);

            if (0 != status)
                return FDB_RETURN_CODE_HASH_OP_FAIL;
        }
    }
    else
    {
        syslog_ax_fdb_err("null node found when delete static fdb entry");
        return FDB_RETURN_CODE_GENERAL;
    }

    return FDB_RETURN_CODE_SUCCESS;
}

unsigned int npd_fdb_static_mirror_entry_add
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int eth_g_index,
    unsigned int  profile
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    int fdb_count = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    dbItem.ifIndex = eth_g_index;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    dbItem.isMirror = 1;
    dbItem.mirrorProfile = profile;
    fdb_count = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, NULL, 0, npd_fdb_static_filter);

    if (fdb_count >= AX_STATIC_FDB_ENTRY_SIZE)
    {
        syslog_ax_fdb_event("npd static mirror fdb database overrun.\n");
        return FDB_RETURN_CODE_MAX;
    }

    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isBlock)
        {
            syslog_ax_fdb_err("The item has exist as one blacklist item. Please delete it first \n");
            return NPD_ERR;
        }

        if (dupItem.isStatic)
        {
            if (!dupItem.isMirror)
            {
                dupItem.isMirror = 1;
                status = dbtable_hash_update(npd_fdb_hashmac_index, &dupItem, &dupItem);
            }
            else
            {
                syslog_ax_fdb_err("The item has exist as one static item. Please delete it first \n");
                return NPD_ERR;
            }
        }
    }
    else
    {
        status = dbtable_hash_insert(npd_fdb_hashmac_index, &dbItem);
    }

    if (0 != status)
        return NPD_ERR;

    return NPD_OK;
}
int npd_mirror_fdb_source_port_exist_check(unsigned short valnid, unsigned char *mac, unsigned int eth_g_index, unsigned int profile)
{
    int status = 0;
    struct fdb_entry_item_s dbItem;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.vlanid = valnid;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dbItem);

    if (0 == status)
    {
        if (dbItem.ifIndex == eth_g_index)
        {
            return NPD_OK;
        }
    }

    return NPD_ERR;
}

unsigned int npd_fdb_check_static_mirror_entry_exist
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int * profile
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isMirror)
            *profile = dupItem.mirrorProfile;

        return NPD_OK;
    }

    return NPD_ERR;
}

unsigned int npd_fdb_mirror_cmp
(
    void * in1,
    void * in2
)
{
    return npd_fdb_static_filter_by_mirror_profile(in1, in2);
}

int npd_fdb_check_contain_mirror(unsigned int profile)
{
    struct fdb_entry_item_s dbItem;
    int status;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.isMirror = 1;
    dbItem.isStatic = 1;
    dbItem.mirrorProfile = profile;
    status = dbtable_hash_head(npd_fdb_hashmac_index, &dbItem, &dbItem, npd_fdb_mirror_cmp);

    if (0 != status)
        return NPD_ERR;

    return NPD_OK;
}

unsigned int npd_fdb_static_mirror_entry_del
(
    unsigned char* mac,
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    dbItem.isMirror = 1;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        syslog_ax_fdb_dbg("fdb mirror entry del: find entry %.2x:%.2x:%.2x:%.2x:%.2x:%.2x vlan %d, mirror flag %d\n",\
                          dupItem.mac[0],dupItem.mac[1],dupItem.mac[2],dupItem.mac[3],\
                          dupItem.mac[4],dupItem.mac[5],dupItem.vlanid,dupItem.isMirror);

        if (dupItem.isMirror)
        {
            status = dbtable_hash_delete(npd_fdb_hashmac_index, &dupItem, &dupItem);

            if (0 != status)
                return NPD_ERR;
        }
    }
    else
    {
        syslog_ax_fdb_err("null node found when delete static fdb entry");
        return NPD_ERR;
    }

    return NPD_OK;
}

/**********************************************************************************
 * npd_fdb_static_mirror_entry_del_by_profile
 *
 * del FDB entry from global FDB database
 *
 *	INPUT:
 *		item - struct fdb_entry_item_s
 *		flag  - operation flag
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		ret - NPD_FDB_ERR_NONE
 *                 FDB_RETURN_CODE_HASH_OP_FAIL
 *
 **********************************************************************************/

int npd_fdb_static_mirror_entry_del_by_profile(unsigned int profile)
{
    struct fdb_entry_item_s dbItem;
    int status;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.isMirror = 1;
    dbItem.isStatic = 1;
    dbItem.mirrorProfile = profile;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 2, &dbItem,npd_fdb_static_filter_by_mirror_profile,\
                                    npd_fdb_entry_del);
    syslog_ax_fdb_dbg("FDB mirror delete by profile %d: delete %d entries\n", profile, status);
    return NPD_OK;
}

int npd_fdb_static_mirror_entry_count(unsigned int profile)
{
    struct fdb_entry_item_s dbItem;
    int count = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.isMirror = 1;
    dbItem.isStatic = 1;
    dbItem.mirrorProfile = profile;
    count = dbtable_hash_traversal(npd_fdb_hashmac_index, TRUE, &dbItem,npd_fdb_static_filter_by_mirror_profile,\
                                   NULL);
    return count;
}


/**********************************************************************************
 * npd_fdb_static_blacklist_entry_add
 *
 * add static FDB entry to global FDB database
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *		eth_index - global eth port index
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/

unsigned int npd_fdb_static_blacklist_entry_add
(
    unsigned char *mac,
    unsigned short vid,
    unsigned char flag
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    int fdb_count = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    dbItem.isBlock = 1;
    //dbItem.ifIndex = npd_netif_vlan_get_index(vid);
    dbItem.ifIndex = 0;
    dbItem.blockMode = NPD_FDB_BLACKLIST_MODE_GET(flag);
    fdb_count = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, NULL, 0, npd_fdb_blacklist_filter);

    if (fdb_count >= AX_STATIC_FDB_ENTRY_SIZE)
    {
        syslog_ax_fdb_err("npd blacklist fdb database overrun");
        return FDB_RETURN_CODE_MAX;
    }

    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isMirror)
        {
            syslog_ax_fdb_err("The item has exist as mirror item. Please delete it first \n");
            return FDB_RETURN_CODE_NODE_EXIST;
        }

        if (dupItem.isBlock)
        {
            if (dupItem.blockMode & NPD_FDB_BLACKLIST_MODE_GET(flag))
            {
                syslog_ax_fdb_err("The item has exist\n");
                return FDB_RETURN_CODE_NODE_EXIST;
            }

            dupItem.blockMode |= NPD_FDB_BLACKLIST_MODE_GET(flag);
            status = dbtable_hash_update(npd_fdb_hashmac_index, &dupItem, &dupItem);

            if (0 != status)
            {
                return FDB_RETURN_CODE_HASH_OP_FAIL;
            }

            return FDB_RETURN_CODE_SUCCESS;
        }
        else
        {
            status = dbtable_hash_delete(npd_fdb_hashmac_index, &dupItem, &dupItem);

            if (0 != status)
            {
                return FDB_RETURN_CODE_HASH_OP_FAIL;
            }
        }
    }

    status = dbtable_hash_insert(npd_fdb_hashmac_index, &dbItem);

    if (0 != status)
        return FDB_RETURN_CODE_HASH_OP_FAIL;

    return FDB_RETURN_CODE_SUCCESS;
}


/**********************************************************************************
 * npd_fdb_static_blacklist_entry_del
 *
 * delete static FDB entry to global FDB database
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *		eth_index - global eth port index
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_static_blacklist_entry_del
(
    char* mac,
    unsigned short vid,
    unsigned char flag
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    dbItem.blockMode = NPD_FDB_BLACKLIST_MODE_GET(flag);
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isBlock)
        {
            if (dupItem.blockMode & NPD_FDB_BLACKLIST_MODE_GET(flag))
            {
                dupItem.blockMode &= ~(NPD_FDB_BLACKLIST_MODE_GET(flag));

                if (dupItem.blockMode == 0)
                    status = dbtable_hash_delete(npd_fdb_hashmac_index, &dbItem, &dbItem);
                else
                    status = dbtable_hash_update(npd_fdb_hashmac_index, &dupItem, &dupItem);

                if (0 != status)
                    return FDB_RETURN_CODE_HASH_OP_FAIL;
            }
        }
    }
    else
    {
        syslog_ax_fdb_err("null node found when delete static fdb entry");
        return FDB_RETURN_CODE_GENERAL;
    }

    return FDB_RETURN_CODE_SUCCESS;
}



/**********************************************************************************
 * npd_fdb_static_blacklist_entry_del_by_vlan
 *
 * delete static FDB entry to global FDB database
 *
 *	INPUT:
 *		vid - vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/

unsigned int npd_fdb_static_blacklist_entry_del_by_vlan
(
    unsigned short vlanId
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vlanId;
    dbItem.isStatic = 1;
    dbItem.isBlock = 1;
    status = dbtable_hash_traversal(npd_fdb_hashmac_index, 3, &dbItem, \
                                    npd_fdb_blacklist_filter_by_vlan, npd_fdb_entry_del);
    syslog_ax_fdb_dbg("FDB blacklist delete by vlan %d: delete %d entries\n", vlanId, status);
    return FDB_RETURN_CODE_SUCCESS;
}

/**********************************************************************************
 * npd_fdb_static_authen_entry_add
 *
 * add static FDB entry to global FDB database
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *		eth_index - global eth port index
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/

unsigned int npd_fdb_static_authen_entry_add
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int eth_g_index
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    int fdb_count = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    dbItem.ifIndex = eth_g_index;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    dbItem.isAuthen = 1;
    fdb_count = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, NULL, 0, npd_fdb_static_filter);

    if (fdb_count >= AX_STATIC_FDB_ENTRY_SIZE)
    {
        syslog_ax_fdb_event("npd static mirror fdb database overrun.\n");
        return FDB_RETURN_CODE_MAX;
    }

    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status && dupItem.isStatic)
    {
        syslog_ax_fdb_err("The item has exist as one static item. Please delete it first \n");
        return FDB_RETURN_CODE_NODE_EXIST;
    }

    status = dbtable_hash_insert(npd_fdb_hashmac_index, &dbItem);

    if (0 != status)
        return FDB_RETURN_CODE_HASH_OP_FAIL;

    return FDB_RETURN_CODE_SUCCESS;
}


unsigned int npd_fdb_check_static_authen_entry_exist
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int * eth_g_index
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isAuthen)
        {
            *eth_g_index = dupItem.ifIndex;
            return NPD_FDB_ERR_NODE_EXIST;
        }
    }

    return FDB_RETURN_CODE_NODE_NOT_EXIST;
}

/**********************************************************************************
 * npd_fdb_static_authen_entry_del
 *
 * delete static FDB entry to global FDB database
 *
 *	INPUT:
 *		mac -mac address
 *		vid - vlan id
 *
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		node - static fdb entry info stucture
 *
 **********************************************************************************/
unsigned int npd_fdb_static_authen_entry_del
(
    unsigned char* mac,
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
    dbItem.isAuthen = 1;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isStatic & dupItem.isAuthen)
        {
            status = dbtable_hash_delete(npd_fdb_hashmac_index, &dbItem, &dbItem);

            if (0 != status)
                return FDB_RETURN_CODE_HASH_OP_FAIL;
        }
    }
    else
    {
        syslog_ax_fdb_err("null node found when delete static fdb entry");
        return FDB_RETURN_CODE_GENERAL;
    }

    return FDB_RETURN_CODE_SUCCESS;
}


unsigned int npd_fdb_static_vrrp_entry_add
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int eth_g_index
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    int fdb_count = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    dbItem.ifIndex = eth_g_index;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
	dbItem.flagApp = FDB_APP_FLAG_VRRP;
	
    fdb_count = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, NULL, 0, npd_fdb_static_filter);

    if (fdb_count >= AX_STATIC_FDB_ENTRY_SIZE)
    {
        syslog_ax_fdb_event("npd static mirror fdb database overrun.\n");
        return FDB_RETURN_CODE_MAX;
    }

    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        syslog_ax_fdb_err("The item has exist as one blacklist item. Please delete it first \n");
        return FDB_RETURN_CODE_NODE_EXIST;
    }

    status = dbtable_hash_insert(npd_fdb_hashmac_index, &dbItem);

    if (0 != status)
        return FDB_RETURN_CODE_HASH_OP_FAIL;

    return FDB_RETURN_CODE_SUCCESS;
}


unsigned int npd_fdb_static_vrrp_entry_del
(
    unsigned char* mac,
    unsigned short vid
)
{
    struct fdb_entry_item_s dbItem, dupItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    memset(&dupItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.vlanid = vid;
    memcpy(dbItem.mac, mac, MAC_ADDRESS_LEN);
    dbItem.isStatic = 1;
	dbItem.flagApp = FDB_APP_FLAG_VRRP;
    status = dbtable_hash_search(npd_fdb_hashmac_index, &dbItem, NULL, &dupItem);

    if (0 == status)
    {
        if (dupItem.isStatic && 
			(dupItem.flagApp == FDB_APP_FLAG_VRRP))
        {
            status = dbtable_hash_delete(npd_fdb_hashmac_index, &dbItem, &dbItem);

            if (0 != status)
                return FDB_RETURN_CODE_HASH_OP_FAIL;
        }
    }
    else
    {
        syslog_ax_fdb_err("null node found when delete static fdb entry");
        return FDB_RETURN_CODE_GENERAL;
    }

    return FDB_RETURN_CODE_SUCCESS;
}


unsigned int npd_show_fdb_count(void)
{
	unsigned int ret = 0;
	ret = dbtable_hash_count(npd_fdb_hashmac_index);
	return ret;
}

unsigned int npd_fdb_get_one_item
(
    struct fdb_entry_item_s *fdb_item,
	unsigned char macAddr[6],
	unsigned short	vlanId
)
{
	struct fdb_entry_item_s dbItem;	
	int ret = 0;
	
	memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
	dbItem.vlanid = vlanId;
	memcpy(dbItem.mac, macAddr, 6);
	

	ret = dbtable_hash_search( npd_fdb_hashmac_index, &dbItem, NULL, fdb_item);
	    
	return ret;		
}

unsigned int npd_fdb_get_dynamic_item
(
    struct fdb_entry_item_s *fdb_array,
    unsigned int   size
)
{ 
	struct fdb_entry_item_s dbItem;
	int count = 0;
	

	memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));

	count = dbtable_hash_return( npd_fdb_hashmac_index, &dbItem, fdb_array, size, npd_fdb_dynamic_filter);
	    
	return count;	
}

unsigned int npd_fdb_get_item_by_netif_index
(
    struct fdb_entry_item_s *fdb_array,
    unsigned int   size,
    unsigned int   netif_index
)
{ 
	struct fdb_entry_item_s dbItem;
	int count = 0;
	

	memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
	dbItem.ifIndex = netif_index;

	count = dbtable_hash_return( npd_fdb_hashmac_index, &dbItem, fdb_array, size, npd_fdb_netif_index_filter);
	    
	return count;	
}

unsigned int npd_fdb_get_item_by_vlan
(
    struct fdb_entry_item_s *fdb_array,
    unsigned int   size,
    unsigned short vlan
)
{ 
	struct fdb_entry_item_s dbItem;
	int count = 0;
	

	memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
	dbItem.vlanid = vlan;

	count = dbtable_hash_return( npd_fdb_hashmac_index, &dbItem, fdb_array, size, npd_fdb_vlan_filter);
	    
	return count;	
}

unsigned int npd_fdb_get_item_by_mac
(
    struct fdb_entry_item_s *fdb_array,
    unsigned int   	size,
    unsigned char 	macAddr[6]
)
{ 
	struct fdb_entry_item_s dbItem;
	int count = 0;
	

	memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
	memcpy(dbItem.mac, macAddr, 6);

	count = dbtable_hash_return( npd_fdb_hashmac_index, &dbItem, fdb_array, size, npd_fdb_mac_filter);
	    
	return count;	
}

unsigned int npd_fdb_get_all_item
(
    struct fdb_entry_item_s *fdb_array,
    unsigned int   size
)
{ 
	struct fdb_entry_item_s dbItem;	
	int count = 0;
	
	memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));


	//count = dbtable_hash_count(npd_fdb_hashmac_index);

	count = dbtable_hash_return( npd_fdb_hashmac_index, (void *)&dbItem, fdb_array, size, npd_fdb_all_filter);
	    
	return count;	
}

unsigned int npd_fdb_get_blacklist_item
(
    struct fdb_entry_item_s *blacklist_array,
    unsigned int   size
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.isStatic = 1;
    dbItem.isBlock = 1;
    status = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, blacklist_array, size, npd_fdb_blacklist_filter);
    return status;
}

unsigned int npd_fdb_get_static_item
(
    struct fdb_entry_item_s *static_array,
    unsigned int   size
)
{
    struct fdb_entry_item_s dbItem;
    int count = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.isStatic = 1;
    count = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, static_array, size, npd_fdb_static_filter);
    return count;
}

unsigned int npd_fdb_get_static_mirror_item
(
    struct fdb_entry_item_s *static_mirror_array,
    unsigned int   size,
    unsigned int profile
)
{
    struct fdb_entry_item_s dbItem;
    int status = 0;
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.isStatic = 1;
    dbItem.isMirror = 1;
    dbItem.mirrorProfile = profile;
    status = dbtable_hash_return(npd_fdb_hashmac_index, &dbItem, static_mirror_array, size, \
                                 npd_fdb_static_filter_by_mirror_profile);
    return status;
}


/********************************************************************/
/* functions for DLDP															*/
/********************************************************************/
/**********************************************************************************
 * npd_fdb_add_dldp_vlan_system_mac()
 *	DESCRIPTION:
 *		add a FDB item(system mac and vlanId) when enable DLDP on vlan
 *
 *	INPUTS:
 *		unsigned short vlanId
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NPD_FDB_ERR_VLAN_NONEXIST	- vlan not exist
 *		NPD_FDB_ERR_NONE			- success
 *		NPD_FDB_ERR_OCCUR_HW		- error when add FDB item to table FDB
***********************************************************************************/
unsigned int npd_fdb_add_dldp_vlan_system_mac
(
    unsigned short vlanId
)
{
    unsigned int ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int intf_flag = NPD_FALSE;
    unsigned int ifindex = 0;

    if (NPD_FALSE == npd_check_vlan_exist(vlanId))
    {
        syslog_ax_fdb_err("add dldp fdb, vlan %d not exist\n", vlanId);
        ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
    }
    else
    {
        ret = npd_vlan_interface_check(vlanId, &ifindex);

        if ((NPD_TRUE == ret) && (ifindex != (~0UI)))
        {
            npd_syslog_dbg("add dldp fdb, vlan %d is L3 vlan interface %d\n", vlanId, ifindex);
            intf_flag = NPD_TRUE;
        }

        ret = nam_fdb_static_system_source_mac_add(vlanId, intf_flag);

        if (FDB_CONFIG_SUCCESS != ret)
        {
            syslog_ax_fdb_err("add dldp fdb vlan %d, HW error\n", vlanId);
            ret = FDB_RETURN_CODE_OCCUR_HW;
        }
    }

    return ret;
}

/**********************************************************************************
 * npd_fdb_del_dldp_vlan_system_mac()
 *	DESCRIPTION:
 *		del the  FDB item(system mac and vlanId) when enable DLDP on vlan
 *
 *	INPUTS:
 *		unsigned short vlanId
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NPD_FDB_ERR_VLAN_NONEXIST	- vlan not exist
 *		NPD_FDB_ERR_NONE			- success
 *		NPD_FDB_ERR_OCCUR_HW		- error when del FDB item to table FDB
***********************************************************************************/
unsigned int npd_fdb_del_dldp_vlan_system_mac
(
    unsigned short vlanId
)
{
    unsigned int ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int intf_flag = NPD_FALSE;
    unsigned int ifindex = 0;

    if (NPD_FALSE == npd_check_vlan_exist(vlanId))
    {
        syslog_ax_fdb_err("del dldp fdb, vlan %d not exist\n", vlanId);
        ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
    }
    else
    {
        ret = npd_vlan_interface_check(vlanId, &ifindex);

        if ((NPD_TRUE == ret) && (ifindex != (~0UI)))
        {
            npd_syslog_dbg("del dldp fdb, vlan %d is L3 vlan interface %d\n", vlanId, ifindex);
            intf_flag = NPD_TRUE;
        }

        ret = nam_fdb_static_system_source_mac_del(vlanId, intf_flag);

        if (FDB_CONFIG_SUCCESS != ret)
        {
            syslog_ax_fdb_err("del dldp fdb vlan %d, HW error\n", vlanId);
            ret = FDB_RETURN_CODE_OCCUR_HW;
        }
    }

    return ret;
}


#ifdef HAVE_VRRP
/**********************************************************************************
 *  npd_fdb_create_for_vrrp
 *
 *	DESCRIPTION:
 * 		this routine create vrrp and set static FDB
 *
 *	INPUT:
 *		ifindex	-- intf id
 *		name  -- dev name
 *		addr	  --  system mac address
 *
 *	OUTPUT:
 *
 *
 * 	RETURN:
 *		NPD_DBUS_ERROR
 *		NPD_SUCCESS
 *
 *
 **********************************************************************************/
int npd_fdb_create_for_vrrp
(
    unsigned char* ifname
)
{
    int ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int ifIndex = 0,eth_g_index = 0;
    unsigned short vid;

    if (NPD_OK != (ret = npd_intf_get_info((char *)ifname,&ifIndex,&vid,&eth_g_index)))
    {
        syslog_ax_fdb_err("get interface %s info error\n",ifname);
        return FDB_RETURN_CODE_GENERAL;
    }

    if (0 == ret)
    {
        if (0 != (ret = nam_static_fdb_entry_mac_set_for_vrrp(vid)))
        {
            syslog_ax_intf_err("set static mac addr ERROR ret %d\n",ret);
            ret = FDB_RETURN_CODE_GENERAL;
        }
        else
        {
            ret = FDB_RETURN_CODE_SUCCESS;
        }
    }

    return ret;
}


/**********************************************************************************
 *  npd_fdb_del_for_vrrp
 *
 *	DESCRIPTION:
 * 		this routine del vrrp and set static FDB
 *
 *	INPUT:
 *		ifindex	-- intf id
 *		name  -- dev name
 *		addr	  --  system mac address
 *
 *	OUTPUT:
 *
 *
 * 	RETURN:
 *		NPD_DBUS_ERROR
 *		NPD_SUCCESS
 *
 *
 **********************************************************************************/
int npd_fdb_del_for_vrrp
(
    unsigned char* ifname
)
{
    int ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int ifIndex = 0,eth_g_index = 0;
    unsigned short vid;

    if (NPD_OK != (ret = npd_intf_get_info((char *)ifname,&ifIndex,&vid,&eth_g_index)))
    {
        syslog_ax_fdb_err("get interface %s info error\n",ifname);
        return FDB_RETURN_CODE_GENERAL;
    }

    if (0 == ret)
    {
        if (0 != (ret = nam_static_fdb_entry_mac_del_for_vrrp(vid)))
        {
            syslog_ax_intf_err("set static mac addr ERROR ret %d\n",ret);
            ret = FDB_RETURN_CODE_GENERAL;
        }
        else
        {
            ret = FDB_RETURN_CODE_SUCCESS;
        }
    }

    return ret;
}
#endif
/***************************************************************************************************
 *		FDB netif index operation
 *
 ***************************************************************************************************/
void npd_fdb_notify_event
(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT evt,
    char *private, int len
)
{
    int type = npd_netif_type_get(netif_index);
    unsigned int vid = npd_netif_vlan_get_vid(netif_index);
    syslog_ax_fdb_dbg("npd notify fdb index event: index 0x%x event %d\n", netif_index, evt);

    if ((type == NPD_NETIF_ETH_TYPE)
            ||(type == NPD_NETIF_TRUNK_TYPE))
    {
        switch (evt)
        {
            case PORT_NOTIFIER_L2CREATE:
                /*npd_fdb_entry_del_by_port(netif_index);*/
                break;
            case PORT_NOTIFIER_LINKUP_E:
            case PORT_NOTIFIER_INSERT:
            case PORT_NOTIFIER_STPTC:
                break;
            case PORT_NOTIFIER_DISCARD:
            case PORT_NOTIFIER_LINKDOWN_E:
            case PORT_NOTIFIER_REMOVE:
                npd_fdb_dynamic_entry_del_by_port(netif_index);
                /*fdb also deleted in eth port update*/
                break;
            case PORT_NOTIFIER_L2DELETE:
                npd_fdb_static_entry_del_by_port(netif_index);
                npd_fdb_dynamic_entry_del_by_port(netif_index);
                /*fdb also deleted in switchport deleting*/
                npd_fdb_number_port_set(netif_index, -1);
                break;
            default:
                break;
        }
    }

    if (type == NPD_NETIF_VLAN_TYPE)
    {
        switch (evt)
        {
            case PORT_NOTIFIER_DELETE:
                npd_fdb_static_entry_del_by_vlan((unsigned short)vid);
				npd_fdb_blacklist_entry_del_by_vlan((unsigned short)vid);
                break;
            default:
                break;
        }
    }

    return;
}

int npd_fdb_agingtime(unsigned int agingtime)
{
    int ret = 0;
    struct npd_fdb_cfg_s npd_fdb_cfg_set = {0};
    npd_fdb_cfg_set.agingtime = agingtime;
    ret = dbtable_array_update(npd_fdb_cfg_index, 0, &npd_fdb_cfg_set, &npd_fdb_cfg_set);
    return ret;
}

void npd_fdb_relate_event
(
    unsigned int vlan_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event,
    char *private, int len
)
{
    int type = npd_netif_type_get(netif_index);
    unsigned long vlan_type = npd_netif_type_get(vlan_index);
    unsigned int vid = npd_netif_vlan_get_vid(vlan_index);
    syslog_ax_fdb_dbg("npd notify fdb relate event: vlan 0x%x index 0x%x event %d\n", \
                      vlan_index, netif_index, event);

    if (NPD_NETIF_VLAN_TYPE != vlan_type)
    {
        if (NPD_NETIF_TRUNK_TYPE == vlan_type && (type == NPD_NETIF_ETH_TYPE))
        {
            if (event == PORT_NOTIFIER_JOIN)
            {
                npd_fdb_entry_del_by_port(netif_index);
            }
        }

        return;
    }

    if (NPD_NETIF_ETH_TYPE != type && NPD_NETIF_TRUNK_TYPE != type)
    {
        return;
    }

    switch (event)
    {
        case PORT_NOTIFIER_JOIN:
            break;
        case PORT_NOTIFIER_LEAVE:
            npd_fdb_static_entry_del_by_vlan_port(vid, netif_index);
            npd_fdb_dynamic_entry_del_by_vlan_port(vid, netif_index);
            break;
        default:
            syslog_ax_fdb_err("unknown relate event type\n");
            break;
    }

    return;
}
struct timeval last_over_time;
int npd_fdb_learning(struct fdb_entry_item_s *entry, int isAdd)
{
    int status = 0;
    struct vlan_s *vlan = NULL;
    struct switch_port_db_s *switch_port = NULL;
    struct fdb_entry_item_s old_entry = {0};
    int ret;
    struct timeval tnow;
    struct timezone tzone;
	
    switch_port = malloc(sizeof(struct switch_port_db_s));

    if (NULL == switch_port)
        goto error;

    memset(switch_port, 0, sizeof(struct switch_port_db_s));
    switch_port->global_port_ifindex = entry->ifIndex;
    ret = dbtable_hash_search(switch_ports_hash, switch_port, NULL, switch_port);

    if ((ret == 0) && 
		(switch_port->fdb_learning_mode == 0 || 
		/*
		switch_port->fdb_learning_mode == PORT_SECURITY_MAC_AUTH ||*/
		switch_port->fdb_learning_mode == 4))/*PORT_SECURITY_STICKY*/
    {
        char name[50];
		if(switch_port->fdb_learning_mode == 4)/*PORT_SECURITY_STICKY*/
		{
			entry->isStatic = 1;
		}
		else
		{
			/*
			if(switch_port->fdb_learning_mode == PORT_SECURITY_MAC_AUTH)
			{
			    npd_asd_send_fdb(entry->ifIndex, entry->vlanid, entry->mac);
			}
			*/
            gettimeofday(&tnow, &tzone);
    
            if (tnow.tv_sec - last_over_time.tv_sec >= 1)
            {
                npd_netif_index_to_user_fullname(entry->ifIndex, name);
                npd_syslog_official_event("Protect switch port %s receive new address %.02x:%.02x:%.02x:%.02x:%.02x:%.02x\n",
                                          name, entry->mac[0],entry->mac[1],entry->mac[2],entry->mac[3],entry->mac[4],
                                          entry->mac[5]);
            }
    
            last_over_time.tv_sec = tnow.tv_sec;
            free(switch_port);
            return FDB_RETURN_CODE_NODE_EXIST;
		}
    }
#if 0
#ifdef HAVE_AAA
    if( NPD_TRUE == npd_asd_check_authMode_MAB(entry->ifIndex))
    {
        npd_asd_send_new_mac_auth(entry->ifIndex, entry->vlanid, entry->mac);		
		//TODO: prevent same mac flood to CPU 
		entry->isBlock = 1;
		entry->blockMode = NPD_FDB_BLACKLIST_SMAC_MODE;
    }
#endif
#endif
    status = dbtable_hash_search(npd_fdb_hashmac_index, entry, NULL, &old_entry);

    if (0 == status)
    {
        if (old_entry.isBlock)
        {
            syslog_ax_fdb_err("The item has exist as one blacklist item. Please delete it first \n");
            free(switch_port);
            return FDB_RETURN_CODE_NODE_EXIST;
        }

        if (old_entry.isStatic)
        {
            syslog_ax_fdb_err("The item has exist as one static item. Please delete it first \n");
            free(switch_port);
            return FDB_RETURN_CODE_NODE_EXIST;
        }

        if (old_entry.isMirror)
        {
            free(switch_port);
            return FDB_RETURN_CODE_NODE_EXIST;
        }

        if (old_entry.isAuthen)
        {
            free(switch_port);
            return FDB_RETURN_CODE_NODE_EXIST;
        }
    }
	
    status = npd_system_verify_basemac((char *)entry->mac);

    if (1 == status)
    {
        free(switch_port);
        return FDB_RETURN_CODE_SUCCESS;
    }


    if (FALSE == isAdd)
    {
        if (status != 0)
        {
            free(switch_port);
            return 0;
        }

        status = dbtable_hash_delete(npd_fdb_hashmac_index, entry, entry);
        free(switch_port);
        return status;
    }

    vlan = npd_find_vlan_by_vid(entry->vlanid);

    if (NULL == vlan)
        goto error;

    if (vlan->fdb_limit >= 0)
    {
        unsigned int count;
        npd_fdb_fdbCount_by_vlan(vlan->vid, &count);

        if (count >= vlan->fdb_limit)
        {
            gettimeofday(&tnow, &tzone);

            if (tnow.tv_sec - last_over_time.tv_sec >= 1)
            {
                npd_syslog_official_event("VLAN %d receive more new address than limitation %d\n",
                                          vlan->vid, vlan->fdb_limit);
            }

            last_over_time.tv_sec = tnow.tv_sec;
            goto error;
        }
    }

    if ((ret == 0) && (switch_port->fdb_limit >= 0))
    {
        unsigned int count;
        npd_fdb_fdbCount_by_port(entry->ifIndex, &count);

        if (count >= switch_port->fdb_limit)
        {
            char name[50];
            gettimeofday(&tnow, &tzone);

            if (tnow.tv_sec - last_over_time.tv_sec >= 1)
            {
                npd_netif_index_to_user_fullname(entry->ifIndex, name);
                npd_syslog_official_event("Switch port %s receive more new address than limitation %d\n",
                                          name, switch_port->fdb_limit);
            }

            last_over_time.tv_sec = tnow.tv_sec;
            goto error;
        }
    }

    entry->time = time(NULL);
    free(switch_port);
    free(vlan);
#if 0
#ifdef HAVE_VRRP
	if (isAdd && (NPD_TRUE == npd_vrrp_vlan_vmac_check(entry->vlanid, vmac))
		&& !memcmp(entry->mac, vmac, MAC_ADDRESS_LEN))
	{
		/* if the device is vrrp master, can't add vlan mac in fdb */
		return FDB_RETURN_CODE_SUCCESS;		
	}
#endif
#endif
    status = dbtable_hash_insert(npd_fdb_hashmac_index, entry);

    if (0 != status)
        return FDB_RETURN_CODE_HASH_OP_FAIL;

    return FDB_RETURN_CODE_SUCCESS;
error:

    if (switch_port)
        free(switch_port);

    if (vlan)
        free(vlan);

    return FDB_RETURN_CODE_HASH_OP_FAIL;
}

int npd_fdb_aging(struct fdb_entry_item_s entry)
{
    int status;
    status = dbtable_hash_delete(npd_fdb_hashmac_index, &entry, &entry);
    return FDB_RETURN_CODE_SUCCESS;
}
/***************************************************************************************************
 *		NPD dbus operation
 *
 ***************************************************************************************************/

DBusMessage * npd_dbus_fdb_config_agingtime(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    /*unsigned short	agingtime = 0;*/
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&g_agingtime,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    npd_syslog_dbg("configure fdb table agingtime %d . \n",g_agingtime);

    if (CHASSIS_AGINGTIME_ISLEGAL(g_agingtime))
    {
        ;/*npd_syslog_dbg("legal agingtime %d",g_agingtime);*/
    }
    else
    {
        npd_syslog_dbg("Illegal agingtime %d",g_agingtime);
        ret = FDB_RETURN_CODE_BADPARA;
        /*better to tell user to input vlanId again*/
    }

    /*
     *add fdb entry config op
     *this op set hw chip
    */
    ret = npd_fdb_agingtime(g_agingtime);

    if (ret != FDB_RETURN_CODE_SUCCESS)
    {
        npd_syslog_dbg("npd_dbus_fdb_config_agingtime:: fdb agingtime %d set ERROR. \n",g_agingtime);
        ret = FDB_RETURN_CODE_OCCUR_HW;
        /*ret = NPD_DBUS_ERROR; in this case dbus message can not reply*/
    }

    /*to update the fdb info data struct in sw */
    /*fdb active op ,need to modify the vid_list only*/
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &ret);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &g_agingtime);
    return reply;
}


DBusMessage * npd_dbus_fdb_no_config_agingtime(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    /*unsigned short	agingtime;*/
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&g_agingtime,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    if (CHASSIS_AGINGTIME_ISLEGAL(g_agingtime))
    {
    }
    else
    {
        syslog_ax_fdb_err("Illegal agingtime %d",g_agingtime);
        ret = FDB_RETURN_CODE_BADPARA;
    }

    /*
     *add fdb entry config op
     *this op set hw chip
    */
    ret = npd_fdb_agingtime(g_agingtime);

    if (ret != FDB_RETURN_CODE_SUCCESS)
    {
        npd_syslog_dbg("npd_dbus_fdb_config_agingtime:: fdb agingtime %d default ERROR. \n",g_agingtime);
        ret = FDB_RETURN_CODE_OCCUR_HW;
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&g_agingtime);
    return reply;
}

DBusMessage * npd_dbus_fdb_delete_blacklist(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned short	vlanId;
    unsigned char   macAddr[6] = {0};
    int result = 0;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int flag = 3;
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&flag,
                                DBUS_TYPE_UINT16,&vlanId,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args!\n ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s\n",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            if (NPD_TRUE != npd_check_vlan_exist(vlanId))
            {
                syslog_ax_fdb_err("the vlanname is not register!\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                ret = npd_fdb_static_blacklist_entry_del((char *)macAddr,vlanId,flag);

                if (ret != FDB_RETURN_CODE_SUCCESS)
                {
                    syslog_ax_fdb_err("npd_fdb_static_blacklist_entry_del ERROR return value is:%d\n",ret);
                    ret = FDB_RETURN_CODE_GENERAL;
                }
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_delete_blacklist_with_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    unsigned short	vlanId = 0;
    struct vlan_s* node = NULL;
    char* vlanname=NULL;
    unsigned int	ret=FDB_RETURN_CODE_SUCCESS;
    unsigned char   macAddr[6] = {0};
    int result = 0;
    unsigned int   flag =3;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&flag,
                                DBUS_TYPE_STRING,&vlanname,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args!\n ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s\n",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            node= npd_find_vlan_by_name(vlanname);

            if (node == NULL)
            {
                syslog_ax_fdb_err("the vlanname is not register!\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                vlanId = node->vid;

                if (!(CHASSIS_VLANID_ISLEGAL(vlanId)))
                {
                    syslog_ax_fdb_err("illegal vlanId %d\n",vlanId);
                    ret = FDB_RETURN_CODE_BADPARA;
                }
                else
                {
                    ret = npd_fdb_static_blacklist_entry_del((char *)macAddr,vlanId,flag);

                    if (ret != FDB_RETURN_CODE_SUCCESS)
                    {
                        syslog_ax_fdb_err("npd_fdb_static_blacklist_entry_del node NOT exist return value is:%d\n",ret);
                        ret = FDB_RETURN_CODE_GENERAL;
                    }
                }

                free(node);
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32, &ret);
    return reply;
}



DBusMessage * npd_dbus_fdb_create_blacklist(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    unsigned int   flag =3;
    unsigned short	vlanId;
    unsigned char   macAddr[6] = {0};
    int result = 0;
    unsigned int	ret=FDB_RETURN_CODE_SUCCESS;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&flag,
                                DBUS_TYPE_UINT16,&vlanId,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            if (NPD_TRUE != npd_check_vlan_exist(vlanId))
            {
                syslog_ax_fdb_err("the vlan is not register!\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                ret = npd_fdb_static_blacklist_entry_add(macAddr,vlanId,flag);

                if (ret != FDB_RETURN_CODE_SUCCESS)
                {
                    syslog_ax_fdb_err("npd_fdb_static_blacklist_entry_add node exist return value is:%d\n",ret);
                    ret = FDB_RETURN_CODE_GENERAL;
                }
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,	DBUS_TYPE_UINT32, &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_create_blacklist_with_vlanname(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    unsigned int    flag = 3;
    unsigned short	vlanId;
    struct vlan_s* node = NULL;
    char* vlanName=NULL;
    unsigned char   macAddr[6] = {0};
    int result = 0;
    unsigned int	ret=FDB_RETURN_CODE_SUCCESS;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&flag,
                                DBUS_TYPE_STRING,&vlanName,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            node= npd_find_vlan_by_name(vlanName);

            if (node == NULL)
            {
                syslog_ax_fdb_err("the vlanname is not register!\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                vlanId = node->vid;

                if (!(CHASSIS_VLANID_ISLEGAL(vlanId)))
                {
                    syslog_ax_fdb_err("illegal vlanId %d\n",vlanId);
                    ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
                }
                else
                {
                    ret = npd_fdb_static_blacklist_entry_add(macAddr,vlanId,flag);

                    if (ret != FDB_RETURN_CODE_SUCCESS)
                    {
                        syslog_ax_fdb_err("npd_fdb_static_entry_del erro return value is:%d\n",ret);
                        ret = FDB_RETURN_CODE_GENERAL;
                    }
                }

                free(node);
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32, &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_show_agingtime(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    struct npd_fdb_cfg_s npd_fdb_cfg_set;
    unsigned int ret = FDB_RETURN_CODE_SUCCESS;
    ret = dbtable_array_get(npd_fdb_cfg_index, 0, &npd_fdb_cfg_set);

    if (ret != 0)
    {
        ret = FDB_RETURN_CODE_NODE_NOT_EXIST;
    }

    g_agingtime = npd_fdb_cfg_set.agingtime;
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32,  &ret);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32,  &g_agingtime);
    return reply;
}


DBusMessage * npd_dbus_fdb_config_static_fdb_item(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    unsigned short	vlanId = 0;
    unsigned char   macAddr[6] = {0};
    int result = 0;
    unsigned int	eth_g_index = 0;
    unsigned short  trunkId = 0;
    unsigned int     ret=FDB_RETURN_CODE_SUCCESS;
    unsigned char istag = 0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanId,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args!\n ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s\n",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            if (NPD_TRUE != npd_check_vlan_exist(vlanId))
            {
                syslog_ax_fdb_err("the vlan index no exist\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                ret = npd_vlan_check_contain_port(vlanId,eth_g_index, &istag);

                if (ret!=NPD_TRUE)
                {
                    syslog_ax_fdb_err("the port do not belong to the vlan!\n");
                    ret =FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN;
                }
                else if (NPD_SUCCESS== npd_eth_port_get_ptrunkid(eth_g_index,&trunkId))
                {
                    syslog_ax_fdb_err("The port belongs to trunk %d!\n",trunkId);
                    ret =FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN;
                }
                else
                {
                    ret =  npd_fdb_static_entry_add(macAddr,vlanId,eth_g_index,FDB_STATIC_NOMIRROR);

                    if (ret != FDB_RETURN_CODE_SUCCESS)
                    {
                        syslog_ax_fdb_err("npd_fdb_static_entry_add erro! return value is %d\n",ret);
                        ret = FDB_RETURN_CODE_GENERAL;
                    }
                }
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,	DBUS_TYPE_UINT32, &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_config_static_fdb_trunk_item(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned short  trunkId = 0;
    unsigned short  vlanId = 0;
    unsigned char  isTagged = 0;
    unsigned char	 macAddr[6] = {0};
    int result = 0;
    unsigned int	  ret=FDB_RETURN_CODE_SUCCESS;
    dbus_error_init(&err);
    npd_syslog_dbg("enter fdb create static fdb trunk\n");

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanId,
                                DBUS_TYPE_UINT16,&trunkId,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args!\n ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s\n",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            if (NPD_TRUE != npd_check_vlan_exist(vlanId))
            {
                syslog_ax_fdb_err("the vlan index no exist\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                if (TRUNK_RETURN_CODE_TRUNK_EXISTS != npd_check_trunk_exist(trunkId))
                {
                    syslog_ax_fdb_err("Error trunk no input!!\n");
                    ret = FDB_RETURN_CODE_NODE_NOT_EXIST;
                }
                else if (TRUNK_RETURN_CODE_PORT_EXISTS != npd_check_trunk_member_exist(trunkId))
                {
                    syslog_ax_fdb_err("No member in trunk!!\n");
                    ret = FDB_RETURN_CODE_TRUNK_NO_MEMBER;
                }
                else
                {
                    if ((npd_vlan_check_contain_trunk(vlanId,trunkId,&isTagged))||    \
                            (1 == vlanId))
                    {
                        unsigned int netif_index = npd_netif_trunk_get_index(trunkId);
                        ret =	npd_fdb_static_entry_add((unsigned char *)macAddr,vlanId,netif_index, 0);

                        if (ret != FDB_RETURN_CODE_SUCCESS)
                        {
                            syslog_ax_fdb_err("npd_fdb_static_entry_add erro!Return value is %d \n",ret);
                            ret = FDB_RETURN_CODE_GENERAL;
                        }
                    }
                    else
                    {
                        syslog_ax_fdb_err("The vlan does not contain the trunk\n");
                        ret = FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN;
                    }
                }
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,  DBUS_TYPE_UINT32, &ret);
    return reply;
}


DBusMessage * npd_dbus_fdb_config_static_fdb_trunk_with_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned short  trunkId = 0;
    unsigned short  vlanId = 0;
    char*  vlanname = NULL;
    unsigned char  isTagged = 0;
    char   macAddr[6] = {0};
    int result = 0;
    unsigned int	   ret=FDB_RETURN_CODE_SUCCESS;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_STRING,&vlanname,
                                DBUS_TYPE_UINT16,&trunkId,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args!\n ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s\n",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            vlanId = npd_vlan_get_id_by_name(vlanname);

            if (0==vlanId)
            {
                syslog_ax_fdb_err("the vlan index no exist\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                if (TRUNK_RETURN_CODE_TRUNK_EXISTS != npd_check_trunk_exist(trunkId))
                {
                    syslog_ax_fdb_err("Error trunk no input!!\n");
                    ret = FDB_RETURN_CODE_NODE_NOT_EXIST;
                }
                else if (TRUNK_RETURN_CODE_PORT_EXISTS != npd_check_trunk_member_exist(trunkId))
                {
                    syslog_ax_fdb_err("No member in trunk!!\n");
                    ret = NPD_FDB_ERR_PORT_NOTIN_VLAN;
                }
                else
                {
                    if ((npd_vlan_check_contain_trunk(vlanId,trunkId,&isTagged))||	 \
                            (!(npd_vlan_check_contain_trunk(vlanId,trunkId,&isTagged))&&(1 == vlanId)))
                    {
                        unsigned int netif_index = npd_netif_trunk_get_index(trunkId);
                        ret =  npd_fdb_static_entry_add((unsigned char *)macAddr,vlanId,netif_index,0);

                        if (ret != FDB_RETURN_CODE_SUCCESS)
                        {
                            syslog_ax_fdb_err("npd_fdb_static_entry_add erro! Return value is %d\n",ret);
                            ret = FDB_RETURN_CODE_GENERAL;
                        }
                    }
                    else
                    {
                        syslog_ax_fdb_err("The vlan does not contain the trunk\n");
                        ret = FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN;
                    }
                }
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,  DBUS_TYPE_UINT32, &ret);
    return reply;
}



DBusMessage * npd_dbus_fdb_config_static_fdb_with_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter  iter;
    unsigned short  vlanId = 0;
    unsigned char	 macAddr[6] = {0};
    int result = 0;
    unsigned int	 eth_g_index = 0;
    char *vlanname=NULL;
    int ret=FDB_RETURN_CODE_SUCCESS;
    unsigned char istag = 0;
    unsigned short  trunkId = 0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_STRING,&vlanname,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args!\n ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s\n",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            vlanId = npd_vlan_get_id_by_name(vlanname);

            if (0 == vlanId)
            {
                syslog_ax_fdb_err("the vlanname is not register!\n");
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                if (!(CHASSIS_VLANID_ISLEGAL(vlanId)))
                {
                    syslog_ax_fdb_err("illegal vlanId %d\n",vlanId);
                    ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
                }
                else
                {
                    ret = npd_vlan_check_contain_port(vlanId,eth_g_index, &istag);

                    if (ret!=NPD_TRUE)
                    {
                        syslog_ax_fdb_err("the port do not belong to the vlan!\n");
                        ret = FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN;
                    }
                    else if (NPD_SUCCESS== npd_eth_port_get_ptrunkid(eth_g_index,&trunkId))
                    {
                        syslog_ax_fdb_err("The port belongs to trunk %d!\n",trunkId);
                        ret =FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN;
                    }
                    else
                    {
                        ret = npd_fdb_static_entry_add(macAddr,vlanId,eth_g_index,FDB_STATIC_NOMIRROR);

                        if (ret != FDB_RETURN_CODE_SUCCESS)
                        {
                            syslog_ax_fdb_err("npd_fdb_static_entry_add erro!return value is %d \n",ret);
                            ret = FDB_RETURN_CODE_GENERAL;
                        }
                    }
                }
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;
}


DBusMessage * npd_dbus_fdb_delete_static_fdb(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusError err;
    unsigned short	vlanId = 0;
    unsigned char   macAddr[6] = {0};
    int result = 0;
    unsigned int	ret=FDB_RETURN_CODE_SUCCESS;
    memset(macAddr,0,6);
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanId,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        if (NPD_TRUE != npd_check_vlan_exist(vlanId))
        {
            syslog_ax_fdb_err("The vlan %d is not exist!\n",vlanId);
            ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
        }
        else
        {
            result = npd_system_verify_basemac((char *)macAddr);

            if (0 == result)
            {
                ret = npd_fdb_static_entry_del(macAddr,vlanId);

                if (ret != FDB_RETURN_CODE_SUCCESS)
                {
                    syslog_ax_fdb_err("npd_fdb_static_entry_del erro return value is:%d\n",ret);
                    ret = FDB_RETURN_CODE_GENERAL;
                }
            }
            else if (1 == result)
            {
                ret = FDB_RETURN_CODE_SYSTEM_MAC;
            }
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32,  &ret);
    return reply;
}


DBusMessage * npd_dbus_fdb_delete_static_fdb_with_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusError err;
    struct vlan_s* node = NULL;
    unsigned short	vlanId = 0;
    unsigned char   macAddr[6] = {0};
    int result = 0;
    char* vlanname = NULL;
    /*unsigned int g_index = 0;*/
    int	ret=FDB_RETURN_CODE_SUCCESS;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                /*DBUS_TYPE_STRING,&vlanname,*/
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_STRING,&vlanname,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        result = npd_system_verify_basemac((char *)macAddr);

        if (0 == result)
        {
            node = npd_find_vlan_by_name(vlanname);

            if (NULL == node)
            {
                syslog_ax_fdb_err("No this vlan:%s\n",vlanname);
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                vlanId = node->vid;

                if (!(CHASSIS_VLANID_ISLEGAL(vlanId)))
                {
                    syslog_ax_fdb_err("illegal vlanId %d",vlanId);
                    ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
                }
                else
                {
                    ret= npd_fdb_static_entry_del(macAddr,vlanId);

                    if (ret != FDB_RETURN_CODE_SUCCESS)
                    {
                        syslog_ax_fdb_err("npd_fdb_static_entry_del erro return value is:%d\n",ret);
                        ret = FDB_RETURN_CODE_GENERAL;
                    }
                }

                free(node);
            }
        }
        else if (1 == result)
        {
            ret = FDB_RETURN_CODE_SYSTEM_MAC;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32,  &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_show_fdb_one(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS,number = 0;;
    unsigned short	vlanId = 0;
    unsigned char   macAddr[6]={0};
    NPD_FDB dcli_show;
    memset(&dcli_show,0,sizeof(NPD_FDB));
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanId,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    if (CHASSIS_VLANID_ISLEGAL(vlanId))
    {
        ret = npd_check_vlan_exist(vlanId);

        if (NPD_TRUE == ret)
        {
            ret = nam_show_fdb_one(&dcli_show,macAddr, vlanId);

            if (ret != 0)
            {
                syslog_ax_fdb_err("NO FDB ITEM! \n");
                number = 0;
                ret = FDB_RETURN_CODE_GENERAL;
            }
            else
            {
                number = 1;
                ret = FDB_RETURN_CODE_SUCCESS;
            }
        }
        else
        {
            syslog_ax_fdb_err("The vlanId %d is not exists",vlanId);
            number  = 0;
            ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
        }
    }
    else
    {
        syslog_ax_fdb_err("Illegal vlanId %d",vlanId);
        number = 0;
        ret = FDB_RETURN_CODE_BADPARA;
        /*better to tell user to input vlanId again*/
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_UINT32,
     &ret);
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_UINT32,
     &number);
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_UINT16,
     &(dcli_show.vlanid));
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_UINT32,
     &dcli_show.value);
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_BYTE,
     &(dcli_show.ether_mac[0]));
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_BYTE,
     &(dcli_show.ether_mac[1]));
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_BYTE,
     &(dcli_show.ether_mac[2]));
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_BYTE,
     &(dcli_show.ether_mac[3]));
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_BYTE,
     &(dcli_show.ether_mac[4]));
    dbus_message_iter_append_basic
    (&iter,
     DBUS_TYPE_BYTE,
     &(dcli_show.ether_mac[5]));
    return reply;
}


DBusMessage * npd_dbus_fdb_show_fdb_mac(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    DBusError err;
    unsigned char   macAddr[6];
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int i=0;
    static NPD_FDB *dcli_show;
    dcli_show = malloc(sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);

    if (NULL == dcli_show)
        return NULL;

    memset(dcli_show, 0, sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_BYTE, &macAddr[0],
                                DBUS_TYPE_BYTE, &macAddr[1],
                                DBUS_TYPE_BYTE, &macAddr[2],
                                DBUS_TYPE_BYTE, &macAddr[3],
                                DBUS_TYPE_BYTE, &macAddr[4],
                                DBUS_TYPE_BYTE, &macAddr[5],
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        ret = nam_show_fdb_mac(dcli_show, macAddr, NPD_FDB_TABLE_SIZE);

        if (ret == 0)
        {
            syslog_ax_fdb_err("NO FDB ITEM! \n");
        }

        if (ret > NPD_FDB_TABLE_SIZE)
        {
            ret = 0;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT16_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);

    for (; i < ret; i++)
    {
        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT16,
         &(dcli_show[i].vlanid));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].value));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[0]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[1]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[2]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[3]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[4]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[5]));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);
    free(dcli_show);
    return reply;
}

DBusMessage * npd_dbus_fdb_show_fdb(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    unsigned int start_index;
    unsigned int get_num;
    static NPD_FDB *dcli_show;
    unsigned int	ret = 0, op_ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int i=0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&start_index,
                                DBUS_TYPE_UINT32,&get_num,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    dcli_show = malloc(sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);

    if (NULL == dcli_show)
        goto rtncode;

    memset(dcli_show, 0, sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);
    ret = nam_show_fdb(dcli_show, start_index, NPD_FDB_TABLE_SIZE);

    if (ret == 0)
    {
        syslog_ax_fdb_err("NO FDB ITEM! \n");
        goto rtncode;
    }

    if (ret > NPD_FDB_TABLE_SIZE)
    {
        ret = 0;
    }

rtncode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32, &op_ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT16_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);

    for (i = 0; i < ret; i++)
    {
        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        /* VLAN ID*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT16,
         &(dcli_show[i].vlanid));
        /*netif index*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].value));
        /*ITEM type*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &dcli_show[i].type_flag);
        /*MAC address*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[0]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[1]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[2]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[3]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[4]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[5]));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);
    free(dcli_show);
    return reply;
}

DBusMessage * npd_dbus_fdb_show_single_unit_fdb(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    unsigned int unit;
    unsigned int get_num;
    static NPD_FDB *dcli_show;
    unsigned int	ret = 0, op_ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int i=0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&unit,
                                DBUS_TYPE_UINT32,&get_num,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    dcli_show = malloc(sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);

    if (NULL == dcli_show)
        goto rtncode;

    memset(dcli_show, 0, sizeof(NPD_FDB) * NPD_FDB_BOARD_TABLE_SIZE);
    ret = nam_show_single_unit_fdb(dcli_show, unit, NPD_FDB_BOARD_TABLE_SIZE);

    if (ret == 0)
    {
        syslog_ax_fdb_err("NO FDB ITEM! \n");
        goto rtncode;
    }

    if (ret > NPD_FDB_TABLE_SIZE)
    {
        ret = 0;
    }

rtncode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32, &op_ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT16_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);

    for (i = 0; i < ret; i++)
    {
        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        /* VLAN ID*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT16,
         &(dcli_show[i].vlanid));
        /*netif index*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].value));
        /*ITEM type*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &dcli_show[i].type_flag);
        /*MAC address*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[0]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[1]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[2]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[3]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[4]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[5]));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);
    free(dcli_show);
    return reply;
}

DBusMessage * npd_dbus_fdb_show_dynamic_fdb(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusError err;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    unsigned int start_index;
    unsigned int get_num;
    static NPD_FDB *dcli_show;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int i = 0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&start_index,
                                DBUS_TYPE_UINT32,&get_num,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    dcli_show = malloc(sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);

    if (NULL == dcli_show)
        goto rtncode;

    memset(dcli_show, 0, sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);
    ret = nam_show_dynamic_fdb(dcli_show, start_index, NPD_FDB_TABLE_SIZE);

    if (ret == 0)
    {
        syslog_ax_fdb_err("NO FDB ITEM! \n");
        goto rtncode;
    }

    if (ret > NPD_FDB_TABLE_SIZE)
    {
        ret = 0;
    }

rtncode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT16_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);

    for (i = 0; i < ret; i++)
    {
        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        /* VLAN ID*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT16,
         &(dcli_show[i].vlanid));
        /* netif index*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].value));
        /*ITEM type*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &dcli_show[i].type_flag);
        /*MAC address*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[0]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[1]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[2]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[3]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[4]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[5]));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);
    free(dcli_show);
    return reply;
}

DBusMessage * npd_dbus_fdb_show_fdb_count(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    ret = nam_show_fdb_count();

    if (ret == 0)
    {
        syslog_ax_fdb_err("NO FDB ITEM! \n");
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_delete_fdb_with_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned short vlanid;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanid,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        if (NPD_TRUE != npd_check_vlan_exist(vlanid))
        {
            ret= FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
        }
        else
        {
            ret = npd_fdb_static_entry_del_by_vlan(vlanid);
            ret = npd_fdb_dynamic_entry_del_by_vlan(vlanid);

#ifndef HAVE_FDB_SW_SYNC
            ret = dbtable_array_get(npd_fdb_cfg_index, 0, &npd_fdb_cfg_set);

            if (ret != 0)
            {
                ret = FDB_RETURN_CODE_NODE_NOT_EXIST;
            }

            npd_fdb_cfg_set.del_all = 0;
            npd_fdb_cfg_set.del_netif_index = 0;
            npd_fdb_cfg_set.del_vlanid = vlanid;
            dbtable_array_update(npd_fdb_cfg_index, 0, NULL, &npd_fdb_cfg_set);

			npd_fdb_cfg_set.del_vlanid = 0;
            dbtable_array_update(npd_fdb_cfg_index, 0, NULL, &npd_fdb_cfg_set);			
#endif
		}
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_delete_fdb_with_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned int eth_g_index;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        ret = npd_fdb_static_entry_del_by_port(eth_g_index);
        ret = npd_fdb_dynamic_entry_del_by_port(eth_g_index);


#ifndef HAVE_FDB_SW_SYNC		
        ret = dbtable_array_get(npd_fdb_cfg_index, 0, &npd_fdb_cfg_set);

        if (ret != 0)
        {
            ret = FDB_RETURN_CODE_NODE_NOT_EXIST;
        }
        npd_fdb_cfg_set.del_all = 0;
        npd_fdb_cfg_set.del_netif_index = eth_g_index;
        npd_fdb_cfg_set.del_vlanid = 0;
        dbtable_array_update(npd_fdb_cfg_index, 0, NULL, &npd_fdb_cfg_set);

		npd_fdb_cfg_set.del_netif_index = 0;
        dbtable_array_update(npd_fdb_cfg_index, 0, NULL, &npd_fdb_cfg_set);			
#endif

	}

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;
}


DBusMessage * npd_dbus_fdb_delete_static_fdb_with_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned short vlanid;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanid,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        if (NPD_FALSE == npd_check_vlan_exist(vlanid))
        {
            ret= FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
        }
        else
        {
            ret = npd_fdb_static_entry_del_by_vlan(vlanid);

            if (FDB_RETURN_CODE_SUCCESS != ret)
            {
                syslog_ax_fdb_err("Delete fdb static by vlan in sw error: %d\n",ret);
                ret = FDB_RETURN_CODE_GENERAL;
            }
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_delete_static_fdb_with_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned int eth_g_index;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        ret = npd_fdb_static_entry_del_by_port(eth_g_index);

        if (FDB_RETURN_CODE_SUCCESS != ret)
        {
            syslog_ax_fdb_err("Delete fdb static by port error:%d\n",ret);
            ret = FDB_RETURN_CODE_GENERAL;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_delete_fdb_with_trunk(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned short trunk_no = 0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&trunk_no,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    }
    else
    {
        if (TRUNK_RETURN_CODE_TRUNK_EXISTS == npd_check_trunk_exist(trunk_no))
        {
            unsigned int netif_index = npd_netif_trunk_get_index(trunk_no);
			
            npd_fdb_static_entry_del_by_port(netif_index);
            ret = npd_fdb_entry_del_by_port(netif_index);

            if (ret!= FDB_RETURN_CODE_SUCCESS)
            {
                syslog_ax_fdb_err("nam_fdb_table_delete_entry_with_port:err! \n");
                ret = FDB_RETURN_CODE_OCCUR_HW;
            }
#ifndef HAVE_FDB_SW_SYNC
			ret = dbtable_array_get(npd_fdb_cfg_index, 0, &npd_fdb_cfg_set);

            if (ret != 0)
            {
                ret = FDB_RETURN_CODE_NODE_NOT_EXIST;
            }

            npd_fdb_cfg_set.del_all = 0;
            npd_fdb_cfg_set.del_netif_index = netif_index;
            npd_fdb_cfg_set.del_vlanid = 0;
            dbtable_array_update(npd_fdb_cfg_index, 0, NULL, &npd_fdb_cfg_set);

			npd_fdb_cfg_set.del_netif_index = 0;
	        dbtable_array_update(npd_fdb_cfg_index, 0, NULL, &npd_fdb_cfg_set);			
#endif
        }
        else
        {
            syslog_ax_fdb_err("The trunk does not exist\n");
            ret = FDB_RETURN_CODE_GENERAL;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;
}
DBusMessage * npd_dbus_fdb_static_delete_fdb_with_trunk(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    DBusError err;
    unsigned short trunk_no = 0;
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&trunk_no,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    }
    else
    {
        if (TRUNK_RETURN_CODE_TRUNK_EXISTS == npd_check_trunk_exist(trunk_no))
        {
            unsigned int netif_index = npd_netif_trunk_get_index(trunk_no);
            struct npd_fdb_cfg_s npd_fdb_cfg_set;
            npd_fdb_static_entry_del_by_port(netif_index);
            
            npd_fdb_cfg_set.del_all = 0;
            npd_fdb_cfg_set.del_netif_index = netif_index;
            npd_fdb_cfg_set.del_vlanid = 0;
            npd_fdb_cfg_set.del_netif_index = 0;
            ret=dbtable_array_update(npd_fdb_cfg_index, 0, NULL, &npd_fdb_cfg_set);
            if (ret!= FDB_RETURN_CODE_SUCCESS)
            {
                syslog_ax_fdb_err("nam_fdb_table_delete_entry_with_port:err! \n");
                ret = FDB_RETURN_CODE_OCCUR_HW;
            }
        }
        else
        {
            syslog_ax_fdb_err("The trunk does not exist\n");
            ret = FDB_RETURN_CODE_GENERAL;
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &ret);
    return reply;
}

DBusMessage * npd_dbus_fdb_show_static_fdb(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    struct fdb_entry_item_s *dcli_show = NULL;
    unsigned int	ret = FDB_RETURN_CODE_SUCCESS;
    unsigned int i=0;
    unsigned char flag;
    unsigned int number = AX_STATIC_FDB_ENTRY_SIZE + AX_STATIC_FDB_ENTRY_SIZE;

    if (number == 0)
    {
        syslog_ax_fdb_err("NO FDB ITEM! \n");
    }
    else
    {
        dcli_show = malloc(number * sizeof(struct fdb_entry_item_s));

        if (dcli_show == NULL)
        {
            syslog_ax_fdb_err("memory malloc fail when show static fdb\n");
            return NULL;
        }

        memset(dcli_show, 0, number * sizeof(struct fdb_entry_item_s));
        ret = npd_fdb_get_static_item(dcli_show, number);

        if (ret == 0)
        {
            syslog_ax_fdb_err("NO FDB ITEM! \n");
            free(dcli_show);
        }

        npd_syslog_dbg("Get totoal %d Static FDB\n", ret);
        number = ret ;
    }

    /*ret value respect the number not exist in software*/
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&number);

    if (number > 0)
    {
        dbus_message_iter_open_container(&iter,
                                         DBUS_TYPE_ARRAY,
                                         DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                         DBUS_TYPE_UINT16_AS_STRING
                                         DBUS_TYPE_UINT32_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_STRUCT_END_CHAR_AS_STRING,
                                         &iter_array);

        for (i=0; i < number; i++)
        {
            flag = STATIC_FLAG;

            if (dcli_show[i].isBlock)
                flag = BLACK_FLAG;
            else if (dcli_show[i].isMirror)
                flag = MIRROR_FLAG;
            else if (dcli_show[i].isAuthen)
                flag = AUTH_FLAG;

            DBusMessageIter iter_struct;
            dbus_message_iter_open_container(&iter_array,
                                             DBUS_TYPE_STRUCT,
                                             NULL,
                                             &iter_struct);
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_UINT16,
             &(dcli_show[i].vlanid));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_UINT32,
             &(dcli_show[i].ifIndex));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(dcli_show[i].mac[0]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(dcli_show[i].mac[1]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(dcli_show[i].mac[2]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(dcli_show[i].mac[3]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(dcli_show[i].mac[4]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(dcli_show[i].mac[5]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(flag));
            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }

        free(dcli_show);
        dbus_message_iter_close_container(&iter, &iter_array);
    }

    return reply;
}
DBusMessage * npd_dbus_fdb_show_blacklist_fdb(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    struct fdb_entry_item_s *blacklist_show = NULL;
    unsigned int number = 0;
    unsigned int i=0, ret = FDB_RETURN_CODE_SUCCESS;
    number = AX_STATIC_FDB_ENTRY_SIZE;

    if (number > 0)
    {
        blacklist_show = (struct fdb_entry_item_s *)malloc(number * sizeof(struct fdb_entry_item_s));
        memset(blacklist_show, 0, number * sizeof(struct fdb_entry_item_s));
        ret = npd_fdb_get_blacklist_item(blacklist_show,number);

        if (ret == 0)
        {
            syslog_ax_fdb_err("NO FDB BLACKLIST ITEM! \n");
            free(blacklist_show);
        }

        if (ret == FDB_RETURN_CODE_MAX)
        {
            syslog_ax_fdb_err("FDB BLACKLIST ITEM FULL! \n");
            free(blacklist_show);
        }

        number = ret;
    }

    npd_syslog_dbg("Get totoal %d blacklist FDB\n", number);
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&number);

    if (number > 0 && number != FDB_RETURN_CODE_MAX)
    {
        dbus_message_iter_open_container(&iter,
                                         DBUS_TYPE_ARRAY,
                                         DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING /*dmac*/
                                         DBUS_TYPE_BYTE_AS_STRING  /*smac*/
                                         DBUS_TYPE_UINT16_AS_STRING /*vlanid*/
                                         DBUS_TYPE_BYTE_AS_STRING   /*mac address[6]*/
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_TYPE_BYTE_AS_STRING
                                         DBUS_STRUCT_END_CHAR_AS_STRING,
                                         &iter_array);

        for (i = 0; i < number; i++)
        {
            char dmac, smac;
            dmac = blacklist_show[i].blockMode & NPD_FDB_BLACKLIST_DMAC_MODE;
            smac = blacklist_show[i].blockMode & NPD_FDB_BLACKLIST_SMAC_MODE;

            if (smac)
            {
                dmac = NPD_FDB_BLACKLIST_DMAC_MODE;
            }

            DBusMessageIter iter_struct;
            dbus_message_iter_open_container(&iter_array,
                                             DBUS_TYPE_STRUCT,
                                             NULL,
                                             &iter_struct);
            /*dmac*/
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(dmac));
            /*printf("The blacklist_show[%d].dmac is %d \n",i,blacklist_show[i].dmac);*/
            /*snac*/
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(smac));
            /*printf("The blacklist_show[%d].smac is %d \n",i,blacklist_show[i].smac);*/
            /*vlanid*/
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_UINT16,
             &(blacklist_show[i].vlanid));
            /*mac address*/
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(blacklist_show[i].mac[0]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(blacklist_show[i].mac[1]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(blacklist_show[i].mac[2]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(blacklist_show[i].mac[3]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(blacklist_show[i].mac[4]));
            dbus_message_iter_append_basic
            (&iter_struct,
             DBUS_TYPE_BYTE,
             &(blacklist_show[i].mac[5]));
            dbus_message_iter_close_container(&iter_array, &iter_struct);
        }

        free(blacklist_show);
        dbus_message_iter_close_container(&iter, &iter_array);
    }

    return reply;
}

DBusMessage * npd_dbus_fdb_show_fdb_port(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    DBusError err;
    unsigned int	ret=0, op_ret = FDB_RETURN_CODE_SUCCESS; ;
    unsigned int i=0;
    unsigned int start_index;
    unsigned int get_num;
    unsigned int eth_g_index = 0;
    NPD_FDB *dcli_show = NULL;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&start_index,
                                DBUS_TYPE_UINT32,&get_num,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    dcli_show = malloc(sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);

    if (NULL == dcli_show)
    {
        ret = 0;
        goto rtncode;
    }

    memset(dcli_show,0,sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);
    ret = nam_show_fdb_port(dcli_show, start_index, NPD_FDB_TABLE_SIZE, eth_g_index);

    if (ret == 0)
    {
        syslog_ax_fdb_err("NO FDB ITEM! \n");
        goto rtncode;
    }

    if (ret > NPD_FDB_TABLE_SIZE)
    {
        ret = 0;
        op_ret = FDB_RETURN_CODE_OCCUR_HW;
    }

rtncode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32,&op_ret);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT16_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);
	
	for (i = 0; i<ret; i++)
    {
        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT16,
         &(dcli_show[i].vlanid));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].value));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].type_flag));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[0]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[1]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[2]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[3]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[4]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[5]));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);

    if (dcli_show)
    {
        free(dcli_show);
    }

    return reply;
}

DBusMessage * npd_dbus_fdb_show_fdb_vlan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    DBusError err;
    NPD_FDB  *dcli_show = NULL;
    unsigned int	ret=0;
    unsigned int dnumber = 0;
    unsigned int i=0;
    unsigned int start_index = 0, get_num = 0;
    unsigned short vlan = 0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&start_index,
                                DBUS_TYPE_UINT32,&get_num,
                                DBUS_TYPE_UINT16,&vlan,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    dcli_show = malloc(sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);

    if (NULL == dcli_show)
    {
        goto rtncode;
    }

    memset(dcli_show,0,sizeof(NPD_FDB) * NPD_FDB_TABLE_SIZE);
    ret = nam_show_fdb_vlan(dcli_show, start_index, NPD_FDB_TABLE_SIZE, vlan);

    if (ret == 0)
    {
        syslog_ax_fdb_err("NO FDB ITEM! \n");
        goto rtncode;
    }

    if (ret > NPD_FDB_TABLE_SIZE)
    {
        ret = 0;
    }

    dnumber = ret;
rtncode:
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&dnumber);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT16_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);

	for (i = 0; dnumber > i; i++)
    {
        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT16,
         &(dcli_show[i].vlanid));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].value));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].type_flag));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[0]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[1]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[2]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[3]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[4]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[5]));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);

    if (dcli_show)
    {
        free(dcli_show);
    }

    return reply;
}

DBusMessage * npd_dbus_fdb_show_fdb_vlan_with_name(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
#if 0
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusMessageIter	 iter_array;
    DBusError err;
    unsigned int dcli_flag=0;
    unsigned int trans_value1=0;
    unsigned int trans_value2=0;
    static NPD_FDB  dcli_show [FDB_TABLE_LEN] ;
    unsigned int i=0;
    unsigned short vlan = 0;
    struct vlan_s* node = NULL;
    unsigned int eth_g_index = 0,slot_index = 0,slot_no = 0,local_port_index = 0,local_portno = 0;
    char* vlanname=NULL;
    int	ret=FDB_RETURN_CODE_SUCCESS;
    unsigned int dnumber=0;
    memset(dcli_show,0,sizeof(NPD_FDB)*FDB_TABLE_LEN);
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_STRING,&vlanname,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
    else
    {
        node= npd_find_vlan_by_name(vlanname);

        if (node == NULL)
        {
            ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
        }
        else
        {
            vlan = node->vid;

            if (!(CHASSIS_VLANID_ISLEGAL(vlan)))
            {
                syslog_ax_fdb_err("illegal vlanId %d",vlan);
                ret = FDB_RETURN_CODE_NODE_VLAN_NONEXIST;
            }
            else
            {
                dnumber = nam_show_fdb_vlan(dcli_show,FDB_TABLE_LEN,vlan);

                if (dnumber == 0)
                {
                    syslog_ax_fdb_err("NO FDB ITEM! \n");
                    ret = FDB_RETURN_CODE_SUCCESS;
                }
            }
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&dnumber);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);

    for (; dnumber> i; i++)
    {
        if (dcli_show[i].inter_type == CPSS_INTERFACE_PORT_E)
        {
            dcli_flag = CPSS_INTERFACE_PORT_E;

            if (g_cpu_port_index == dcli_show[i].value)
            {
                trans_value1 = CPU_PORT_VIRTUAL_SLOT;
                trans_value2 = CPU_PORT_VIRTUAL_PORT;
            }
            else if (g_spi_port_index == dcli_show[i].value)
            {
                trans_value1 = SPI_PORT_VIRTUAL_SLOT;
                trans_value2 = SPI_PORT_VIRTUAL_PORT;
            }
            else
            {
                eth_g_index = dcli_show[i].value;
                slot_index=SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
                slot_no = CHASSIS_SLOT_INDEX2NO(slot_index);
                local_port_index = ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
                local_portno = ETH_LOCAL_INDEX2NO(slot_index,local_port_index);
                trans_value1 = slot_no;
                trans_value2 = local_portno;
            }
        }
        else if ((dcli_show[i] .inter_type > CPSS_INTERFACE_PORT_E)&&(dcli_show[i] .inter_type <CPSS_INTERFACE_MAX))
        {
            if (dcli_show[i] .inter_type == CPSS_INTERFACE_TRUNK_E)
            {
                dcli_flag = CPSS_INTERFACE_TRUNK_E;
            }
            else if (dcli_show[i] .inter_type == CPSS_INTERFACE_VIDX_E)
            {
                dcli_flag = CPSS_INTERFACE_VIDX_E;
            }
            else if (dcli_show[i] .inter_type == CPSS_INTERFACE_VID_E)
            {
                dcli_flag = CPSS_INTERFACE_VID_E;
            }

            trans_value1 = dcli_show[i].value;
            trans_value2 = 0;
        }
        else
        {
            syslog_ax_fdb_err("sorry interface type wrong !\n");
            syslog_ax_fdb_err("interface type is: %d\n",dcli_show->inter_type);
        }

        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &dcli_flag);
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(dcli_show[i].vlanid));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &trans_value1);
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &trans_value2);
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[0]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[1]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[2]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[3]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[4]));
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(dcli_show[i].ether_mac[5]));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);
    return reply;
#endif
    return NULL;
}


DBusMessage *npd_dbus_fdb_config_port_number(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    unsigned int eth_g_index = 0;
    unsigned int fdblimit = 0;
    unsigned int ret = FDB_RETURN_CODE_SUCCESS;
    nam_learn_limit_t limit;
    memset(&limit,0,sizeof(nam_learn_limit_t));
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_UINT32,&fdblimit,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    }

    ret = npd_fdb_number_port_set(eth_g_index, fdblimit);
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &fdblimit);
    return reply;
}

DBusMessage *npd_dbus_fdb_config_vlan_number(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    unsigned short vlanid = 0;
    unsigned int fdblimit = 0;
    unsigned int ret= FDB_RETURN_CODE_SUCCESS;
    nam_learn_limit_t limit;
    memset(&limit,0,sizeof(nam_learn_limit_t));
    
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanid,
                                DBUS_TYPE_INT32,&fdblimit,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    }

    ret = npd_fdb_number_vlan_set(vlanid, fdblimit);
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &fdblimit);
    return reply;
}

DBusMessage *npd_dbus_fdb_config_vlan_port_number(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusError err;
    unsigned short vlanid = 0;
    unsigned int eth_g_index;
    unsigned int fdblimit = 0;
    unsigned int ret = FDB_RETURN_CODE_SUCCESS;
    nam_learn_limit_t limit;
    memset(&limit,0,sizeof(nam_learn_limit_t));
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT16,&vlanid,
                                DBUS_TYPE_UINT32,&eth_g_index,
                                DBUS_TYPE_UINT32,&fdblimit,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_fdb_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_fdb_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &fdblimit);
    return reply;
}

DBusMessage *npd_dbus_fdb_show_limit_item(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    DBusMessageIter	 iter_array;
    unsigned char slot_no =0;
    unsigned int slot_index = 0;
    unsigned char local_port_no =0;
    unsigned int fdblimit=0;
    unsigned int ret =FDB_RETURN_CODE_SUCCESS;
    unsigned int number = 0;
    unsigned int count = 0;
    int i = 0;
    /*TODO::display 50 items now,then modified it.*/
    struct fdb_number_limit_s node[50];
    memset(node,0,sizeof(struct fdb_number_limit_s)*50);

    /*for the vlan-based*/
    for (i = 1; i<4095; i++)
    {
        ret = npd_check_vlan_exist(i);

        if (NPD_TRUE == ret)
        {
            ret = npd_fdb_number_vlan_set_check(i,&fdblimit);

            if (1 == ret)
            {
                number += fdblimit;
                node[count].vlanId = i;
                node[count].number = fdblimit;

                if (count < 50)
                    count++;
            }
        }
    }

    /*for the port-base*/
    for (slot_no = CHASSIS_SLOT_START_NO; slot_no < (CHASSIS_SLOT_COUNT+CHASSIS_SLOT_START_NO); slot_no++)
    {
        if (CHASSIS_SLOTNO_ISLEGAL(slot_no))
        {
            slot_index = CHASSIS_SLOT_NO2INDEX(slot_no);

            for (local_port_no = ETH_LOCAL_PORT_START_NO(slot_index); local_port_no <(ETH_LOCAL_PORT_COUNT(slot_index)+ETH_LOCAL_PORT_START_NO(slot_index)); local_port_no++)
            {
                if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no,local_port_no))
                {
                    unsigned int eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_INDEX(slot_index, ETH_LOCAL_NO2INDEX(slot_index, local_port_no));
                    ret = npd_fdb_number_port_set_check(eth_g_index,&fdblimit);

                    if (1 == ret)
                    {
                        number += fdblimit;
                        node[count].slot_no = slot_no;
                        node[count].local_port_no = local_port_no;
                        node[count].number = fdblimit;

                        if (count<50)
                            count++;
                    }
                }
            }
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,&count);
    dbus_message_iter_open_container(&iter,
                                     DBUS_TYPE_ARRAY,
                                     DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                     DBUS_TYPE_UINT16_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_BYTE_AS_STRING
                                     DBUS_TYPE_UINT32_AS_STRING
                                     DBUS_STRUCT_END_CHAR_AS_STRING,
                                     &iter_array);

    for (i = 0; i < count; i++)
    {
        DBusMessageIter iter_struct;
        dbus_message_iter_open_container(&iter_array,
                                         DBUS_TYPE_STRUCT,
                                         NULL,
                                         &iter_struct);
        /* VLAN ID*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT16,
         &(node[i].vlanId));
        /* SLOT number or VLAN ID or VIDX or Trunk ID*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(node[i].slot_no));
        /* PORT number*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_BYTE,
         &(node[i].local_port_no));
        /*FDB limit number*/
        dbus_message_iter_append_basic
        (&iter_struct,
         DBUS_TYPE_UINT32,
         &(node[i].number));
        dbus_message_iter_close_container(&iter_array, &iter_struct);
    }

    dbus_message_iter_close_container(&iter, &iter_array);
    return reply;
}
#ifdef HAVE_VRRP
DBusMessage * npd_dbus_create_vrrp_by_ifname(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter	 iter;
    DBusError err;
    unsigned int ifindex1 = ~0UI,ifindex2 = ~0UI,ret = FDB_RETURN_CODE_SUCCESS;
    unsigned char* pname1 = NULL;
    unsigned char* pname2 = NULL;
    int add = 0;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_STRING,&pname1,
                                DBUS_TYPE_STRING,&pname2,
                                DBUS_TYPE_UINT32,&add,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_intf_err("Unable to get input args ");

        if (dbus_error_is_set(&err))
        {
            syslog_ax_intf_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

    syslog_ax_intf_dbg("call %s line %d pname1 %s, ifindex1 %d,pname2 %s,ifindex2 %d\r\n", __func__, __LINE__,pname1,ifindex1,pname2,ifindex2);

    if (add)
    {
        if (0 == strcmp((char *)pname1,(char *)pname2))
        {
            ret = npd_fdb_create_for_vrrp(pname1);
        }
        else if (FDB_RETURN_CODE_SUCCESS != (ret = npd_fdb_create_for_vrrp(pname1)))
        {
            syslog_ax_intf_err("call npd_create_vlan_intf_by_vid ret %d,ifindex %d\n",ret ,ifindex1);
        }
        else if (FDB_RETURN_CODE_SUCCESS != (ret = npd_fdb_create_for_vrrp(pname2)))
        {
            syslog_ax_intf_err("call npd_create_vlan_intf_by_vid ret %d,ifindex %d\n",ret ,ifindex2);
        }
    }
    else
    {
        if (0 == strcmp((char *)pname1,(char *)pname2))
        {
            ret = npd_fdb_del_for_vrrp(pname1);
        }
        else if (FDB_RETURN_CODE_SUCCESS != (ret = npd_fdb_del_for_vrrp(pname1)))
        {
            syslog_ax_intf_err("call npd_create_vlan_intf_by_vid ret %d,ifindex %d\n",ret ,ifindex1);
        }
        else if (FDB_RETURN_CODE_SUCCESS != (ret = npd_fdb_del_for_vrrp(pname2)))
        {
            syslog_ax_intf_err("call npd_create_vlan_intf_by_vid ret %d,ifindex %d\n",ret ,ifindex2);
        }
    }

    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,  &ret);
    return reply;
}
#endif
int npd_fdb_show_running_entry(
    hash_table_index_t *hash,
    void *in,
    unsigned int flag)
{
    struct fdb_entry_item_s *dbItem = (struct fdb_entry_item_s *)in;
    char tmpBuf[256] = {0};
    char *cursor = NULL;
    unsigned int lenth = 0;

    if (dbItem == NULL)
        return 0;

    if ((NULL == npd_fdb_showStr) ||(npd_fdb_showStr_len >= NPD_FDB_SHOWRUN_CFG_SIZE - 2))
    {
        syslog_ax_fdb_err("not enough memory\n");
        return 0;
    }

    cursor = npd_fdb_showStr + npd_fdb_showStr_len;

    if (dbItem->isMirror != 1)
    {
        if (dbItem->isBlock != 1)
        {
            {
                char name[50] = {0};
                npd_netif_index_to_user_fullname(dbItem->ifIndex, name);
                lenth = sprintf(tmpBuf,"mac-address-table static %02x:%02x:%02x:%02x:%02x:%02x vlan %d %s\n",	\
                                dbItem->mac[0], dbItem->mac[1], dbItem->mac[2], dbItem->mac[3],	\
                                dbItem->mac[4], dbItem->mac[5], dbItem->vlanid, name);
            }

            if ((cursor+lenth-npd_fdb_showStr) >= NPD_FDB_SHOWRUN_CFG_SIZE - 2) /* 2 - two '\n' in the head and bottom*/
                return 0;

            sprintf(cursor,"%s",tmpBuf);
            cursor +=lenth;
            npd_fdb_showStr_len += lenth;
        }
        else
        {
            if (dbItem->blockMode & NPD_FDB_BLACKLIST_DMAC_MODE)
            {
                lenth = sprintf(tmpBuf,"mac-address-table blacklist %s %02x:%02x:%02x:%02x:%02x:%02x vlan %d \n", "destination",
                                dbItem->mac[0], dbItem->mac[1], dbItem->mac[2], dbItem->mac[3],\
                                dbItem->mac[4], dbItem->mac[5], dbItem->vlanid);

                if ((cursor+lenth-npd_fdb_showStr) >= (NPD_FDB_SHOWRUN_CFG_SIZE - 2)) /* 2 - two '\n' in the head and bottom*/
                    return 0;

                sprintf(cursor,"%s",tmpBuf);
                cursor +=lenth;
                npd_fdb_showStr_len += lenth;
            }

            if (dbItem->blockMode & NPD_FDB_BLACKLIST_SMAC_MODE)
            {
                lenth = sprintf(tmpBuf,"mac-address-table blacklist %02x:%02x:%02x:%02x:%02x:%02x vlan %d \n",	\
                                dbItem->mac[0], dbItem->mac[1], dbItem->mac[2], dbItem->mac[3],\
                                dbItem->mac[4], dbItem->mac[5], dbItem->vlanid);

                if ((cursor+lenth-npd_fdb_showStr) >= (NPD_FDB_SHOWRUN_CFG_SIZE - 2)) /* 2 - two '\n' in the head and bottom*/
                    return 0;

                sprintf(cursor,"%s",tmpBuf);
                cursor +=lenth;
                npd_fdb_showStr_len += lenth;
            }
        }
    }

    return 0;
}


DBusMessage *npd_dbus_fdb_show_running_config(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    char *cursor = NULL;
    struct fdb_entry_item_s dbItem;
    unsigned int lenth = 0;
    npd_fdb_showStr = (char*)malloc(NPD_FDB_SHOWRUN_CFG_SIZE);

    if (NULL == npd_fdb_showStr)
    {
        syslog_ax_fdb_err("memory malloc error\n");
        return NULL;
    }

    memset(npd_fdb_showStr,0,NPD_FDB_SHOWRUN_CFG_SIZE);
    cursor = npd_fdb_showStr;
    cursor += sprintf(cursor,"\n");
    memset(&dbItem, 0, sizeof(struct fdb_entry_item_s));
    dbItem.isStatic = 1;
    dbtable_hash_traversal(
        npd_fdb_hashmac_index,
        TRUE,
        &dbItem,
        npd_fdb_static_filter,
        npd_fdb_show_running_entry
    );
    cursor = npd_fdb_showStr + npd_fdb_showStr_len;

    if (!((cursor+25-npd_fdb_showStr) >= NPD_FDB_SHOWRUN_CFG_SIZE - 2)&&(g_agingtime != 300))
    {
        char tmpBuf[128] = {0};
        lenth =  sprintf(tmpBuf,"bridge aging-time %d\n",g_agingtime);
        npd_syslog_dbg("agingtim changed\n");
        sprintf(cursor,"%s",tmpBuf);
        cursor +=lenth;
    }

    sprintf(cursor,"\n");
    /* npd_syslog_dbg("%s",npd_fdb_showStr);*/
    reply = dbus_message_new_method_return(msg);
    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_STRING,
                                   &npd_fdb_showStr);
    free(npd_fdb_showStr);
    npd_fdb_showStr = NULL;
    npd_fdb_showStr_len = 0;
    return reply;
}

#ifdef __cplusplus
}
#endif
