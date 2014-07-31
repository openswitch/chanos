#ifndef __NPD_POE_API_H__
#define __NPD_POE_API_H__

#ifdef HAVE_POE
/*NPD LAYER API*/

int npd_get_poeport_by_global_index(unsigned int netif_index, unsigned char *poePort);
void npd_create_poe_intf
(
	unsigned int netif_index
) ;
int npd_poe_init(void);

void* npd_poe_poll_thread(void);


/*NAM LAYER API*/
typedef struct poe_driver_s
{

    int (*poe_port_poe_endis_set)(unsigned char portNum, unsigned char isEnable);
	int (*poe_global_poe_endis_set)(unsigned char isEnable);
	int (*poe_power_manage_set)(unsigned char powerManageMode);
    int (*poe_port_priority_set)(unsigned char portNum, unsigned char portPriority);
	int (*poe_totalpower_and_guradband_set)(unsigned char mpss, int totalPower, int guardBand);
	int (*poe_port_status_get)(unsigned char portNum, unsigned char *portSta, unsigned char *errType, unsigned char *pdClass, unsigned char *pdType);
    int (*poe_total_power_allocate_get)(int *total_power_allocate, int *available_power);
	int (*poe_port_config_get)(unsigned char portNum, unsigned char *isEnable, unsigned char *autoMode, unsigned char *detectType, unsigned char *classType, unsigned char *disconnType, unsigned char *pairType);
	int (*poe_port_extended_config_get)(unsigned char portNum, unsigned char *powerMode, unsigned char *violaType, int *maxPower, unsigned char *portPriority);
	int (*poe_power_manage_config_get)(unsigned char mpss, unsigned char *powerManaMode, int *totalPower, int *guardBand);
	int (*poe_muti_port_status_get)(unsigned char portNum1, unsigned char portNum2, unsigned char portNum3, unsigned char portNum4, unsigned char *status1, unsigned char *status2, unsigned char *status3, unsigned char *status4);
	int (*poe_port_measure_get)(unsigned char portNum, int *voltage, int *poeCurrent, int *temperature, int *power);
}poe_driver_t;
int nam_poe_init(void);
int nam_poe_operate(unsigned char *request_data,unsigned char *response_data);

int nam_poe_power_manage_set(unsigned char powerManageMode);
int nam_poe_totalpower_and_guradband_set(unsigned char mpss, int totalPower, int guardBand);
int nam_poe_global_poe_endis_set(unsigned char isEnable);
int nam_poe_port_status_get(unsigned char portNum, unsigned char *portSta, unsigned char *errType, unsigned char *pdClass, unsigned char *pdType);
int  nam_poe_total_power_allocate_get(int *total_power_allocate, int *available_power);
int nam_poe_port_extended_config_get(unsigned char portNum, unsigned char *powerMode, unsigned char *violaType, int *maxPower, unsigned char *portPriority);
int nam_poe_port_poe_endis_set(unsigned char portNum, unsigned char isEnable);
int nam_poe_maxpower_threshold_set(unsigned char portNum, unsigned int maxPower);
int nam_poe_port_poe_endis_set(unsigned char portNum, unsigned char isEnable);
int nam_poe_port_pair_set(unsigned char portNum, unsigned char pairType);
int nam_poe_power_threshold_set(unsigned char portNum, unsigned char powerThresholdType);
int nam_poe_detect_type_set(unsigned char portNum, unsigned char detectType);
int nam_poe_port_config_get(unsigned char portNum, unsigned char *isEnable, unsigned char *autoMode, unsigned char *detectType, unsigned char *classType, unsigned char *disconnType, unsigned char *pairType);
int nam_poe_port_measure_get(unsigned char portNum, int *voltage, int *poeCurrent, int *temperature, int *power);
int nam_poe_power_manage_config_get(unsigned char mpss, unsigned char *powerManaMode, int *totalPower, int *guardBand);
int nam_poe_port_priority_set(unsigned char portNum, unsigned char portPriority);
int nam_poe_port_err_count_get(unsigned char portNum, unsigned char *absent_count, unsigned char *overload_count, unsigned char *short_count, unsigned char *denied_count, unsigned char *invalid_count);

#endif
#endif


