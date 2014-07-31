
#ifndef _NPD_GARP_H_
#define _NPD_GARP_H_

typedef struct
{
    struct list_head list;
	unsigned short vlan_id;
	unsigned short is_static;
}gvrp_vlan_entry_list;

typedef struct /* Garp */
{
    int process_id;
    void *gid;
    unsigned *gip;

    unsigned max_gid_index;

    unsigned last_gid_used;

    void(*join_indication_fn)(void *, void *my_port, unsigned short joining_gid_index,int directly);
    void(*leave_indication_fn)(void *, void *gid,
                               unsigned short leaving_gid_index, int directly);
    void(*join_propagated_fn)(void *, void *gid,
                              unsigned short joining_gid_index);
    void(*leave_propagated_fn)(void *, void *gid,
                               unsigned short leaving_gid_index);
    void(*transmit_fn)(void *, void *gid);
    void (*receive_fn)(void *, void *gid, void *pdu);
    void (*added_port_fn)(void *, unsigned int netif_index);
    void (*removed_port_fn)(void *, unsigned int netif_index);
} GARP;
#endif

