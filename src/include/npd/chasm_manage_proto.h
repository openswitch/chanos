#ifndef __CHASM_MANAGE_PROTO_H__
#define __CHASM_MANAGE_PROTO_H__


enum
{
    CHASM_PDU_TYPE_REG_REQ,
    CHASM_PDU_TYPE_REG_RES,
    CHASM_PDU_TYPE_STATUS,
    CHASM_PDU_TYPE_TIME,
    CHASM_PDU_TYPE_QUERY,
    CHASM_PDU_TYPE_CMD,
    CHASM_PDU_TYPE_SWITCHOVER,
    CHASM_PDU_TYPE_RESET,
    CHASM_PDU_TYPE_AMS_REG,
    CHASM_SUBBOARD_HWREMOVE,
    CHASM_PDU_TYPE_MAX
};

typedef struct chasm_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned char body[0];
}chasm_tlv_t;

typedef struct chasm_pdu_head_s
{
    unsigned long  version;
    unsigned long  length;
    struct chasm_tlv_s tlv[0];
}chasm_pdu_head_t;

typedef struct chasm_system_fixinfo_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short product_series;
    unsigned short product_type;
    unsigned long product_short_name[16];
    unsigned long product_full_name[32];
    unsigned char sn[32];
    unsigned char manufacture_date[16];
    unsigned char os_name[32];
    unsigned char sys_version[32];
    unsigned char base_mac[6];
    unsigned char reserved0[2];
}chasm_system_fixinfo_tlv_t;

typedef struct chasm_board_reginfo_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short chassis_id;
    unsigned short slotid;
    unsigned short subslot_id;
    unsigned short board_type;
    unsigned char  board_short_name[16];
    unsigned char  board_full_name[32];
    unsigned char  sn[32];
    unsigned char  manufacture_date[16];
    unsigned char  hw_version[32];
    unsigned char  sw_version[32];
    unsigned char  bootrom_ver[32];
}chasm_board_reginfo_tlv_t;

typedef struct  chasm_board_statusinfo_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short chassis_id;
    unsigned short slotid;
    unsigned short subslot_id;
    unsigned short board_type;
    unsigned short runstate;
	unsigned short online_removed;
    unsigned char active_flags;
    unsigned char workmode;
}chasm_board_statusinfo_tlv_t;

typedef struct chasm_product_info_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short product_type;
    unsigned char  base_mac[6];
    unsigned char  sw_version[28];
    unsigned int build_time;
}chasm_product_info_tlv_t;

typedef struct chasm_board_regres_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short product_type;
    unsigned char  base_mac[6];
    unsigned char  sw_version[28];
    unsigned int build_time;
}chasm_board_regres_tlv_t;

typedef struct chasm_ams_reginfo_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short ams_type;
    unsigned short vender_id;
    unsigned short revid;
    unsigned short pci_id;
    unsigned char  name[32];
}chasm_ams_reginfo_tlv_t;

typedef struct chasm_switchover_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short slot_id;
    unsigned short pre_act_master;
    unsigned short reason;
    unsigned short runstate;
}chasm_switchover_tlv_t;

typedef enum chasm_board_cmd_type_e
{
    CHASM_CMD_DISABLE,
    CHASM_CMD_ENABLE,
    CHASM_CMD_SET_TIME,
    CHASM_CMD_OIR_PERIOD,
    CHASM_CMD_REBOOT,
    CHASM_CMD_OIR,
    
    CHASM_PDU_CMD_MAX
} chasm_board_cmd_type_t;

typedef struct chasm_board_cmd_tlv_s
{
    unsigned short type;
    unsigned short length;
    unsigned short command;
}chasm_board_cmd_tlv_t;

#define CHASM_PROTOCOL_VERSION              1

#define CHASM_PACKET_MAX_LENGTH             1500

#define CHASM_BROADCAST_SLOTID              -1

#define SERVER_CHASM_MANAGE                  0x8000
#define SERVER_MCU_EXIST                     0x8001
#define MAX_CONNECT_CHASM_SLAVE_TIME        3
#define CHASM_CONNECT_TIMEOUT                300
#define CHASM_REG_TIMEOUT                    300
#define CHASM_SWVERERR_TIMEOUT               300
#define CHASM_BOARD_ALLREADY_TIMEOUT         30
/*the time for restore configure file will be very long*/
#define CHASM_WAIT_CONNECT_TIMEOUT           1200
#define CHASM_SBY_CONNECT_TIMEOUT             30
#define CHASM_MCU_EXIST_TIMEOUT               1
#define CHASM_DISCOVERING_TIMEOUT            120
#define CHASM_MANTEST_TIMEOUT                10
#define CHASM_DBSYNC_TIMEOUT                 10
#define NPD_DBSYNC_WAIT_MAX_TIMES            12
#define CHASM_SWITCHOVER_TIMEOUT             3

#define CHASM_SLAVE_INDPNT_WAIT_TIMEOUT      10	

#define CHASM_DEFAULT_BUILD_TIME             1
extern state_desc_t rmt_board_state_desc[];
extern state_desc_t local_slave_state_desc[];
extern state_desc_t local_master_state_desc[];

extern int npd_chassis_manage(void);
int npd_dbtable_sync_startup();

unsigned int chasm_local_check(unsigned int slot_no);

long chasm_board_ready_config(unsigned int slot_index);

long  chasm_os_upgrade(unsigned int slot_index);
long chasm_board_cancel_timeout(int slot_index);


#endif
