
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* netif_index.c
*
* CREATOR:
*       chengjun@autelan.com
*
* DESCRIPTION:
*       API for al interface index operation. 
*
* DATE:
*       08/08/2010
*
*  FILE REVISION NUMBER:
*       $Revision: 1.1.1.1 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>

#include "lib/npd_bitarray.h"
#include "lib/netif_index.h"
#include "chassis_man_app.h"

int netif_is_system_box = -1;

int netif_system_is_box()
{
    netif_is_system_box = app_box_state_get();
    return netif_is_system_box;
}

int eth_port_get_chassis_by_ifindex(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = eth_g_index;
	if(eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
	{
		return -1;
	}
    if(netif_system_is_box() == 1)
        return eth_ifindex.eth_if.slot;
    else
	    return eth_ifindex.eth_if.chassis;
}

int eth_port_get_slot_by_ifindex(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = eth_g_index;
	if(eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
	{
		return -1;
	}
    if(netif_system_is_box() == 1)
        return eth_ifindex.eth_if.sub_slot;
    else
	    return eth_ifindex.eth_if.slot;
}

int eth_port_get_subslot_by_ifindex(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = eth_g_index;
	if(eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
	{
		return -1;
	}
    if(netif_system_is_box() == 1)
        return -1;
    else
	    return eth_ifindex.eth_if.sub_slot;
}

int eth_port_get_portno_by_ifindex(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = eth_g_index;
	if(eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
	{
		return -1;
	}
	return eth_ifindex.eth_if.port;
}

unsigned int eth_port_generate_ifindex(char chassis_id, char slot_id, char module_id, char port_id)
{
    NPD_NETIF_INDEX_U eth_ifindex;
    if(netif_system_is_box() == 1)
    {
    	eth_ifindex.netif_index = 0;
    	eth_ifindex.eth_if.type = NPD_NETIF_ETH_TYPE;
    	eth_ifindex.eth_if.chassis = 0;
    	eth_ifindex.eth_if.slot = chassis_id;
    	eth_ifindex.eth_if.sub_slot = slot_id;
    	eth_ifindex.eth_if.port = port_id;
    	eth_ifindex.eth_if.reserved = 0;
    }
    else
    {
    	eth_ifindex.netif_index = 0;
    	eth_ifindex.eth_if.type = NPD_NETIF_ETH_TYPE;
    	eth_ifindex.eth_if.chassis = chassis_id;
    	eth_ifindex.eth_if.slot = slot_id;
    	eth_ifindex.eth_if.sub_slot = module_id;
    	eth_ifindex.eth_if.port = port_id;
    	eth_ifindex.eth_if.reserved = 0;
    }
	return eth_ifindex.netif_index;
}


unsigned int generate_eth_index(char chassisno, char slotno, char moduleno, char portno, char subportno)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	if(chassisno < 0 || chassisno > MAX_CHASSIS_COUNT)
	{
		return 0;
	}
	if(slotno < 0 || slotno > MAX_CHASSIS_SLOT_COUNT)
	{
		return 0;
	}
	if(moduleno < 0 || moduleno > MAX_SLOT_SUBBOARD_COUNT)
	{
		return 0;
	}
	if(portno < 0 || portno > MAX_ETHPORT_PER_BOARD)
	{
		return 0;
	}
	eth_ifindex.netif_index = 0;
	eth_ifindex.eth_if.type = NPD_NETIF_ETH_TYPE;
    if(netif_system_is_box() == 1)
    {
    	eth_ifindex.eth_if.chassis = 0;
    	eth_ifindex.eth_if.slot = chassisno - 1;
    	eth_ifindex.eth_if.sub_slot = slotno;
    	eth_ifindex.eth_if.port = portno -1;
    	eth_ifindex.eth_if.reserved = subportno;
    }
    else
    {
    	eth_ifindex.eth_if.chassis = chassisno - 1;
    	eth_ifindex.eth_if.slot = slotno - 1;
    	eth_ifindex.eth_if.sub_slot = moduleno;
    	eth_ifindex.eth_if.port = portno - 1;
    	eth_ifindex.eth_if.reserved = subportno;
    }
	return eth_ifindex.netif_index;
}


int dcli_eth_port_array_index_from_ifindex(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	int eth_g_index_temp = 0;
	eth_ifindex.netif_index = eth_g_index;
	if(eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
	{
		return -1;
	}
	eth_g_index_temp = (eth_ifindex.eth_if.chassis*MAX_CHASSIS_SLOT_COUNT*MAX_ETHPORT_PER_BOARD);
	eth_g_index_temp += (eth_ifindex.eth_if.slot*MAX_ETHPORT_PER_BOARD);
    if(eth_ifindex.eth_if.sub_slot)
    {
        eth_g_index_temp += SUBBOARD_START_PORT+(eth_ifindex.eth_if.sub_slot - 1)*MAX_ETHPORT_PER_SUBBOARD;
        
    }
    eth_g_index_temp += (eth_ifindex.eth_if.port);
	return eth_g_index_temp;
}


int dcli_eth_port_array_index_to_ifindex(unsigned int eth_arr_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = 0;
	eth_ifindex.eth_if.type = NPD_NETIF_ETH_TYPE;
	eth_ifindex.eth_if.chassis = eth_arr_index/(MAX_CHASSIS_SLOT_COUNT*MAX_ETHPORT_PER_BOARD);
	eth_arr_index -= (eth_ifindex.eth_if.chassis*(MAX_CHASSIS_SLOT_COUNT*MAX_ETHPORT_PER_BOARD));
	eth_ifindex.eth_if.slot = eth_arr_index/(MAX_ETHPORT_PER_BOARD);
	eth_arr_index -= (eth_ifindex.eth_if.slot*(MAX_ETHPORT_PER_BOARD));
    if(eth_arr_index < SUBBOARD_START_PORT)
    {
        eth_ifindex.eth_if.sub_slot = 0;
	    eth_ifindex.eth_if.port = eth_arr_index;
    }
    else
    {
    	eth_ifindex.eth_if.sub_slot = ((eth_arr_index-SUBBOARD_START_PORT)/(MAX_ETHPORT_PER_SUBBOARD) + 1);
    	eth_arr_index -= ((eth_ifindex.eth_if.sub_slot - 1)*(MAX_ETHPORT_PER_SUBBOARD) + SUBBOARD_START_PORT);
    	eth_ifindex.eth_if.port = eth_arr_index;
    }
	return eth_ifindex.netif_index;
}

#ifdef HAVE_CHASSIS_SUPPORT
unsigned int stack_inter_port_generate_ifindex(char chassis_id, char slot, char port)
{
    NPD_NETIF_INDEX_U stack_ifindex;

	if(chassis_id < 0 || chassis_id >= MAX_CHASSIS_COUNT)
	{
		return 0;
	}

	stack_ifindex.netif_index = 0;
	stack_ifindex.stack_if.type = NPD_NETIF_STACK_TYPE;
    if(netif_system_is_box() == 1)
    {
    	stack_ifindex.stack_if.chassis = 0;
        stack_ifindex.stack_if.slot = chassis_id;
        stack_ifindex.stack_if.port = port + SUBBOARD_START_PORT + MAX_ETHPORT_PER_SUBBOARD*slot;
    }
    else
    {
    	stack_ifindex.stack_if.chassis = chassis_id;
        stack_ifindex.stack_if.slot = slot;
        stack_ifindex.stack_if.port = port + STACK_INTER_ETHPORT_START;
    }
	return stack_ifindex.netif_index;
    
}

#endif
unsigned long npd_netif_type_get(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    npd_netif_index.netif_index = netif_index;
    return npd_netif_index.common_if.type;
}

unsigned long npd_netif_eth_index(unsigned long chassis_id, unsigned long slot_id, unsigned long port_id)
{
    return eth_port_generate_ifindex(chassis_id, slot_id, 0, port_id);
}

unsigned long npd_netif_vlan_index(unsigned long vid)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    npd_netif_index.netif_index = 0;
    npd_netif_index.vlan_if.type = NPD_NETIF_VLAN_TYPE;
    npd_netif_index.vlan_if.vlanid = vid;
    return npd_netif_index.netif_index;
}

unsigned long npd_netif_trunk_index(unsigned long tid)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    npd_netif_index.netif_index = 0;
    npd_netif_index.trunk_if.type = NPD_NETIF_TRUNK_TYPE;
    npd_netif_index.trunk_if.trunkid = tid;
    return npd_netif_index.netif_index;
}

unsigned long npd_netif_eth_get_chassis(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;

    npd_netif_index.netif_index = netif_index;
    if(netif_system_is_box() == 1)
        return npd_netif_index.eth_if.slot;
    else
        return npd_netif_index.eth_if.chassis;
}

unsigned long npd_netif_eth_get_slot(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;

    npd_netif_index.netif_index = netif_index;
    if(netif_system_is_box() == 1)
        return npd_netif_index.eth_if.sub_slot;
    else
        return npd_netif_index.eth_if.slot;
}

int npd_netif_eth_get_subslot(unsigned int netif_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = netif_index;
	if(eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
	{
		return -1;
	}
	return eth_ifindex.eth_if.sub_slot;
}

unsigned long npd_netif_eth_get_port(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    unsigned long port_no = 0;
    npd_netif_index.netif_index = netif_index;
    port_no = npd_netif_index.eth_if.port;
    return port_no;
}


unsigned long npd_netif_vlan_get_vid(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    unsigned long vlan_id = 0;
    npd_netif_index.netif_index = netif_index;
    vlan_id = npd_netif_index.vlan_if.vlanid;
    return vlan_id;
}

unsigned long npd_netif_trunk_get_tid(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    unsigned long trunk_id = 0;
    npd_netif_index.netif_index = netif_index;
    trunk_id = npd_netif_index.trunk_if.trunkid;
    return trunk_id;
}

unsigned long npd_netif_tunnel_get_tunnelid(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    unsigned long tunnel_id = 0;
    npd_netif_index.netif_index = netif_index;
    tunnel_id = npd_netif_index.tunnel_if.tunnelid;
	return tunnel_id;
}

unsigned long npd_netif_vlan_get_index(unsigned short vid)
{
    NPD_NETIF_INDEX_U npd_netif_index;
	npd_netif_index.netif_index = 0;
    npd_netif_index.vlan_if.type = NPD_NETIF_VLAN_TYPE;
    npd_netif_index.vlan_if.vlanid = vid;
    return npd_netif_index.netif_index;
}

unsigned long npd_netif_trunk_get_index(unsigned int tid)
{
    NPD_NETIF_INDEX_U npd_netif_index;
	npd_netif_index.netif_index = 0;
    npd_netif_index.trunk_if.type = NPD_NETIF_TRUNK_TYPE;
    npd_netif_index.trunk_if.trunkid = tid;
    return npd_netif_index.netif_index;
}

unsigned long npd_netif_tunnel_get_index(unsigned int tunnel_id)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    npd_netif_index.netif_index = 0;
    npd_netif_index.tunnel_if.type = NPD_NETIF_TUNNEL_TYPE;
    npd_netif_index.tunnel_if.tunnelid = tunnel_id;
    return npd_netif_index.netif_index;
}




unsigned long npd_netif_vidx_get_index(unsigned int vidx)
{
    NPD_NETIF_INDEX_U npd_netif_index;
	npd_netif_index.netif_index = 0;
    npd_netif_index.vidx_if.type = NPD_NETIF_VIDX_TYPE;
    npd_netif_index.vidx_if.vidx = vidx;
    return npd_netif_index.netif_index;
}

unsigned long npd_netif_vidx_get_vidx(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
	npd_netif_index.netif_index = netif_index;
    return npd_netif_index.vidx_if.vidx;
}

int trunk_array_index_from_ifindex_in(unsigned int netif_index)
{
    int tid;
    unsigned int array_index;
    
    tid = npd_netif_trunk_get_tid(netif_index);
    array_index = TRUNK_BITMAP_START_PORT+tid;
    return array_index;
}

int trunk_array_index_to_ifindex_in(unsigned int netif_array_index)
{
    int tid;
    unsigned int netif_index;

    tid = netif_array_index - TRUNK_BITMAP_START_PORT;
    
    netif_index = npd_netif_trunk_get_index(tid);
	return netif_index;
}

int tunnel_array_index_from_ifindex_in(unsigned int netif_index)
{
    int tunnel_id;
    unsigned int array_index;
    
    tunnel_id = npd_netif_tunnel_get_tunnelid(netif_index);
    array_index = TUNNEL_BITMAP_START_PORT+tunnel_id;
    return array_index;
}

int tunnel_array_index_to_ifindex_in(unsigned int netif_array_index)
{
    unsigned int tunnelid;
    unsigned int netif_index;

    tunnelid = netif_array_index - TUNNEL_BITMAP_START_PORT;
    
    netif_index = npd_netif_tunnel_get_index(tunnelid);
	return netif_index;
}
int eth_port_array_index_from_ifindex_in(unsigned int eth_g_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	int eth_g_index_temp = 0;
	eth_ifindex.netif_index = eth_g_index;
	if(eth_ifindex.eth_if.type != NPD_NETIF_ETH_TYPE)
	{
		return -1;
	}
	eth_g_index_temp = (eth_ifindex.eth_if.chassis*MAX_CHASSIS_SLOT_COUNT*MAX_ETHPORT_PER_BOARD);
	eth_g_index_temp += (eth_ifindex.eth_if.slot*MAX_ETHPORT_PER_BOARD);
    if(eth_ifindex.eth_if.sub_slot)
    {
        eth_g_index_temp += SUBBOARD_START_PORT+(eth_ifindex.eth_if.sub_slot - 1)*MAX_ETHPORT_PER_SUBBOARD;
        
    }
	eth_g_index_temp += (eth_ifindex.eth_if.port);
	return eth_g_index_temp;
}

int eth_port_array_index_to_ifindex_in(unsigned int eth_arr_index)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = 0;
	eth_ifindex.eth_if.type = NPD_NETIF_ETH_TYPE;
	eth_ifindex.eth_if.chassis = eth_arr_index/(MAX_CHASSIS_SLOT_COUNT*MAX_ETHPORT_PER_BOARD);
	eth_arr_index -= (eth_ifindex.eth_if.chassis*(MAX_CHASSIS_SLOT_COUNT*MAX_ETHPORT_PER_BOARD));
	eth_ifindex.eth_if.slot = eth_arr_index/(MAX_ETHPORT_PER_BOARD);
	eth_arr_index -= (eth_ifindex.eth_if.slot*(MAX_ETHPORT_PER_BOARD));
    if(eth_arr_index < SUBBOARD_START_PORT)
    {
        eth_ifindex.eth_if.sub_slot = 0;
	    eth_ifindex.eth_if.port = eth_arr_index;
    }
    else
    {
    	eth_ifindex.eth_if.sub_slot = ((eth_arr_index-SUBBOARD_START_PORT)/(MAX_ETHPORT_PER_SUBBOARD) + 1);
    	eth_arr_index -= ((eth_ifindex.eth_if.sub_slot - 1)*(MAX_ETHPORT_PER_SUBBOARD) + SUBBOARD_START_PORT);
    	eth_ifindex.eth_if.port = eth_arr_index;
    }
	return eth_ifindex.netif_index;
}


int netif_array_index_from_ifindex(unsigned int netif_index)
{
    switch(npd_netif_type_get(netif_index))
    {
        case NPD_NETIF_ETH_TYPE:
            return eth_port_array_index_from_ifindex_in(netif_index);
        case NPD_NETIF_TRUNK_TYPE:
            return trunk_array_index_from_ifindex_in(netif_index);
        case NPD_NETIF_TUNNEL_TYPE:
            return tunnel_array_index_from_ifindex_in(netif_index);
        case NPD_NETIF_WIRELESS_TYPE:
            return wifi_port_array_index_from_ifindex_in(netif_index);
        default:
            return -1;
    }
}

int netif_array_index_to_ifindex(unsigned int array_index)
{
    if(array_index < MAX_ETHPORT_PER_SYSTEM)
        return eth_port_array_index_to_ifindex_in(array_index);
    else if(array_index < WIFI_BITMAP_START_PORT)
        return trunk_array_index_to_ifindex_in(array_index);
    else if(array_index < TUNNEL_BITMAP_START_PORT)
        return wifi_port_array_index_to_ifindex_in(array_index);
    else if(array_index < NPD_PBMP_PORT_MAX)
        return tunnel_array_index_to_ifindex_in(array_index);
    else
        return -1;
    
}

int trunk_array_index_from_ifindex(unsigned int netif_index)
{
    return netif_array_index_from_ifindex(netif_index);
}

int trunk_array_index_to_ifindex(unsigned int netif_array_index)
{
    return netif_array_index_to_ifindex(netif_array_index);
}

int tunnel_array_index_from_ifindex(unsigned int netif_index)
{
    return netif_array_index_from_ifindex(netif_index);
}

int tunnel_array_index_to_ifindex(unsigned int netif_array_index)
{
    return netif_array_index_to_ifindex(netif_array_index);
}
int eth_port_array_index_from_ifindex(unsigned int eth_g_index)
{
    return netif_array_index_from_ifindex(eth_g_index);
}

int eth_port_array_index_to_ifindex(unsigned int eth_arr_index)
{
    return netif_array_index_to_ifindex(eth_arr_index);
}


int npd_eth_index_to_name(unsigned int eth_g_index, char *name)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = eth_g_index;
	if(NULL == name)
	{
		return -1;
	}
	if(eth_ifindex.eth_if.type == NPD_NETIF_ETH_TYPE)
	{
        if(netif_system_is_box() == 1)
        {
   		    sprintf(name, "eth%d_%d_%d", eth_ifindex.eth_if.slot + 1, eth_ifindex.eth_if.sub_slot, eth_ifindex.eth_if.port + 1);
        }
        else
        {
    		if(eth_ifindex.eth_if.sub_slot != 0)
    		{
    		    sprintf(name, "eth%d_%d_%d_%d", eth_ifindex.eth_if.chassis+1, eth_ifindex.eth_if.slot + 1, 
                    eth_ifindex.eth_if.sub_slot, (eth_ifindex.eth_if.port + 1));
    		}
    		else
    		{
    		    sprintf(name, "eth%d_%d_%d", eth_ifindex.eth_if.chassis+1, eth_ifindex.eth_if.slot + 1,  eth_ifindex.eth_if.port + 1);
    		}
        }
		return 0;
	}
	return -1;
}

int npd_vlan_index_to_name(unsigned int netif_index, char *name)
{
    NPD_NETIF_INDEX_U vlan_ifindex;
	vlan_ifindex.netif_index = netif_index;
	if(NULL == name)
	{
		return -1;
	}
	if(vlan_ifindex.vlan_if.type == NPD_NETIF_VLAN_TYPE)
	{
		sprintf(name, "vlan%.4d",vlan_ifindex.vlan_if.vlanid);
		return 0;
	}
	return -1;
}

int npd_trunk_index_to_name(unsigned int netif_index, char *name)
{
    NPD_NETIF_INDEX_U trunk_ifindex;
	trunk_ifindex.netif_index = netif_index;
	if(NULL == name)
	{
		return -1;
	}
	if(trunk_ifindex.trunk_if.type == NPD_NETIF_TRUNK_TYPE)
	{
		sprintf(name, "lag%.3d",trunk_ifindex.trunk_if.trunkid);
		return 0;
	}
	return -1;
}

int npd_tunnel_index_to_name(unsigned int netif_index, char *name)
{
    NPD_NETIF_INDEX_U tunnel_ifindex;
	tunnel_ifindex.netif_index = netif_index;
	if(NULL == name)
	{
		return -1;
	}
	if(tunnel_ifindex.tunnel_if.type == NPD_NETIF_TUNNEL_TYPE)
	{
		sprintf(name, "tunnel%.3d",tunnel_ifindex.tunnel_if.tunnelid);
		return 0;
	}
	return -1;
}

int npd_netif_index_wifi_subinterface_name_get(unsigned int netif_index, char *wifi_subinterface_name)
{
	FILE			*fp = NULL;
	char	*position = NULL;
	unsigned char	Buff[64+1] = {};	
	unsigned int	bssindex_in_conf = 0;
	unsigned int	wlan_id = 0;
	unsigned int	bssindex = 0;
	unsigned int	search_flag = FALSE;


	fp = fopen("/var/run/wcpss/bssindex_wlan_relation", "r");
	if(!fp){
		sprintf(wifi_subinterface_name, "N/A");
		return -1;
	}

	bssindex = npd_netif_get_bssindex(netif_index);
	while(fgets((char*)Buff, 64,fp)){
		bssindex_in_conf = (unsigned int)atoi((char*)Buff);
		if(bssindex_in_conf == bssindex){
			unsigned int ap_id = 0;
			unsigned int radio_id = 0;

			search_flag = TRUE;
			position = strstr((char*)Buff, ":");
			wlan_id = (unsigned int)atoi((position +1));
			ap_id = npd_netif_wtpid_get(netif_index);
			radio_id = npd_netif_local_radio_id_get(netif_index);
			sprintf(wifi_subinterface_name, "Wifi %d-%d(WLAN %d)", ap_id, radio_id+1, wlan_id);
		}
	}
	
	fclose(fp);
	if(search_flag != TRUE){
		sprintf(wifi_subinterface_name, "N/A");
		return -1;
	}
	return 0;	
}

int npd_netif_index_to_user_fullname(unsigned int netif_index, char *ifname)
{
    if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
    {
        int chassisno = npd_netif_eth_get_chassis(netif_index)+1;
        int slot = npd_netif_eth_get_slot(netif_index)+1;
		int sub_slot = npd_netif_eth_get_subslot(netif_index);
        int port = npd_netif_eth_get_port(netif_index)+1;
        if(netif_system_is_box() == 1)
        {
            sprintf(ifname, "ethernet %d/%d/%d", chassisno, sub_slot, port);
        }
		else
		{
            sprintf(ifname, "ethernet %d/%d/%d", chassisno, slot, port);
		}
        return 0;
    }
    else if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
    {
        sprintf(ifname, "port-channel %d", (int)npd_netif_trunk_get_tid(netif_index));
        return 0;
    }
    else if(NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
    {
		sprintf(ifname, "vlan %d", (int)npd_netif_vlan_get_vid(netif_index));
        return 0;
    }
    else if(NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(netif_index))
    {
		npd_netif_index_wifi_subinterface_name_get(netif_index, ifname);
        return 0;
    }
	else if(NPD_NETIF_TUNNEL_TYPE == npd_netif_type_get(netif_index))
    {
		sprintf(ifname, "tunnel %d", (int)npd_netif_tunnel_get_tunnelid(netif_index));
        return 0;
    }
    else
    {
        sprintf(ifname, "%x", netif_index);
        return 0;
    }
    sprintf(ifname, "UNKNOWN");
    return -1;

}
int npd_netif_index_to_name(unsigned int netif_index, char *ifname)
{
	int netif_type = npd_netif_type_get(netif_index);
	if(ifname == NULL)
	{
		return -1;
	}
	switch(netif_type)
	{
		case NPD_NETIF_ETH_TYPE:
			return npd_eth_index_to_name(netif_index, ifname);
		case NPD_NETIF_VLAN_TYPE:
			return npd_vlan_index_to_name(netif_index, ifname);
		case NPD_NETIF_TRUNK_TYPE:
			return npd_trunk_index_to_name(netif_index, ifname);
	    case NPD_NETIF_WIRELESS_TYPE:
			return npd_netif_index_to_user_fullname(netif_index, ifname);
		case NPD_NETIF_TUNNEL_TYPE:
			return npd_tunnel_index_to_name(netif_index, ifname);
		default:
            sprintf(ifname, "UNKNOWN");
			return -1;
	}
}

int npd_netif_index_to_user_name(unsigned int netif_index, char *ifname)
{
    if(NPD_NETIF_ETH_TYPE == npd_netif_type_get(netif_index))
    {
        int chassisno = npd_netif_eth_get_chassis(netif_index)+1;
        int slot = npd_netif_eth_get_slot(netif_index)+1;
		int sub_slot = npd_netif_eth_get_subslot(netif_index);
        int port = npd_netif_eth_get_port(netif_index)+1;
        if(netif_system_is_box() == 1)
        {
            sprintf(ifname, "eth%d/%d/%d", chassisno, sub_slot, port);
        }
		else
            sprintf(ifname, "eth%d/%d/%d", chassisno, slot, port);
        return 0;
    }
    else if(NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(netif_index))
    {
        sprintf(ifname, "p-channel%d", (int)npd_netif_trunk_get_tid(netif_index));
        return 0;
    }
    else if(NPD_NETIF_VLAN_TYPE == npd_netif_type_get(netif_index))
    {
		sprintf(ifname, "VLAN%.4d", (int)npd_netif_vlan_get_vid(netif_index));
        return 0;
    }
	else if (NPD_NETIF_WIRELESS_TYPE == npd_netif_type_get(netif_index))
	{
		return npd_netif_index_to_user_fullname(netif_index, ifname);
	}
	
    else if(NPD_NETIF_TUNNEL_TYPE == npd_netif_type_get(netif_index))
    {
        sprintf(ifname, "tunnel%.3d", (int)npd_netif_tunnel_get_tunnelid(netif_index));
        return 0;
    }
    sprintf(ifname, "UNKNOWN");
    return -1;

}

int npd_netif_index_to_l3intf_name(unsigned int netif_index, char *ifname)
{
	int netif_type = npd_netif_type_get(netif_index);
	if(ifname == NULL)
	{
		return -1;
	}
	switch(netif_type)
	{
		case NPD_NETIF_ETH_TYPE:
		{
			int chassisno = npd_netif_eth_get_chassis(netif_index)+1;
	        int slot = npd_netif_eth_get_slot(netif_index)+1;
	        int port = npd_netif_eth_get_port(netif_index)+1;
            if(netif_system_is_box() == 1)
                slot = slot - 1;
	        sprintf(ifname, "eth%d/%d/%d", chassisno, slot, port);
	        return 0;
		}
		case NPD_NETIF_VLAN_TYPE:
			sprintf(ifname, "vlan %d", (int)npd_netif_vlan_get_vid(netif_index));
	        return 0;
		case NPD_NETIF_TRUNK_TYPE:
			sprintf(ifname, "lag%.3d", (int)npd_netif_trunk_get_tid(netif_index));
			return 0;
		case NPD_NETIF_TUNNEL_TYPE:			
			sprintf(ifname, "tunnel%.3d", (int)npd_netif_tunnel_get_tunnelid(netif_index));
			return 0;
		default:
			sprintf(ifname, "UNKNOWN");
			return -1;
	}
	
	return -1;
}



int parse_eth_index_to_name(unsigned int eth_g_index, char *name)
{
    NPD_NETIF_INDEX_U eth_ifindex;
	eth_ifindex.netif_index = eth_g_index;
	if(NULL == name)
	{
		return -1;
	}
	if(eth_ifindex.eth_if.type == NPD_NETIF_ETH_TYPE)
	{
        if(netif_system_is_box() == 1)
        {
		    sprintf(name, "%d/%d/%d",eth_ifindex.eth_if.slot + 1, eth_ifindex.eth_if.sub_slot, eth_ifindex.eth_if.port + 1);
        }
        else
		{
			if(eth_ifindex.eth_if.sub_slot != 0)
			{
				sprintf(name, "%d/%d/%d/%d",eth_ifindex.eth_if.chassis + 1, eth_ifindex.eth_if.slot + 1, eth_ifindex.eth_if.sub_slot, eth_ifindex.eth_if.port + 1);
			}
			else
			{
				sprintf(name, "%d/%d/%d",eth_ifindex.eth_if.chassis + 1, eth_ifindex.eth_if.slot + 1, eth_ifindex.eth_if.port + 1);
			}
		}
		return 0;
	}
	return -1;
}

unsigned long dcli_netif_type_get(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    npd_netif_index.netif_index = netif_index;
    return npd_netif_index.common_if.type;
}

int npd_netif_pbmp_group_create(int port_num, int bit_length, int bit_class_len ,npd_netif_pbmp_group_t **group)
{
    int i;
    if(bit_class_len > bit_length)
    {
        group = NULL;
        return -1;
    }
    *group = malloc(sizeof(npd_netif_pbmp_group_t));
    if(!*group)
        return -1;
    memset(*group, 0, sizeof(sizeof(npd_netif_pbmp_group_t)));
    (*group)->port_num = port_num;
    (*group)->bit_length = bit_length;
    (*group)->bit_class_len = bit_class_len;
    (*group)->port_2_bit = malloc(sizeof(unsigned int)*128);
    (*group)->bit_2_port = malloc(sizeof(unsigned int)*(bit_length-bit_class_len)*(1<<bit_class_len));
    (*group)->bit_alloc = malloc(sizeof(unsigned int)*(bit_length-bit_class_len)*(1<<bit_class_len));
    for(i = 0; i < (bit_length-bit_class_len)*(1<<bit_class_len); i++)
    {
        (*group)->bit_2_port[i] = -1;
        (*group)->bit_alloc[i] = -1;
    }
    for(i = 0; i < 128; i++)
        (*group)->port_2_bit[i] = -1;

    return 0;
}

int npd_netif_pbmp_group_entry_alloc(npd_netif_pbmp_group_t *group, unsigned int port, unsigned int *bit)
{
    int i;
    int portmax = (group->bit_length-group->bit_class_len)*(1<<group->bit_class_len);

    if(port > 128)
    {
        *bit = -1;
        return -1;
    }
    if(-1 != group->port_2_bit[port])
    {
        *bit = group->port_2_bit[port];
        return 0;
    }
    
    for(i = portmax; i > 0; i--)
    {
        if(-1 == group->bit_alloc[i])
        {
           group->port_2_bit[port] = (1<<(i%(group->bit_length-group->bit_class_len)))
                + ((i/(group->bit_length-group->bit_class_len)) << (group->bit_length-group->bit_class_len));
           group->bit_2_port[i] = port;
           group->bit_alloc[i] = 1;
           if(group->port_2_bit[port] >> (group->bit_length-group->bit_class_len) == 0)
           {
               group->port_2_bit[port] |= 0x1;
           }
           *bit = group->port_2_bit[port];
           break;
        }
    }
    return 0;
}

int npd_netif_pbmp_group_entry_free(npd_netif_pbmp_group_t *group, unsigned int port, unsigned int bit)
{
    int i;

    if((port > 128) || (bit & (-1<<group->bit_length)))
        return -1;

    if(group->port_2_bit[port] != bit)
        return -1;

    i = (bit>>(group->bit_length-group->bit_class_len)) * (group->bit_length-group->bit_class_len);
    while((i&1) != 1)
    {
        /*bit >> 1;*/
        i++;
    }
       
    group->bit_alloc[i] = -1;
    group->bit_2_port[i] = -1;
    group->port_2_bit[port] = -1;

    return 0;
}

int npd_netif_pbmp_group_get_bit(npd_netif_pbmp_group_t *group, unsigned int port, unsigned int *bit)
{
    if(port > 128)
    {
        *bit = -1;
        return -1;
    }

    *bit = group->port_2_bit[port];
    if(-1 == *bit)
        return -1;
    return 0;
}
int npd_netif_pbmp_group_get_port(npd_netif_pbmp_group_t *group, unsigned int bit, unsigned int *port)
{
    int i;
    int portmax = (group->bit_length-group->bit_class_len)*(1<<group->bit_class_len);

    i = (bit>>(group->bit_length-group->bit_class_len)) * (group->bit_length-group->bit_class_len);
    while((i&1) != 1)
    {
        /*bit >> 1;*/
        i++;
    }
    if(i > portmax)
    {
        *port = -1;
        return -1;
    }

    *port = group->bit_2_port[i];
    if(-1 == *port)
        return -1;
    return 0;
}

int npd_netif_pbmp_group_class_bitempty(npd_netif_pbmp_group_t *group, unsigned int bit_class)
{

    return (0 == (bit_class % (1<<(group->bit_length-group->bit_class_len))));
    
}

int npd_netif_pbmp_group_same_class(npd_netif_pbmp_group_t *group, unsigned int bit1, unsigned int bit2)
{
    return ( (bit1 >> (group->bit_length-group->bit_class_len))
                == (bit2 >> (group->bit_length-group->bit_class_len)) );
}

unsigned int npd_netif_pbmp_group_get_port_bit(npd_netif_pbmp_group_t *group, unsigned int bit1)
{
    return (bit1 % (1<<(group->bit_length-group->bit_class_len)));
}

unsigned int npd_netif_pbmp_group_get_port_mask(npd_netif_pbmp_group_t *group)
{
    return (~1UL >> (32 - group->bit_length + group->bit_class_len));
}

unsigned int npd_netif_pbmp_group_get_class_bit(npd_netif_pbmp_group_t *group, unsigned int bit1)
{
    return (bit1>>(group->bit_length-group->bit_class_len))<<(group->bit_length-group->bit_class_len);
}

unsigned int npd_netif_pbmp_group_get_class(npd_netif_pbmp_group_t *group, unsigned int bit)
{
    return (bit>>(group->bit_length-group->bit_class_len));
}
unsigned int npd_netif_pbmp_group_get_class_mask(npd_netif_pbmp_group_t *group)
{
    return (~1UL >> (32 - group->bit_class_len)) << (group->bit_length-group->bit_class_len);
}

int npd_netif_pbmp_group_free(npd_netif_pbmp_group_t **group)
{
    free((*group)->bit_alloc);
    free((*group)->port_2_bit);
    free((*group)->bit_2_port);
    free(*group);
    *group = NULL;
	return 0;
}

/*get netif index from wtp_id, local radio id and local bss index*/
unsigned long npd_netif_get_from_wtp_radio_bss(int wtp_id, int radio_local_id, int bss_local_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;

    
	npd_netif_index.netif_index = 0;

    npd_netif_index.wireless_if.type  			= NPD_NETIF_WIRELESS_TYPE;
    npd_netif_index.wireless_if.wtpid  			= wtp_id;
	npd_netif_index.wireless_if.radio_local_id	= radio_local_id;
	npd_netif_index.wireless_if.bss_local_index = bss_local_index;
    
    return npd_netif_index.netif_index;
}

/*get wtp id from netif index*/
int npd_netif_wtpid_get(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    
    npd_netif_index.netif_index = netif_index;
    if(npd_netif_index.wireless_if.type != NPD_NETIF_WIRELESS_TYPE){
    	return -1;
    }

    return npd_netif_index.wireless_if.wtpid;
}

/*get local radio id from netif index*/
int npd_netif_local_radio_id_get(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    npd_netif_index.netif_index = netif_index;
    if(npd_netif_index.wireless_if.type != NPD_NETIF_WIRELESS_TYPE){
    	return -1;
    }

	return npd_netif_index.wireless_if.radio_local_id;
}

/*get local bss index from netif index*/
int npd_netif_local_bssindex_get(unsigned long netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;

    npd_netif_index.netif_index = netif_index;
    if(npd_netif_index.wireless_if.type != NPD_NETIF_WIRELESS_TYPE){
    	return -1;
    }

	return  npd_netif_index.wireless_if.bss_local_index;
}

/*get wtp id from netif index*/
unsigned int npd_netif_get_bssindex(unsigned int netif_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;
    
    npd_netif_index.netif_index = netif_index;
    if(npd_netif_index.wireless_if.type != NPD_NETIF_WIRELESS_TYPE){
    	return -1;
    }

    return ((npd_netif_index.wireless_if.wtpid * NETIF_LOCAL_RADIO_NUM) + npd_netif_index.wireless_if.radio_local_id)*NETIF_LOCAL_BSS_NUM + npd_netif_index.wireless_if.bss_local_index;
}

unsigned int npd_netif_bssindex_to_netif_index(unsigned int bss_index)
{
    NPD_NETIF_INDEX_U npd_netif_index;

	npd_netif_index.wireless_if.type = NPD_NETIF_WIRELESS_TYPE;
	npd_netif_index.wireless_if.wtpid = (bss_index/NETIF_LOCAL_BSS_NUM)/NETIF_LOCAL_RADIO_NUM;
	npd_netif_index.wireless_if.radio_local_id = (bss_index/NETIF_LOCAL_BSS_NUM)%NETIF_LOCAL_RADIO_NUM;
	npd_netif_index.wireless_if.bss_local_index = bss_index%NETIF_LOCAL_BSS_NUM;

	return npd_netif_index.netif_index;
}

int wifi_port_array_index_from_ifindex_in(unsigned int netif_index)
{
    NPD_NETIF_INDEX_U wifi_ifindex;
    int g_bss_index = 0;

	
	wifi_ifindex.netif_index = netif_index;
	if(wifi_ifindex.wireless_if.type != NPD_NETIF_WIRELESS_TYPE)
	{
		return -1;
	}
	g_bss_index = npd_netif_get_bssindex(netif_index);
	return (g_bss_index == -1? -1: (WIFI_BITMAP_START_PORT+g_bss_index));
}

int wifi_port_array_index_to_ifindex_in(unsigned int wifi_arr_index)
{
    int g_bss_index = 0;


    g_bss_index = wifi_arr_index - WIFI_BITMAP_START_PORT;
	return npd_netif_bssindex_to_netif_index(g_bss_index);
}



#ifdef __cplusplus
}
#endif

