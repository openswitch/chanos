#ifndef __NPD_PRODUCT_API_H__
#define __NPD_PRODUCT_API_H__

int npd_init_productinfo(void);
extern void product_init(void);
unsigned int npd_query_product_id(void);
extern int register_product_param_array(product_fix_param_t **array);
extern int register_board_param_array(board_fix_param_t **array);
extern int register_product_conn_array(product_conn_type_t **array);
extern int register_board_conn_array(board_conn_type_t **array);
extern int register_board_conn_fullmesh_array(board_conn_type_t **array);
extern int register_board_spec_param_array(board_spec_fix_param_t **array);
extern void device_product_reset(unsigned long product_type);
extern unsigned char* device_product_type2name( unsigned long product_type );
void device_product_reset_admin(void);
unsigned long device_submoduletype_get( unsigned long hwcode );
int init_conn_info(unsigned long board_type, unsigned long product_type);
void device_product_reset_sn(char* serial_no);
unsigned long device_producttype_get( void );
unsigned long device_moduletype_get();

int npd_system_verify_basemac
(
	char *macAddr
);

int npd_system_get_basemac
(
    unsigned char *macAddr,
    unsigned int  size
);


void npd_init_oam
(
	void
);

void npd_check_system_startup_state
(
	void
);

void npd_check_system_attack
(
	void
);

void npd_init_packet_socket
( 
	void
);


void npd_init_tell_whoami
(
	char *tName,
	unsigned char lastTeller
);

void npd_init_tell_stage_end
(
	void
);


#endif

