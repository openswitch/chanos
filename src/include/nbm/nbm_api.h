#ifndef __NBM_API_H__
#define __NBM_API_H__

/* Slot no is the number printed in the front panel, not slot array index used in software*/

extern unsigned char nbm_probe_chassis_module_hw_version
(
	int slotno
);
extern int nbm_probe_chassis_module_id
(
	int slotno
);
extern int nbm_read_backplane_sysinfo
(
	struct product_sysinfo_s *product_info
);
extern void nbm_set_system_init_stage
(
	unsigned char stage
);
int nbm_check_board_state
(
	unsigned char *slot
);
int nbm_get_board_state();




#endif
