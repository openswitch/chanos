#ifndef _CHASSIS_MAN_APP_H_
#define _CHASSIS_MAN_APP_H_

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  !FALSE
#endif


#define CHASSIS_MAN_SLOT_NUM_FILE        "/var/run/slot_num" 
#define CHASSIS_MAN_ENTERPRISE_NAME_FILE    "/devinfo/enterprise_name" 
#define CHASSIS_MAN_PRODUCT_NAME_FILE    "/devinfo/product_name" 
#define CHASSIS_MAN_MODULE_NAME_FILE    "/devinfo/module_name"
#define CHASSIS_MAN_PRODUCT_TYPE_FILE    "/proc/sysinfo/product_type" 
#define CHASSIS_MAN_BOARD_TYPE_FILE    "/proc/sysinfo/board_type" 
#define CHASSIS_MAN_SUPPORT_URL_FILE    "/devinfo/support_url" 
#define CHASSIS_MAN_ENTERPRISE_OID_FILE    "/devinfo/enterprise_snmp_oid" 
#define CHASSIS_MAN_SYSTEM_OID_FILE    "/devinfo/snmp_sys_oid" 
#define CHASSIS_MAN_SLOT_NO_FILE            "/var/run/local.slot"
#define CHASSIS_MAN_ACTMASTER_SLOT_FILE     "/var/run/actmaster.slot"
#define CHASSIS_MAN_SBYMASTER_SLOT_FILE     "/var/run/sbymaster.slot"
#define CHASSIS_MAN_ACTMASTER_STATE_FILE    "/var/run/actmaster.state"
#define CHASSIS_MAN_MASTER_STATE_FILE       "/var/run/master.state"
#define CHASSIS_MAN_BOX_STATE_FILE          "/var/run/box.state"
#define NPD_DBSYNC_DONE_STATE_FILE          "/var/run/dbdone.state.%d"
#define NPD_DBSYNC_ALLDONE_STATE_FILE       "/var/run/dbdone.all"
#define SYS_MODULE_INST_PATH               "/var/run/mod_inst_list.pid"

#define DELIMITER             "--"

#define CHASSIS_MAN_SLAVE_INDPNT_FILE		"/mnt/slave.indpnt"
#define CHASSIS_MAN_SLAVE_RUNNING_INDPNT_FILE "/mnt/slave.indpnt.run"
#define CHASSIS_MAN_SLAVEINDPNT_TEST_FILE	"/mnt/slave_indpnt.test"

#define ZEBRA_RUNNING_STATE "/var/run/zebra.state"
#define PROPERTY_MAX_LEN 64

int write_to_file(char * filename, char * buffer, int len);
int app_slot_num_get();
int app_enterprise_name_get(char *enterprise);
int app_product_name_get(char *pname);
int app_module_name_get(char *pname);
int app_product_type_get();
int app_board_type_get();
int app_support_url_get(char *pname);
int app_enterprise_snmp_oid_get(char *pname);
int app_snmp_system_oid_get(char *pname);
int app_act_master_running();;
int app_local_slot_get();
int app_actmaster_slot_get();
int app_sbymaster_slot_get();
int app_slot_work_mode_get();
int app_slot_slave_indp_get();
int app_box_state_get();
int app_slave_indpnt_runget();
int app_slave_indpnt_get();
int app_slaveindpnt_test_get();
int app_zebra_state_get();
int app_npd_sync_alldone_state_get();

int app_npd_sync_done_state_get(int slot_index);

int app_module_inst_init();
int app_module_inst_get(char *, unsigned int *);
int app_module_inst_set(char *, unsigned int);
int app_module_inst_check(char *);

#endif
