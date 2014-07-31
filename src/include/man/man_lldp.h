#ifndef _MAN_LLDP_H
#define _MAN_LLDP_H

#include <stdint.h>
#include <unistd.h> 

#define LLDP_START_NAME "lldp_start.sh"
#define LLDP_STOP_NAME "lldp_stop.sh"
#define STATE_SIZE 7
#define DOT1_RESERVED 0
#define DOT1_PVID_TLV 1
#define DOT1_PPVID_TLV 2
#define DOT1_VNAME_TLV 3
#define LLDP_MULTIPLIER_CONFIG 1
#define LLDP_TX_DALAY_CONFIG 2
#define LLDP_TX_INTERVAL_CONFIG 3
#define LLDP_REINIT_DELAY_CONFIG 4
#define LLDP_ADMIN_STATUS_CONFIG 5
#define LLDP_CONFIG_DUMP_ERROR -1
#define LLDP_CONFIG_UPDATE_ERROR -2
#define SYSTEM_CAPABILITY_OTHER     1
#define SYSTEM_CAPABILITY_REPEATER  2
#define SYSTEM_CAPABILITY_BRIDGE    4
#define SYSTEM_CAPABILITY_WLAN      8
#define SYSTEM_CAPABILITY_ROUTER    16
#define SYSTEM_CAPABILITY_TELEPHONE 32
#define SYSTEM_CAPABILITY_DOCSIS    64
#define SYSTEM_CAPABILITY_STATION   128
#define END_OF_LLDPDU_TLV       0    /* MANDATORY */
#define CHASSIS_ID_TLV          1    /* MANDATORY */
#define PORT_ID_TLV             2    /* MANDATORY */
#define TIME_TO_LIVE_TLV        3    /* MANDATORY */
#define PORT_DESCRIPTION_TLV    4    /* OPTIONAL  */
#define SYSTEM_NAME_TLV         5    /* OPTIONAL  */
#define SYSTEM_DESCRIPTION_TLV  6    /* OPTIONAL  */
#define SYSTEM_CAPABILITIES_TLV 7    /* OPTIONAL  */
#define MANAGEMENT_ADDRESS_TLV  8    /* OPTIONAL  */
#define ORG_SPECIFIC_TLV        127  /* OPTIONAL */
#define CHASSIS_INFO_SIZE 255
#define PORT_INFO_SIZE 255
#define SYS_CAP_SIZE 256
#define SYS_NAME_SIZE 100
#define SYS_DESC_SIZE 100
#define PORT_DESC_SIZE 100
#define TTL_INFO_SIZE 2
#define PVID_INFO_SIZE 6
#define PPVID_INFO_SIZE 7
#define MAC_PHY_SIZE 9
#define GLOBAL_NUM 1
#define LLDP_SERVICE 1377
#define LLDP_LOCAL_SERVICE 1569
#define IF_NAME_SIZE 16
#define LLDP_FRAME_SIZE 1024
#define LLDP_SHUTDOWN_SIZE 3
#define MAX_VNAME_SIZE 32
#define CHASSIS_SUBTYPE_SIZE 30
#define PORT_SUBTYPE_SIZE 30
#define MANAGE_ADDR_SIZE 31
#define OID_STRING_SIZE 128
#define IANA_ADDR_SIZE 50
#define INTERFACE_SUBTYPE_SIZE 20
struct chassis_id_info{
    char sub_type[CHASSIS_SUBTYPE_SIZE];
    uint8_t id[CHASSIS_INFO_SIZE];   
};
struct port_id_info{
    char sub_type[PORT_SUBTYPE_SIZE];
    uint8_t id[PORT_INFO_SIZE];   
};
struct management_addr_info{
    char sub_type[IANA_ADDR_SIZE];
    uint8_t addr[MANAGE_ADDR_SIZE];
    char if_type[INTERFACE_SUBTYPE_SIZE];
    uint32_t if_number;
    char oid[OID_STRING_SIZE];    
};
struct lldp_tx_port_statistics {
    uint64_t statsFramesOutTotal; /**< Defined by IEEE 802.1AB Secion 10.5.2.1 */
    uint64_t statsTlvOptionNumner;
};
struct lldp_tx_port_timers {
    uint16_t txTTR;         /**< IEEE 802.1AB 10.5.3 - transmit on expire. */
    uint16_t txShutdownWhile;
    uint16_t txDelayWhile;
};
struct lldp_rx_port_statistics {
    uint64_t statsAgeoutsTotal;
    uint64_t statsFramesDiscardedTotal;
    uint64_t statsFramesInErrorsTotal;
    uint64_t statsFramesInTotal;
    uint64_t statsTLVsDiscardedTotal;
    uint64_t statsTLVsUnrecognizedTotal;
};
struct lldp_port_entry{
    uint8_t link_flag; // cannot tx while state is down
    uint32_t if_index;
    char ifname[IF_NAME_SIZE];     // The interface name.
    uint16_t update_time;
    struct lldp_tx_port_statistics tx_stats; 
    struct lldp_rx_port_statistics rx_stats;
    struct chassis_id_info cha_id;
    struct management_addr_info manage_addr;
    struct port_id_info port_id;
    uint8_t opt_default;
    uint8_t opt_publish;
    char port_desc[PORT_DESC_SIZE];
    char sys_name[SYS_NAME_SIZE];
    char sys_desc[SYS_DESC_SIZE];
    uint16_t sys_cap[2];
    uint8_t dot1_default;
    uint8_t dot1_publish;
    uint16_t pvid;
    uint16_t ppvid;
    uint8_t ppvid_flag;
    uint16_t vid;
    char vname[MAX_VNAME_SIZE];
};
struct lldp_port_config{
    uint32_t if_index;
    uint16_t update_time;
    uint8_t portEnabled;
    uint8_t adminStatus;
    uint16_t reinitDelay;    
    uint16_t msgTxHold;    
    uint16_t msgTxInterval;  
    uint16_t txDelay;            
};
struct lldp_tx_port {
    uint64_t sendsize; /**< The size of our tx frame */
    uint8_t state;     /**< The tx state for this interface */
    uint8_t somethingChangedLocal; /**< IEEE 802.1AB var (from where?) */
    uint16_t txTTL;/**< IEEE 802.1AB var (from where?) */
    struct lldp_tx_port_timers timers; /**< The lldp tx state machine timers for this interface */
};
struct lldp_rx_port_timers {
    uint16_t tooManyNeighborsTimer;
    uint16_t rxTTL;
};
struct lldp_rx_port {
    ssize_t recvsize;
    uint8_t state;
    uint8_t badFrame;
    uint8_t rcvFrame;
    uint8_t rxInfoAge;
    uint16_t rxInfoTTL;
    uint8_t somethingChangedRemote;
    uint8_t tooManyNeighbors;
    struct lldp_rx_port_timers timers;
};
struct eth_hdr {
    char dst[6];
    char src[6];
    uint16_t ethertype;
};
enum portAdminStatus {
    disabled,
    enabledTxOnly,
    enabledRxOnly,
    enabledRxTx,
};
struct lldp_port {
  char ifname[IF_NAME_SIZE];     // The interface name.
  uint32_t if_index; // The interface index.
  uint32_t mtu;      // The interface MTU.
  uint8_t source_mac[6];
  uint8_t source_ipaddr[4];
  struct lldp_rx_port rx;
  struct lldp_tx_port tx;
  uint8_t rxChanges;
  uint16_t pvid;
  uint16_t ppvid;
  uint16_t vid;
  char vname[MAX_VNAME_SIZE];
  uint8_t  auto_neg_status;
  uint16_t auto_neg_advertized_capabilities;
  uint16_t operational_mau_type;
};



struct lldp_port_entry *lldp_local_entry_get(struct lldp_port_entry *local_entry_get);
struct lldp_port_entry *lldp_global_entry_get(struct lldp_port_entry *global_entry_get);
int lldp_config_entry_get(struct lldp_port_config *config_port_get);
int lldp_config_entry_delete(struct lldp_port_config *config_port_ref);
int lldp_config_entry_insert(struct lldp_port_config *config_port_ref);
int lldp_config_entry_update(struct lldp_port_config *config_port_ref);
struct lldp_port_entry *lldp_neighbor_entry_get(struct lldp_port_entry *neighbor_entry_get);
int check_lldp_state();
char *capability_name(uint16_t capability);
char *decode_tlv_system_capabilities(uint16_t system_capabilities, uint16_t enabled_capabilities);
char *tlv_typetoname(uint8_t tlv_type);
char *dot1_typetoname(uint8_t tlv_type);
int *parse_time(int *time,int second);
void translate_state(int state, char *stat);
void translate_admin_state(int admin_state, char *stat);
int lldp_config_update(uint8_t config_type, uint32_t netif_index, int arg);
int lldp_config_enable(struct lldp_port_config *config_entry, uint32_t netif_index);

typedef int (*config_entry_handler)(struct lldp_port_config *config,char *config_str);


#endif

