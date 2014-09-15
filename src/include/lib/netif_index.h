#ifndef __NETIF_INDEX_H__
#define __NETIF_INDEX_H__

#ifdef HAVE_MEMORY_SHORT
#define MAX_CHASSIS_COUNT 1
#define MAX_CHASSIS_SLOT_COUNT 1
#define MAX_SLOT_SUBBOARD_COUNT 1
#define MAX_ETHPORT_PER_BOARD 64
#define MAX_SUBPORT_PER_ETHPORT 1
#define MAX_ETHPORT_PER_SYSTEM 64
#define MAX_SWITCHPORT_PER_SYSTEM 72
#define SUBBOARD_START_PORT   52
#define MAX_ETHPORT_PER_SUBBOARD 4
#define MAX_TRUNK_PER_SYSTEM   16
#define CHASSIS_VLAN_RANGE_MIN		1		/*added by wujh*/
#define CHASSIS_VLAN_RANGE_MAX		4096
#define CHASSIS_VIDX_RANGE_MAX		4095
#define CHASSIS_TRUNK_RANGE_MAX		8
#define CHASSIS_WIFI_RANGE_MAX      0
#define CHASSIS_TUNNEL_RANGE_MAX	0
#define TRUNK_BITMAP_START_PORT   (MAX_ETHPORT_PER_SYSTEM)
#define WIFI_BITMAP_START_PORT	  (MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX)
#define TUNNEL_BITMAP_START_PORT	  (MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX+CHASSIS_WIFI_RANGE_MAX)
#else

    #ifdef HAVE_CHASSIS_SUPPORT
    #define MAX_CHASSIS_COUNT 1
    #define MAX_CHASSIS_SLOT_COUNT 16
    #define MAX_SLOT_SUBBOARD_COUNT 5
    #define MAX_ETHPORT_PER_BOARD 64
    #define MAX_SUBPORT_PER_ETHPORT 1
    #define SUBBOARD_START_PORT   52
    #define STACK_INTER_ETHPORT_START   16
    #define MAX_ETHPORT_PER_SUBBOARD 4
    #define MAX_ETHPORT_PER_SYSTEM 1024
    #define MAX_SWITCHPORT_PER_SYSTEM 2048
    #define MAX_TRUNK_PER_SYSTEM   127
    #define CHASSIS_VLAN_RANGE_MIN		1		/*added by wujh*/
    #define CHASSIS_VLAN_RANGE_MAX		4096
    #define CHASSIS_VIDX_RANGE_MAX		4095
    #define CHASSIS_TRUNK_RANGE_MAX		127
    #define STACK_INTER_TRUNK_START      112
    #define CHASSIS_WIFI_RANGE_MAX       512         
    #define CHASSIS_TUNNEL_RANGE_MAX	    512
    #define TRUNK_BITMAP_START_PORT		(MAX_ETHPORT_PER_SYSTEM)
    #define WIFI_BITMAP_START_PORT	(MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX+1)
    #define TUNNEL_BITMAP_START_PORT		(MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX+CHASSIS_WIFI_RANGE_MAX+1)
    #else
    
        #ifdef HAVE_MULTI_CHASSIS_SUPPORT
        #define MAX_CHASSIS_COUNT 4
        #define MAX_CHASSIS_SLOT_COUNT 16
        #define MAX_SLOT_SUBBOARD_COUNT 5
        #define MAX_ETHPORT_PER_BOARD 64
        #define MAX_SUBPORT_PER_ETHPORT 1
        #define SUBBOARD_START_PORT   52
        #define STACK_INTER_ETHPORT_START   16
        #define MAX_ETHPORT_PER_SUBBOARD 4
        #define MAX_ETHPORT_PER_SYSTEM 4096
        #define MAX_SWITCHPORT_PER_SYSTEM 5120
        #define MAX_TRUNK_PER_SYSTEM   127
        #define STACK_INTER_TRUNK_START      112
        #define CHASSIS_VLAN_RANGE_MIN		1		/*added by wujh*/
        #define CHASSIS_VLAN_RANGE_MAX		4096
        #define CHASSIS_VIDX_RANGE_MAX		4095
        #define CHASSIS_TRUNK_RANGE_MAX		127
        #define CHASSIS_WIFI_RANGE_MAX       512         
        #define CHASSIS_TUNNEL_RANGE_MAX	    512
        #define TRUNK_BITMAP_START_PORT		(MAX_ETHPORT_PER_SYSTEM)
        #define WIFI_BITMAP_START_PORT	(MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX+1)
        #define TUNNEL_BITMAP_START_PORT		(MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX+CHASSIS_WIFI_RANGE_MAX+1)
        #else
        #define MAX_CHASSIS_COUNT 1
        #define MAX_CHASSIS_SLOT_COUNT 2
        #define MAX_SLOT_SUBBOARD_COUNT 5
        #define MAX_ETHPORT_PER_BOARD 64
        #define SUBBOARD_START_PORT   64
        #define MAX_SUBPORT_PER_ETHPORT 4
        #define MAX_ETHPORT_PER_SUBBOARD 4
        #define MAX_ETHPORT_PER_SYSTEM 512
        #define MAX_SWITCHPORT_PER_SYSTEM 1024
        #define MAX_TRUNK_PER_SYSTEM   127
        #define CHASSIS_VLAN_RANGE_MIN		1		/*added by wujh*/
        #define CHASSIS_VLAN_RANGE_MAX		4096
        #define CHASSIS_VIDX_RANGE_MAX		4095
        #define CHASSIS_TRUNK_RANGE_MAX		127
        #define CHASSIS_WIFI_RANGE_MAX       0         
        #define CHASSIS_TUNNEL_RANGE_MAX	    512
        #define TRUNK_BITMAP_START_PORT		(MAX_ETHPORT_PER_SYSTEM)
        #define WIFI_BITMAP_START_PORT	(MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX+1)
        #define TUNNEL_BITMAP_START_PORT		(MAX_ETHPORT_PER_SYSTEM+CHASSIS_TRUNK_RANGE_MAX+CHASSIS_WIFI_RANGE_MAX+1)
		#endif
    #endif
#endif
#define MAX_ETH_GLOBAL_INDEX MAX_ETHPORT_PER_SYSTEM

#define NETIF_LOCAL_RADIO_NUM		4
#define NETIF_LOCAL_BSS_NUM			8

#define NPD_GLOBAL_NETIF_INDEX         0x0
#define NPD_NETIF_NULL_TYPE            (0x0)
#define NPD_NETIF_LOOPBACK_TYPE  (0x1)
#define NPD_NETIF_ETH_TYPE             (0x2)
#define NPD_NETIF_VLAN_TYPE            (0x3)
#define NPD_NETIF_TRUNK_TYPE           (0x4)
#define NPD_NETIF_PON_OLT_TYPE         (0x5)
#define NPD_NETIF_PON_ONU_TYPE         (0x6)
#define NPD_NETIF_TUNNEL_TYPE              (0x7)
#define NPD_NETIF_MGRE_TYPE            (0x8)
#define NPD_NETIF_SUB_TYPE             (0x9)
#define NPD_NETIF_VPLS_TYPE            (0xA)
#define NPD_NETIF_PPPOE_TYPE           (0xB)
#define NPD_NETIF_VT_TYPE              (0xC)
#define NPD_NETIF_STACK_TYPE           (0xD)
#define NPD_NETIF_VIDX_TYPE            (0xB)
#define NPD_NETIF_WIRELESS_TYPE	       (0xE)
#define NPD_NETIF_MAX_TYPE             (0xF)

typedef union _NPD_NETIF_INDEX_U_
{
#if __BYTE_ORDER == __BIG_ENDIAN
    struct _common_if_
    {
        unsigned type:4;
        unsigned other:28;
    }common_if;
    struct _null_if_
    {
        unsigned type:4;
        unsigned nullid:12;
        unsigned reserved:16;
    }null_if;
    struct _loopback_if_
    {
        unsigned type:4;
        unsigned loopbackid:12;
        unsigned reserved:16;
    }loopback_if;
    struct _eth_if_
    {
        unsigned type:4;
        unsigned chassis:2;
        unsigned slot:4;
        unsigned sub_slot:2;
        unsigned port:6;
        unsigned reserved:14;
    }eth_if;
    struct _vlan_if_
    {
        unsigned type:4;
        unsigned vlanid:12;
        unsigned reserved:16;
    }vlan_if;
    struct _trunk_if_
    {
        unsigned type:4;
        unsigned trunkid:12;
        unsigned reserved:16;
    }trunk_if;
    struct _pon_olt_if_
    {
        unsigned type:4;
        unsigned chassis:2;
        unsigned slot:4;
        unsigned sub_slot:3;
        unsigned port:6;
        unsigned reserved:12;
    }pon_olt_if;
    struct _pon_onu_if_
    {
        unsigned type:4;
        unsigned chassis:2;
        unsigned slot:4;
        unsigned sub_slot:3;
        unsigned port:6;
        unsigned onu_port:12;
    }pon_onu_if;
    struct _tunnel_if_
    {
        unsigned type:4;
        unsigned tunnelid:10;
        unsigned reserved:18;
    }tunnel_if;
    struct _stack_if_
    {
        unsigned type:4;
        unsigned chassis:2;
        unsigned slot:4;
        unsigned port:6;
        unsigned reserved:16;
    }stack_if;
    struct _vidx_if_
    {
        unsigned type:4;
        unsigned vidx:12;
        unsigned reserved:16;
    }vidx_if;
    struct _wireless_if_
    {
        unsigned type:4;
        unsigned wtpid:6;
        unsigned radio_local_id:2;
        unsigned bss_local_index:3;
        unsigned reserved:17;
    }wireless_if;
#else
	struct _common_if_
    {
	    unsigned other:28;
        unsigned type:4;        
    }common_if;
    struct _null_if_
    {
        unsigned reserved:16;
		unsigned nullid:12;
		unsigned type:4;
    }null_if;
    struct _loopback_if_
    {
        unsigned reserved:16;
		unsigned loopbackid:12;
		unsigned type:4;
    }loopback_if;
    struct _eth_if_
    {
        unsigned reserved:14;
		unsigned port:6;
		unsigned sub_slot:2;
		unsigned slot:4;
		unsigned chassis:2;
		unsigned type:4;
    }eth_if;
    struct _vlan_if_
    {
        unsigned reserved:16;
		unsigned vlanid:12;
		unsigned type:4;
    }vlan_if;
    struct _trunk_if_
    {
        unsigned reserved:16;
		unsigned trunkid:12;
		unsigned type:4;
    }trunk_if;
    struct _tunnel_if_
    {
        unsigned reserved:18;
        unsigned tunnelid:10;
        unsigned type:4;
    }tunnel_if;
    struct _pon_olt_if_
    {
        unsigned reserved:12;
		unsigned port:6;
		unsigned sub_slot:3;
		unsigned slot:4;
		unsigned chassis:3;
		unsigned type:4;
    }pon_olt_if;
    struct _pon_onu_if_
    {
        unsigned onu_port:12;
		unsigned port:6;
		unsigned sub_slot:3;
		unsigned slot:4;
		unsigned chassis:3;
		unsigned type:4;
    }pon_onu_if;
    struct _stack_if_
    {
        unsigned reserved:16;
        unsigned port:6;
		unsigned slot:4;
        unsigned chassis:2;
		unsigned type:4;
    }stack_if;
    struct _vidx_if_
    {
        unsigned reserved:16;
		unsigned vidx:12;
		unsigned type:4;
    }vidx_if;
    struct _wireless_if_
    {
        unsigned reserved:17;
        unsigned bss_local_index:3;
        unsigned radio_local_id:2;
		unsigned wtpid:6;
		unsigned type:4;
    }wireless_if;
#endif
    unsigned long netif_index;
} NPD_NETIF_INDEX_U;
#define NPD_IFINDEX_TYPE(ifindex) ((ifindex & 0xf0000000)>>28)

unsigned int eth_port_generate_ifindex(char chassis_id, char slot_id, char module_id, char port_id, char sub_port);

int wifi_port_array_index_from_ifindex_in(unsigned int netif_index);
int wifi_port_array_index_to_ifindex_in(unsigned int wifi_arr_index);
int netif_array_index_to_ifindex(unsigned int array_index);
int netif_array_index_from_ifindex(unsigned int netif_index);
int eth_port_array_index_from_ifindex(unsigned int eth_g_index);

int eth_port_array_index_to_ifindex(unsigned int eth_arr_index);

int trunk_array_index_from_ifindex(unsigned int netif_index);

int trunk_array_index_to_ifindex(unsigned int netif_array_index);

int tunnel_array_index_from_ifindex(unsigned int netif_index);

int tunnel_array_index_to_ifindex(unsigned int netif_array_index);

unsigned long npd_netif_type_get(unsigned long netif_index);

unsigned long npd_netif_eth_index(unsigned long chassis_id, unsigned long slot_id, unsigned long port_id);

unsigned long npd_netif_vlan_index(unsigned long vid);

unsigned long npd_netif_trunk_index(unsigned long tid);

unsigned long npd_netif_eth_get_chassis(unsigned long netif_index);

unsigned long npd_netif_eth_get_slot(unsigned long netif_index);

unsigned long npd_netif_eth_get_port(unsigned long netif_index);
unsigned long npd_netif_eth_get_sub_port(unsigned long netif_index);

unsigned long npd_netif_vlan_get_vid(unsigned long netif_index);
unsigned long npd_netif_trunk_get_tid(unsigned long netif_index);

unsigned long npd_netif_vlan_get_index(unsigned short vid);

unsigned long npd_netif_trunk_get_index(unsigned int tid);

int npd_eth_index_to_name(unsigned int eth_g_index, char *name);

int npd_vlan_index_to_name(unsigned int netif_index, char *name);

int npd_trunk_index_to_name(unsigned int netif_index, char *name);

int npd_netif_index_to_name(unsigned int netif_index, char *ifname);

int npd_netif_index_to_user_name(unsigned int netif_index, char *ifname);

int npd_netif_index_to_user_fullname(unsigned int netif_index, char *ifname);
int npd_netif_index_to_l3intf_name(unsigned int netif_index, char *ifname);


unsigned int generate_eth_index(char chassisno, char slotno, char moduleno, char portno, char subportno);
int eth_port_get_slot_by_ifindex(unsigned int eth_g_index);
int eth_port_get_portno_by_ifindex(unsigned int eth_g_index);
unsigned int npd_netif_get_bssindex(unsigned int netif_index);


int parse_eth_index_to_name(unsigned int eth_g_index, char *name);

/*本结构主要目的是用有限的位数来完成更多的port的bitmap,多用于TCAM的处理*/
typedef struct npd_netif_pbmp_group_s
{
    int port_num;    /*表示的port的数目*/
    int bit_length;   /*用的位数*/
    int bit_class_len; /*用的做分类的位数*/
    unsigned int *bit_alloc;
    unsigned int *port_2_bit;
    unsigned int *bit_2_port;
}npd_netif_pbmp_group_t;

#ifndef HAVE_CHASSIS_SUPPORT
#define NETIF_PBMP_GROUP_MAX_CLASS 8
#else
#define NETIF_PBMP_GROUP_MAX_CLASS 64
#endif


int npd_netif_pbmp_group_create(int port_num, int bit_length, int bit_class_len, npd_netif_pbmp_group_t **group);
int npd_netif_pbmp_group_entry_alloc(npd_netif_pbmp_group_t *group, unsigned int port, unsigned int *bit);
int npd_netif_pbmp_group_entry_free(npd_netif_pbmp_group_t *group, unsigned int port, unsigned int bit);
int npd_netif_pbmp_group_get_bit(npd_netif_pbmp_group_t *group, unsigned int port, unsigned int *bit);
int npd_netif_pbmp_group_get_port(npd_netif_pbmp_group_t *group, unsigned int bit, unsigned int *port);
int npd_netif_pbmp_group_class_bitempty(npd_netif_pbmp_group_t *group, unsigned int bit_class);
int npd_netif_pbmp_group_free(npd_netif_pbmp_group_t **group);
int npd_netif_pbmp_group_class_bitempty(npd_netif_pbmp_group_t *group, unsigned int bit_class);
int npd_netif_pbmp_group_same_class(npd_netif_pbmp_group_t *group, unsigned int bit1, unsigned int bit2);
unsigned int npd_netif_pbmp_group_get_port_bit(npd_netif_pbmp_group_t *group, unsigned int bit1);
unsigned int npd_netif_pbmp_group_get_port_mask(npd_netif_pbmp_group_t *group);
unsigned int npd_netif_pbmp_group_get_class_bit(npd_netif_pbmp_group_t *group, unsigned int bit1);
unsigned int npd_netif_pbmp_group_get_class_mask(npd_netif_pbmp_group_t *group);


unsigned long npd_netif_get_from_wtp_radio_bss(int wtp_id, int radio_local_id, int bss_local_index);
int npd_netif_local_bssindex_get(unsigned long netif_index);
int npd_netif_local_radio_id_get(unsigned long netif_index);
int npd_netif_wtpid_get(unsigned long netif_index);
int npd_netif_eth_get_subslot(unsigned int netif_index);



#endif
