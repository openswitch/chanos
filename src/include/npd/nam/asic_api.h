#ifndef __NPD_ASIC_API_H__
#define __NPD_ASIC_API_H__

/*NPD LAYER API*/
int npd_fw_engines_init(void);
int npd_fw_engine_initialization_check(void);



/*NAM LAYER API*/

#define NAM_DEBUG(expr)   npd_syslog_asic_dbg expr

#define NAM_SDK_CALL(ret, func) do { \
                           nam_api_lock(); \
                           ret = func; \
                           nam_api_unlock(); \
                           }while(0)
#define NAM_LABEL_START   128


extern unsigned long nam_asic_get_instance_num(void);

extern unsigned int nam_board_after_enable();

extern int nam_asic_init_completion_check(void) ;
extern int  nam_dhcp_packet_trap_cpu_global(int enable);
extern int nam_arp_packet_trap_cpu_global(int enable);
extern int nam_bpdu_packet_trap_cpu_global(int enable);
extern int nam_8021x_packet_trap_cpu_global(int enable);
extern int nam_nd_packet_trap_cpu_global(int enable);
extern int nam_igmp_packet_trap_cpu_global(int enable);
extern int nam_asic_info_get(int unit, struct ams_info_s *info);
extern int nam_asic_log_level_set(	int level);
extern int nam_asic_log_level_clr(int level);
extern int nam_asic_init(int unit);
extern int nam_api_lock();
extern int nam_api_unlock();
#endif

