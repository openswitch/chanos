#ifndef _TEST_ECHO_H_
#define _TEST_ECHO_H_

struct test_item
{
    uint32_t rcv_sequence;
    uint32_t snd_sequence;
	uint32_t server_flag;
};

#define CHASSIS_MAN_SLOT_NUM_FILE        "/proc/sysinfo/slot_num" 
#define CHASSIS_MAN_ENTERPRISE_NAME_FILE    "/devinfo/enterprise_name" 
#define CHASSIS_MAN_PRODUCT_NAME_FILE    "/devinfo/product_name" 
#define CHASSIS_MAN_BOARD_TYPE_FILE    "/proc/sysinfo/board_type" 
#define CHASSIS_MAN_SLOT_NO_FILE            "/var/run/local.slot"
#define CHASSIS_MAN_ACTMASTER_SLOT_FILE     "/var/run/actmaster.slot"
#define CHASSIS_MAN_SBYMASTER_SLOT_FILE     "/var/run/sbymaster.slot"
#define CHASSIS_MAN_ACTMASTER_STATE_FILE    "/var/run/actmaster.state"
#define CHASSIS_MAN_MASTER_STATE_FILE       "/var/run/master.state"
#define CHASSIS_MAN_BOX_STATE_FILE          "/var/run/box.state"
#define CHASSIS_MAN_CTRL_NUM_FILE           "/var/run/ctrl.num"
#define CHASSIS_MAN_CTRL_SWITCH_FILE        "/var/run/ctrl.switch"
#define CHASSIS_MAN_PRE_SLOT_NUM_FILE       "/proc/sysinfo/online_slot_num"
#endif
