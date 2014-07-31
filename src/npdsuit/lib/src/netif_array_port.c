
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* netif_array_port.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD by SWITCH PORT module.
*
* DATE:
*		08/08/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.256 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*
  Network Platform Daemon Ethernet Port Management
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "util/npd_list.h"
#include "npd_database.h"
#include "netif_array_port.h"
#include "netif_index.h"


db_table_t *array_ports_db = NULL;
array_table_index_t *array_ports = NULL;
hash_table_index_t *array_ports_hash = NULL;

unsigned int array_port_hash_key(void *data)
{
    array_port_entry_t *array_port = (array_port_entry_t*)data;

	return netif_array_index_from_ifindex(array_port->netif_index)%MAX_ARRAY_PORT;
}

unsigned int array_port_hash_cmp(void *data1, void *data2)
{
    array_port_entry_t *switch_port1 = (array_port_entry_t*)data1;
    array_port_entry_t *switch_port2 = (array_port_entry_t*)data2;

    return (switch_port1->netif_index == switch_port2->netif_index);
}

void init_array_ports(
    int sync_flag
    )
{
    char name[16];

    if(NULL != array_ports)
        return;
    strcpy(name, "ARRAY_PORT_DB");
    create_dbtable(name, MAX_ARRAY_PORT, sizeof(struct array_port_entry_s),
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, sync_flag, &array_ports_db);

    dbtable_create_array_index("array_index", array_ports_db, &array_ports);
	if(NULL == array_ports)
	{
		perror("memory alloc error for array port init!!!\n");
		return;
	}

    dbtable_create_hash_index("netif_index", array_ports_db, MAX_ARRAY_PORT, 
          &array_port_hash_key, &array_port_hash_cmp,
          &array_ports_hash);

	return;
}


void create_array_port
(
	unsigned int netif_index
) 
{
    int index;
    array_port_entry_t switch_port;

	memset( &switch_port, 0, sizeof(array_port_entry_t));
    switch_port.netif_index = netif_index;
    index = netif_array_index_from_ifindex(netif_index);
    switch_port.array_index = index;

    dbtable_array_insert_byid(array_ports, index, (void*)&switch_port);

	return;
}

void delete_array_port
(
    unsigned int netif_index
) 
{
    array_port_entry_t array_port;
    int array_index;
    int ret;

    array_port.netif_index = netif_index;
    ret = dbtable_hash_search(array_ports_hash, &array_port, NULL, &array_port);
    if(0 == ret)
    {
        array_index = array_port.array_index;

        dbtable_array_delete(array_ports, array_index, (void*)&array_port);
    }
	return;
	
}

unsigned int array_port_array_index(
    unsigned int netif_index,
    unsigned int *array_index
    )
{
    array_port_entry_t array_port;
    int ret;

    array_port.netif_index = netif_index;
    ret = dbtable_hash_search(array_ports_hash, &array_port, NULL, &array_port);
    if(0 == ret)
        *array_index = array_port.array_index;
    else
        *array_index = -1;

    return ret;
}

unsigned int array_port_netif_index(
    unsigned int array_index,
    unsigned int *netif_index
    )
{
    array_port_entry_t array_port;
    int ret;

    ret = dbtable_array_get(array_ports, array_index, &array_port);
    if(0 == ret)
        *netif_index = array_port.netif_index;
    else
        *netif_index = -1;

    return ret;
}

#ifdef __cplusplus
}
#endif

