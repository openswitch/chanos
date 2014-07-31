/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
#ifdef HAVE_VRRP
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_vrrp.h"

unsigned int npd_vrrp_hash_comp_by_ifindex(void *data1, void *data2);
int npd_vrrp_del_ip_arp(unsigned int ipaddr[], int count);
int npd_vrrp_fdb_add_virtual_mac(char* ifname, char* hwaddr);
int npd_vrrp_fdb_del_virtual_mac(char* ifname, char* hwaddr);

extern unsigned int npd_fdb_static_vrrp_entry_add
(
    unsigned char *mac,
    unsigned short vid,
    unsigned int eth_g_index
);
extern unsigned int npd_fdb_static_vrrp_entry_del
(
    unsigned char* mac,
    unsigned short vid
);


extern unsigned int npd_arp_snooping_del_by_network
(
	unsigned int ip,
	unsigned int mask
);
extern int nam_static_fdb_entry_mac_set_for_bind_slot
(	unsigned char macAddr [ ], 
	unsigned short vlanId, 
	unsigned char enable
);
extern unsigned int npd_fdb_dynamic_entry_del
(
    unsigned char* mac,
    unsigned short vid
);
extern int nam_intf_vrrp_set(unsigned int netif_index, unsigned int vrid, unsigned char enable);


int		npd_vrrp_fd = 0;
struct	sockaddr_un 	npd_vrrp_addr;

db_table_t    *npd_vrrp_dbtbl = NULL;
hash_table_index_t *npd_vrrp_hash_index = NULL; 

int npd_parse_clv_attr(struct clv_attr **tb, int max, struct clv_attr* clv, int len)
{
	while (CLV_OK (clv, len))
	{
		if (clv->clv_type <= max)
		{
			tb[clv->clv_type] = clv;
		}

		clv = CLV_NEXT (clv, len);
	}

    return 0;
}

int npd_vrrp_vlan_vmac_check(unsigned short vid, unsigned char *vmac_addr)
{
	npd_vrrp_db_t vrrpEntry;
	unsigned int netif_index = npd_netif_vlan_index(vid);
	unsigned int ifindex = 0;
	
	memset(&vrrpEntry,0,sizeof(vrrpEntry));

	if (NPD_FALSE ==  npd_intf_gindex_exist_check(netif_index, &ifindex))
	{
		return NPD_FALSE;
	}
	vrrpEntry.l3index = ifindex;
	
	if( 0 == dbtable_hash_search(npd_vrrp_hash_index, &vrrpEntry, npd_vrrp_hash_comp_by_ifindex, &vrrpEntry) )
	{
		if(vrrpEntry.state == NPD_VRRP_STATE_MASTER)
		{
			memcpy(vmac_addr, &(vrrpEntry.hwaddr), MAC_ADDR_LEN);
			return NPD_TRUE;
		}	
	}
		
	return NPD_FALSE;
}

int npd_vrrp_intf_vmac_check(unsigned int ifindex, unsigned char *vmac_addr)
{
	npd_vrrp_db_t vrrpEntry = {0};

	vrrpEntry.l3index = ifindex;
	
	if( 0 == dbtable_hash_search(npd_vrrp_hash_index, &vrrpEntry, npd_vrrp_hash_comp_by_ifindex, &vrrpEntry) )
	{
		if(vrrpEntry.state == NPD_VRRP_STATE_MASTER)
		{
			memcpy(vmac_addr, &(vrrpEntry.hwaddr), MAC_ADDR_LEN);
			return NPD_TRUE;
		}	
	}
		
	return NPD_FALSE;
}


int	npd_vrrp_sock_init(int* ser_sock)
{
	memset(&npd_vrrp_addr, 0, sizeof(npd_vrrp_addr));

	if ((*ser_sock = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
	{
		npd_syslog_err("create npd to vrrp socket fail\n");
		return VRRP_RETURN_CODE_ERR;
	}
	
	npd_vrrp_addr.sun_family = AF_LOCAL;
	strcpy(npd_vrrp_addr.sun_path, NPD_VRRP_COMMON_PATH);

    unlink(npd_vrrp_addr.sun_path);

	if (bind(*ser_sock, (struct sockaddr *)&npd_vrrp_addr, sizeof(npd_vrrp_addr)) == -1) 
	{
		npd_syslog_err("npd to vrrp socket created but failed when bind\n");
        close(*ser_sock);
        *ser_sock = -1;
		return VRRP_RETURN_CODE_ERR;
	}

	(void)chmod(npd_vrrp_addr.sun_path, 0777);
    
	return VRRP_RETURN_CODE_OK;	
}

int npd_vrrp_recv_info
(
	struct npd_vrrp_msg_buf* buf,
	unsigned int  buffer_len,
	int *read_len
)
{
	do
	{
		*read_len = recvfrom(npd_vrrp_fd, (char*)buf, buffer_len, 0, NULL, NULL);
		if (*read_len < 0 && errno == EINTR) 
		{
			continue;
		}
		break;
	} while (1);
    
	return VRRP_RETURN_CODE_OK;
}

int npd_vrrp_recvmsg_backup_proc(struct npd_vrrp_msg_buf* vrrp_notify)
{
    int len = 0;
    unsigned int vrrp_mac_mode = 0;
    struct npd_vrrp_common* nvhd = NULL;
    struct clv_attr* tb[NPD_VRRP_MAX];

    nvhd = (struct npd_vrrp_common* )vrrp_notify;
	if (NPD_VRRP_STATE_BACKUP != nvhd->type)
	{
		return -1;
	}

    len = nvhd->length - NVMSG_LENGTH(0);
	if (len < 0)
	{
		return -1;
	}

    memset(tb, 0, sizeof tb);
	npd_parse_clv_attr(tb, NPD_VRRP_MAX - 1, NVMSG_DATA(vrrp_notify), len);

    if ((tb[NPD_VRRP_IFNAME] == NULL)
       || (tb[NPD_VRRP_MAC_MODE] == NULL)
       || (tb[NPD_VRRP_HWADDR] == NULL)
       || (tb[NPD_VRRP_VIRTUAL_IPADDR] == NULL))
	{
		return -1;
	}

    memcpy(&vrrp_mac_mode, CLV_DATA(tb[NPD_VRRP_MAC_MODE]), sizeof(unsigned int));

    if (NPD_VRRP_VIRTUAL_MAC == vrrp_mac_mode)
    {
        char ifname[32];
        memset(ifname, 0, sizeof(ifname));

        strncpy(ifname, (char *) CLV_DATA(tb[NPD_VRRP_IFNAME]), tb[NPD_VRRP_IFNAME]->clv_length - CLV_LENGTH(0));

        npd_vrrp_fdb_del_virtual_mac(ifname, (char *) CLV_DATA(tb[NPD_VRRP_HWADDR]));
    }
    
    return 0;
}

int npd_vrrp_recvmsg_master_proc(struct npd_vrrp_msg_buf* vrrp_notify)
{
    int len = 0;
    unsigned int vrrp_mac_mode = 0;
    struct npd_vrrp_common* nvhd = NULL;
    struct clv_attr* tb[NPD_VRRP_MAX];

    nvhd = (struct npd_vrrp_common* )vrrp_notify;
	if (NPD_VRRP_STATE_MASTER != nvhd->type)
	{
		return -1;
	}

    len = nvhd->length - NVMSG_LENGTH(0);
	if (len < 0)
	{
		return -1;
	}

    memset(tb, 0, sizeof tb);
	npd_parse_clv_attr(tb, NPD_VRRP_MAX - 1, NVMSG_DATA(vrrp_notify), len);

    if ((tb[NPD_VRRP_IFNAME] == NULL)
       || (tb[NPD_VRRP_MAC_MODE] == NULL)
       || (tb[NPD_VRRP_HWADDR] == NULL)
       || (tb[NPD_VRRP_VIRTUAL_IPADDR] == NULL))
	{
		return -1;
	}

    memcpy(&vrrp_mac_mode, CLV_DATA(tb[NPD_VRRP_MAC_MODE]), sizeof(unsigned int));

    if (NPD_VRRP_VIRTUAL_MAC == vrrp_mac_mode)
    {
        char ifname[32];
        memset(ifname, 0, sizeof(ifname));

        strncpy(ifname, (char *) CLV_DATA(tb[NPD_VRRP_IFNAME]), tb[NPD_VRRP_IFNAME]->clv_length - CLV_LENGTH(0));
        npd_vrrp_fdb_add_virtual_mac(ifname, (char *) CLV_DATA(tb[NPD_VRRP_HWADDR]));
    }

    npd_vrrp_del_ip_arp((unsigned int*) CLV_DATA(tb[NPD_VRRP_VIRTUAL_IPADDR]), (tb[NPD_VRRP_VIRTUAL_IPADDR]->clv_length - CLV_LENGTH(0) / sizeof(int)));
    
    return 0;
}

int npd_vrrp_fdb_add_virtual_mac(char* ifname, char* hwaddr)
{
    unsigned int local_index = 0;
    unsigned int global_index = 0;
    unsigned int netif_index = 0;
    unsigned short vlan_id = 0;
	npd_vrrp_db_t vrrpEntry;
	npd_vrrp_db_t prioVrrpEntry;

    npd_intf_ifindex_get_by_ifname((unsigned char *)ifname, &local_index);
    npd_intf_get_global_l3index(local_index, &global_index);

	memset(&vrrpEntry,0,sizeof(vrrpEntry));
	vrrpEntry.l3index = global_index;
	memcpy(vrrpEntry.hwaddr, hwaddr, 6);

   	npd_intf_netif_get_by_ifindex(vrrpEntry.l3index, &netif_index);
	vlan_id = (unsigned short)npd_netif_vlan_get_vid(netif_index);

	/* set the l3intf for vmac */
	npd_intf_set_netif_mac(netif_index, (unsigned char *)hwaddr);

    
	if( 0 != dbtable_hash_search(npd_vrrp_hash_index, &vrrpEntry, NULL, &vrrpEntry) )
	{
		vrrpEntry.state = NPD_VRRP_STATE_MASTER;
		dbtable_hash_insert(npd_vrrp_hash_index, &vrrpEntry);
	}
	else
	{
        prioVrrpEntry = vrrpEntry;
        vrrpEntry.state = NPD_VRRP_STATE_MASTER;
		dbtable_hash_update(npd_vrrp_hash_index, &prioVrrpEntry, &vrrpEntry);
	}
    npd_fdb_dynamic_entry_del((unsigned char *)hwaddr,vlan_id);
	npd_fdb_static_vrrp_entry_add((unsigned char *)hwaddr,vlan_id, 0x0);

    return 0;
}

int npd_vrrp_fdb_del_virtual_mac(char* ifname, char* hwaddr)
{
    unsigned int local_index = 0;
    unsigned int global_index = 0;
	npd_vrrp_db_t vrrpEntry;
	unsigned int netif_index = 0;
	unsigned int vlan_id = 0;

    npd_intf_ifindex_get_by_ifname((unsigned char *)ifname, &local_index);
    npd_intf_get_global_l3index(local_index, &global_index);

	memset(&vrrpEntry, 0, sizeof(vrrpEntry));
	vrrpEntry.l3index = global_index;
	memcpy(vrrpEntry.hwaddr, hwaddr, 6);

	/* set the l3intf for product mac */
	npd_intf_netif_get_by_ifindex(vrrpEntry.l3index, &netif_index);	
	npd_intf_set_netif_mac(netif_index, SYS_PRODUCT_BASEMAC);
	
	if (0 == dbtable_hash_search(npd_vrrp_hash_index, &vrrpEntry, NULL, &vrrpEntry))
	{		
		dbtable_hash_delete(npd_vrrp_hash_index, &vrrpEntry, &vrrpEntry);
	}
	
   	npd_intf_netif_get_by_ifindex(vrrpEntry.l3index, &netif_index);
	vlan_id = (unsigned short)npd_netif_vlan_get_vid(netif_index);

	npd_fdb_static_vrrp_entry_del((unsigned char *)hwaddr, vlan_id);	
        
    return 0;
}

int npd_vrrp_del_ip_arp(unsigned int ipaddr[], int count)
{
    int ni = 0;
    
    for (ni = 0; ni < count; ni++)
    {
        npd_arp_snooping_del_by_network(ipaddr[ni], (unsigned int )(-1));
    }

    return 0;
}

long npd_vrrp_dbtable_handle_update(void *new, void *old)
{    
	return 0;
}

long npd_vrrp_dbtable_handle_insert(void *data)
{	
	int ret = -1;
	unsigned int vrid = 0;
	unsigned short vid = 0;
	unsigned int netif_index = 0;
	npd_vrrp_db_t *vrrpEntry = (npd_vrrp_db_t *)data;

	if (vrrpEntry == NULL)
	{
		return ret;
	}
		
	npd_intf_netif_get_by_ifindex(vrrpEntry->l3index, &netif_index);
	vid = (unsigned short)npd_netif_vlan_get_vid(netif_index);
	vrid = (vrrpEntry->hwaddr[5]&0xFF);
	
	ret = nam_intf_vrrp_set(netif_index, vrid, 1);

	return ret;
}

long npd_vrrp_dbtable_handle_delete(void *data)
{
	int ret = -1;
	unsigned short vid = 0;
	unsigned int vrid = 0;
	unsigned int netif_index = 0;
    
	npd_vrrp_db_t *vrrpEntry = (npd_vrrp_db_t *)data;

    if (vrrpEntry == NULL)
    {
		return ret;
    }
		
	npd_intf_netif_get_by_ifindex(vrrpEntry->l3index, &netif_index);
	vid = (unsigned short)npd_netif_vlan_get_vid(netif_index);
	vrid = (vrrpEntry->hwaddr[5]&0xFF);

	ret = nam_intf_vrrp_set(netif_index, vrid, 0);
	
    //ret = nam_no_static_fdb_entry_mac_vlan_set(vrrpEntry->hwaddr, vid, CPU_PORT_VINDEX);

	return ret;    
}

int npd_vrrp_dbtable_handle_ntoh(void *data)
{
	int ret = -1;
    
	npd_vrrp_db_t *vrrpEntry = (npd_vrrp_db_t *)data;

    if (vrrpEntry == NULL)
	{
		return ret;
	}

	vrrpEntry->l3index = ntohl(vrrpEntry->l3index);
	return 0;
}

int npd_vrrp_dbtable_handle_hton(void *data)
{
	int ret = -1;
	npd_vrrp_db_t *vrrpEntry = (npd_vrrp_db_t *)data;
	if(vrrpEntry == NULL)
		return ret;

	vrrpEntry->l3index = htonl(vrrpEntry->l3index);
	return 0;
}

unsigned int npd_vrrp_hash_genkey(void *data)
{
	unsigned int key = 0;	
	npd_vrrp_db_t *vrrpEntry = (npd_vrrp_db_t *)data;
	
	key = vrrpEntry->l3index;
	key = key % NPD_VRRP_MAX_SIZE;
    
	return key;
}

unsigned int npd_vrrp_hash_comp(void *data1, void *data2)
{
	unsigned int equal = TRUE;

	npd_vrrp_db_t *itemA, *itemB;
	
	if (NULL == data1 || NULL == data2 )
	{
		return FALSE;
	}

	itemA = (npd_vrrp_db_t *)data1;
	itemB = (npd_vrrp_db_t *)data2;

	if (itemA->l3index != itemB->l3index) 
    {/* L3 intf index*/
		equal = FALSE;
	}

	if (0 != memcmp((char*)itemA->hwaddr, (char*)itemB->hwaddr, MAC_ADDRESS_LEN))
    { /* MAC*/
		equal = FALSE;
	}

	return equal;
}

unsigned int npd_vrrp_hash_comp_by_ifindex(void *data1, void *data2)
{
	unsigned int equal = TRUE;

	npd_vrrp_db_t *itemA, *itemB;
	
	if (NULL == data1 || NULL == data2 )
	{
		return FALSE;
	}

	itemA = (npd_vrrp_db_t *)data1;
	itemB = (npd_vrrp_db_t *)data2;

	if (itemA->l3index != itemB->l3index) 
    {/* L3 intf index*/
		equal = FALSE;
	}

	return equal;
}


int npd_vrrp_dbtable_init()
{
	int ret = 0;

	ret = create_dbtable("npdvrrp", NPD_VRRP_MAX_SIZE, sizeof(npd_vrrp_db_t), 
		npd_vrrp_dbtable_handle_update,
		NULL,
		npd_vrrp_dbtable_handle_insert,
		npd_vrrp_dbtable_handle_delete,
		NULL, NULL, NULL,
		npd_vrrp_dbtable_handle_ntoh,
		npd_vrrp_dbtable_handle_hton,
		DB_SYNC_ALL,
		&(npd_vrrp_dbtbl));
	if( 0 != ret )
	{
		npd_syslog_err("create npd vrrp database fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_hash_index("npdvrrphash", npd_vrrp_dbtbl, NPD_VRRP_MAX_SIZE,
												npd_vrrp_hash_genkey,
												npd_vrrp_hash_comp,
												&npd_vrrp_hash_index);
	if(0 != ret)
	{
		npd_syslog_err("create npd vrrp hash index fail\n");
		return NPD_FAIL;
	}

	return NPD_SUCCESS;
}

int npd_vrrp_msg_handler(char *msg, int len)
{
    struct npd_vrrp_msg_buf *msg_buf = (struct npd_vrrp_msg_buf *)msg;
	
    switch (msg_buf->nvhdr.type)
    {
        case NPD_VRRP_STATE_BACKUP:
        {
            npd_vrrp_recvmsg_backup_proc(msg_buf);
            break;
        }
        case NPD_VRRP_STATE_MASTER:
        {
            npd_vrrp_recvmsg_master_proc(msg_buf);
            break;
        }
        default :
        {
            break;
        }
    }
    return 0;
}

int npd_vrrp_msg_init(void)
{
	/* create socket communication */
	if (VRRP_RETURN_CODE_OK != npd_vrrp_sock_init(&npd_vrrp_fd))
	{
		npd_syslog_err("Faile to create vrrp msg socket.\n");
		return -1;
	}

	npd_vrrp_dbtable_init();
	
	npd_syslog_dbg("Create vrrp msg socket %d ok\n", npd_vrrp_fd);

	npd_app_msg_socket_register(npd_vrrp_fd, "vrrpMsg", npd_vrrp_msg_handler, sizeof(struct npd_vrrp_msg_buf));

    return 0;
}

#ifdef __cplusplus
}
#endif
#endif

