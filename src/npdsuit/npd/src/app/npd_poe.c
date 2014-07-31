/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_poe.c
*
*
* CREATOR:
*		wuhao@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD by POE module.
*
* DATE:
*		02/02/2012	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.256 $	
*******************************************************************************/
#ifdef HAVE_POE
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"

#include "npd_poe.h" 

#define POE_SHOW_TIME_SIZE (128 * 1024)
#define POE_ABS_TIME             (1<<0)
#define POE_TIME_WEEKEND         (1<<1)
#define POE_TIME_WORKDAY         (1<<2)
#define POE_TIME_EVERYDAY        (1<<3)

#define IN_TIME_RANGE             1
#define OUT_TIME_RANGE            0

extern int npd_startup_end ;


db_table_t *poe_pse_db = NULL;
array_table_index_t *poe_pse_index = NULL;

db_table_t *poe_intfs_db = NULL;
array_table_index_t *poe_intfs = NULL;
hash_table_index_t *poe_intfs_hash = NULL;

/*poe global configuration*/
db_table_t         *npd_poe_cfgtbl = NULL;
array_table_index_t *npd_poe_cfg_index = NULL;
unsigned int poe_cfg_global_no = 0;


db_table_t         *npd_poe_tr_cfgtbl = NULL;
array_table_index_t *npd_poe_time_range_index = NULL;

unsigned int poe_intf_hash_key(void *data)
{
    poe_intf_db_t *poe_intf = (poe_intf_db_t*)data;
	unsigned int netif_index = poe_intf->netif_index;
	int hash_index = 0;
	hash_index += ((netif_index & 0xFFFFFFF) >> 14);
    return hash_index%MAX_ETH_GLOBAL_INDEX;
}

unsigned int poe_intf_hash_cmp(void *data1, void *data2)
{
    poe_intf_db_t *poe_intf1 = (poe_intf_db_t*)data1;
    poe_intf_db_t *poe_intf2 = (poe_intf_db_t*)data2;

    return (poe_intf1->netif_index 
                  == poe_intf2->netif_index);
}


long poe_intf_handle_delete(void *newdata);
long poe_intf_handle_update(void * newdata, void *olddata);
long poe_intf_handle_insert(void *newdata);

long pse_handle_delete(void *newdata);
long pse_handle_update(void * newdata, void *olddata);
long pse_handle_insert(void *newdata);


#define POEPORTS_BMP_IS_PORT_SET(portsBmp,portNum)   \
    (((portsBmp)->poe_ports[(portNum)>>5] & (1 << ((portNum)& 0x1f)))? 1 : 0)

int npd_poe_port_check(int slot, int sub_slot, int port)
{
	poe_module_fix_param_t* poe_param = MODULE_POEPORTS_ON_SUBSLOT_INDEX(slot, sub_slot);
	if (NULL == poe_param)
		return 0;
	return POEPORTS_BMP_IS_PORT_SET(poe_param, port);
}

void npd_syslog_poe_dbg(unsigned char dbg_flag, unsigned char last_status, poe_intf_db_t poe_dbg_intf)
{
	
	char *poe_operate_status[8] = {
		"OFF",
		"ON",
		"MPS-ABSENT",
		"SHORT",
		"OVERLOAD",
		"POWER-DENIED",
		"TERMINAL-SHUTDOWN",
		"STARTUP-FAILURE"
	};
	char *poe_detect_status[7] = {
		"DISABLE",
		"SEARCHING",
		"POWERING",
		"TEST",
		"FAULT",
		"POE_DETECT_OTHER_FAULT",
		"POE_DETECTT_REQUEST_POWER"
	};
	
	char port_string[10] = {0};
	float poe_intf_temperature = 0;
	float poe_intf_current = 0;
	float poe_intf_voltage = 0;
	float poe_intf_power = 0;
	unsigned char absent_count = 0;
	unsigned char overload_count = 0;
	unsigned char short_count = 0;
	unsigned char power_denied_count = 0;
	unsigned char invalid_signal_count = 0;
	unsigned char poe_port = 0;
	int ret = 0;

	if (dbg_flag == POE_LOG_NONE)
		return;
	

	ret = npd_get_poeport_by_global_index(poe_dbg_intf.netif_index, &poe_port);
	if(NPD_SUCCESS != ret )
    {
		return;
	}
	nam_poe_port_err_count_get(poe_port, &absent_count, 
		&overload_count, &short_count, 
		&power_denied_count, &invalid_signal_count);
	
	poe_intf_temperature = poe_dbg_intf.temperature * 0.01;
	poe_intf_current = poe_dbg_intf.current;
	poe_intf_voltage = poe_dbg_intf.voltage * 0.01*0.001;
	poe_intf_power = poe_dbg_intf.power_user * 0.1;
	
    parse_eth_index_to_name(poe_dbg_intf.netif_index, (unsigned char*)port_string);
	npd_syslog_official_event("----------------POE PORT INFO-----------------\n");
	npd_syslog_official_event("Port                 : %s\n", port_string);
	npd_syslog_official_event("last status          : %s\n", poe_detect_status[last_status]);
	npd_syslog_official_event("current status       : %s\n", poe_detect_status[poe_dbg_intf.detect_status]);



	if (POE_LOG_ERR_FLAG & dbg_flag)
	{
		if ((POE_DETECT_FAULT == poe_dbg_intf.detect_status) 
			|| (POE_DETECT_OTHER_FAULT == poe_dbg_intf.detect_status))
		{
			npd_syslog_official_event("error type           : %s\n", poe_operate_status[poe_dbg_intf.operate_status]);
		}
		else
		{
			npd_syslog_official_event("error type           : NONE\n");
		}
	}
	if (POE_LOG_TEMP_FLAG & dbg_flag)
	{
		npd_syslog_official_event("temperature          : %.1fC\n", poe_intf_temperature);
	}
	if (POE_LOG_CLASS_FLAG & dbg_flag)
	{
		npd_syslog_official_event("pd class             : %d\n", poe_dbg_intf.pd_class);

	}
	if (POE_LOG_CURRENT_FLAG & dbg_flag)
	{
		npd_syslog_official_event("current              : %.1fmA\n", poe_intf_current);
	}
	if (POE_LOG_VOL_FLAG & dbg_flag)
	{
		npd_syslog_official_event("voltage              : %.1fV\n", poe_intf_voltage);
	}
	if (POE_LOG_POWER_FLAG & dbg_flag)
	{
		npd_syslog_official_event("power                : %.1fW\n", poe_intf_power);

	}
	if (POE_LOG_POWER_FLAG & dbg_flag)
	{
		npd_syslog_official_event("MPS absent count     : %d\n", absent_count );

	}
	if (POE_LOG_POWER_FLAG & dbg_flag)
	{
		npd_syslog_official_event("overload count       : %d\n", overload_count );

	}
	//npd_syslog_official_event("%s", poe_dbg_info);
	if (POE_LOG_POWER_FLAG & dbg_flag)
	{
		npd_syslog_official_event("short count          : %d\n", short_count );

	}
	if (POE_LOG_POWER_FLAG & dbg_flag)
	{
		npd_syslog_official_event("power deny count     : %d\n", power_denied_count );

	}
	if (POE_LOG_POWER_FLAG & dbg_flag)
	{
		npd_syslog_official_event("invalid signal count : %d\n", invalid_signal_count );

	}
	npd_syslog_official_event("----------------------------------------------\n");
	
	
}

void npd_poe_notify_event(
    unsigned int netif_index,
    enum PORT_NOTIFIER_ENT event
    )
{
	int type = 0;
	int ret = 0;
	
	unsigned char poePort = 0;
	unsigned char portSta = 0;
	unsigned char errType = 0;
	unsigned char pdClass = 0;
	unsigned char pdType = 0;
	int total_power_allocate = 0;
	int available_power = 0;
	unsigned char powerMode = 0;
	unsigned char violaType = 0;
	int maxPower = 0;
	unsigned char portPriority = 0;
	int pse_total_power = 0;
	poe_intf_db_t poe_intf = {0};
	poe_intf_db_t poe_dbg_intf = {0};
	poe_port_t poe_port = {0};
	int  portStatus = 0;
	unsigned int portSpeed = 0;
	int voltage = 0;
	int poeCurrent = 0;
	int temperature = 0;
	int power = 0;
	float poe_intf_temperature = 0.0;
	type = npd_netif_type_get(netif_index);

	npd_poe_cfg_t npd_poe_cfg_get;
	

	if(type != NPD_NETIF_ETH_TYPE)
	{
	 return ;
	}
	ret = npd_get_poeport_by_global_index(netif_index, &poePort);
	if(NPD_SUCCESS != ret )
    {
		return;
	}
	poe_intf.netif_index = netif_index;
	ret = dbtable_hash_search(poe_intfs_hash, &poe_intf, NULL, &poe_intf);
	if(NPD_SUCCESS != ret )
	{
		return;
	}
	nam_poe_port_status_get(poePort, &portSta, &errType, &pdClass, &pdType);
	
	switch(event)
	{	
		case NOTIFIER_POE_OPERATE:
		{
        	nam_poe_total_power_allocate_get(&total_power_allocate, &available_power);
        	nam_poe_port_extended_config_get(poePort, &powerMode, &violaType, &maxPower, &portPriority);
        	nam_poe_port_measure_get(poePort, &voltage, &poeCurrent, &temperature, &power);
        	pse_total_power = MODULE_PSE_TOTAL_POWER_ON_SLOT_INDEX(0);
        	poe_port.port = poePort;
        	
        	char port_string[10] = {0};
            parse_eth_index_to_name(poe_intf.netif_index, (unsigned char*)port_string);
           
        	poe_intf_temperature = temperature * 0.01;
        
        	poe_dbg_intf.netif_index = netif_index;
        	poe_dbg_intf.detect_status = portSta;
        	poe_dbg_intf.operate_status = errType;
        	poe_dbg_intf.temperature = temperature;
        	poe_dbg_intf.pd_class = pdClass;
        	poe_dbg_intf.current = poeCurrent;
        	poe_dbg_intf.voltage = voltage;
        	poe_dbg_intf.power_user = power;
        		
			if (poe_intf.detect_status != portSta)
			{
				npd_syslog_poe_dbg(POE_LOG_NONE, poe_intf.detect_status, poe_dbg_intf);
				
				if (portSta == POE_DETECT_DELIVERING_POWER)
				{
					if (poe_intf.priority == POE_PRIORITY_NONE)
						poe_intf.priority = POE_PRIORITY_LOW;
					if ((poe_intf.power_threshold_type == POE_THRESHOLD_USER_DEFINE) && (poe_intf.max_power_admin_flag == 0))
					{
						switch (pdClass)
						{
							case 0:
								poe_intf.max_power = CLASS0_POWER;
								break;
							case 1:
								poe_intf.max_power = CLASS1_POWER;
								break;
							case 2:
								poe_intf.max_power = CLASS2_POWER;
								break;
							case 3:
								poe_intf.max_power = CLASS3_POWER;
								break;
							case 4:
								poe_intf.max_power = CLASS4_POWER;
								break;
							default:
								poe_intf.max_power = CLASS4_POWER;
								break;

						}
	
						ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
						if (0 != ret)
							return;
						/*剩余功率不足以支持端口功率 提升到 指定等级功率*/
						if ((poe_intf.max_power > maxPower) && ((poe_intf.max_power - maxPower)  > (pse_total_power - total_power_allocate)))
						{                                          
							nam_poe_port_poe_endis_set(poePort, 0);
							nam_poe_maxpower_threshold_set(poePort, poe_intf.max_power);
							nam_poe_port_poe_endis_set(poePort, 1);
						}
					
					//printf("poePort %d, power_threshold_type %d, pdClass %d, maxPower %d.\n", poePort, poe_intf.power_threshold_type, pdClass, poe_intf.max_power);
					}	
				}
				else
				{
					if (poe_intf.priority == POE_PRIORITY_LOW)
						poe_intf.priority = POE_PRIORITY_NONE;
				}
				switch (portSta)
				{
					case POE_DETECT_DISABLE:
						poe_intf.operate_status = POE_POWER_OFF;
						break;
					case POE_DETECT_DELIVERING_POWER:
						poe_intf.operate_status = POE_POWER_ON;
						break;
					case POE_DETECT_FAULT:
						poe_intf.operate_status = errType + 1;
						break;
					case POE_DETECT_OTHER_FAULT:
						poe_intf.operate_status = errType + 1;
						break;
					default:
						poe_intf.operate_status = POE_POWER_OFF;
						break;	
				}
				ret = npd_get_port_link_status(netif_index, &portStatus);

        		if (NPD_SUCCESS == ret)
        		{
               	 	if (portStatus == ETH_ATTR_ON)
                	{
                		ret = nam_get_port_speed(netif_index, &portSpeed);
						if (0 == ret)
						{
							if (portSpeed >= PORT_SPEED_1000_E)
							{
								poe_port.speed = POE_GE;
							}
							else
							{
								poe_port.speed = POE_100M;
							}
							if (portSta == POE_DETECT_DELIVERING_POWER)
							{
								poe_port.state = POE_NORMAL;
							}
							else if ((portSta == POE_DETECT_FAULT) || (portSta == POE_DETECT_OTHER_FAULT))
							{
								poe_port.state = POE_ALARM;
							}
							else
							{
								poe_port.state = POE_OFF;
							}
						}
                	}
                	else
                	{
                		poe_port.port = poePort;
						poe_port.speed = POE_LINK_DOWN;
						poe_port.state = POE_LINKDOWN;
                	}
					ret = nbm_poe_led(&poe_port);
					//printf("poe notiy, set poe led, port %d, spped %d, state %d, ret %d.\n", poe_port.port, poe_port.speed, poe_port.state, ret);
					if (ret != 0)
					{
						npd_syslog_dbg("write register error!.\n");
					}
       	 		}
				else
				{
					npd_syslog_dbg("Get port link status failed.\n");
				}
				poe_intf.detect_status = portSta;
				dbtable_hash_update(poe_intfs_hash, &poe_intf, &poe_intf);
			}
			break;
		}
		case PORT_NOTIFIER_LINKUP_E:
		{
			if (!npd_startup_end)
			{
				//printf("startup_end not over.\n");
				return ;
			}
			//printf("get port speed.\n");
			ret = nam_get_port_speed(netif_index, &portSpeed);
			if (0 == ret)
			{
				if (portSpeed >= PORT_SPEED_1000_E)
				{
					poe_port.speed = POE_GE;
				}
				else
				{
					poe_port.speed = POE_100M;
				}
				if (portSta == POE_DETECT_DELIVERING_POWER)
				{
					poe_port.state = POE_NORMAL;
				}
				else if ((portSta == POE_DETECT_FAULT) || (portSta == POE_DETECT_OTHER_FAULT))
				{
					poe_port.state = POE_ALARM;
				}
				else
				{
					poe_port.state = POE_OFF;
				}
				poe_port.port = poePort;
				ret = nbm_poe_led(&poe_port);
				//printf("link up set poe led, port %d, spped %d, state %d, ret %d.\n", poe_port.port, poe_port.speed, poe_port.state, ret);
				if (ret != 0)
				{
					npd_syslog_dbg("write register error!.\n");
				}
			}
			break;
		}
		case PORT_NOTIFIER_LINKDOWN_E:
		{
			if (!npd_startup_end)
			{
				//printf("startup_end not over.\n");
				return ;
			}
			//printf("get port speed.\n");
			
			poe_port.speed = POE_LINK_DOWN;
			poe_port.state = POE_LINKDOWN;
			poe_port.port = poePort;
			ret = nbm_poe_led(&poe_port);
			//printf("link down set poe led, port %d, spped %d, state %d, ret %d.\n", poe_port.port, poe_port.speed, poe_port.state, ret);
			if (ret != 0)
			{
				npd_syslog_dbg("write register error!.\n");
			}
			break;
		}
		default:
			break;
	}
	return;
}

void npd_poe_relate_event(
    unsigned int father_netif_index,
    unsigned int netif_index,
    enum PORT_RELATE_ENT event
    )
{
	return;
}

netif_event_notifier_t poe_netif_notifier =
{
    .netif_event_handle_f = &npd_poe_notify_event,
    .netif_relate_handle_f = &npd_poe_relate_event
};


int npd_get_poeport_by_global_index(unsigned int netif_index, unsigned char *poePort)
{
	int ret = 0;
	int poe_port = 0;
	unsigned int local_port_no = 0, port_index = 0;
	unsigned int  slot_index = 0, slot_no = 0;

	port_index 		= ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(netif_index);
	slot_no			= CHASSIS_SLOT_INDEX2NO(slot_index);
	slot_index 		= SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(netif_index);

	local_port_no	= ETH_LOCAL_INDEX2NO(slot_index, port_index);

	ret = eth_port_legal_check(netif_index);
    if(0 != ret)
        return -1;
	if (!CHASSIS_SLOTNO_ISLEGAL(slot_no)) 
		return -1;
	if (!ETH_LOCAL_PORTNO_ISLEGAL(slot_no,local_port_no)) 
		ret = -1;
	poe_port = PANEL_2_POE_PORT(local_port_no);
	//printf("netif_index 0x%x, local_port_no %d, poe_port %d.\n", netif_index, local_port_no, poe_port);
	if (-1 == poe_port)
		return -1;
	*poePort = (unsigned char)poe_port;
	return 0;
}

int npd_poe_on_time(unsigned int index, unsigned int *op_ret)
{
    unsigned int ret = 0;
    time_t now;
    struct tm *timenow = NULL;
    int standard = 0;  
    int start = 0;     
    int end = 0;      
    int standard_day = 0;
    int start_day = 0;
    int end_day = 0;
    struct poe_time_range_info_s poe_time;

    memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));

    ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time);

    time(&now);
    timenow = localtime(&now);

//    printf("TIME_POE: %d:%d ->%d:%d\n",
//        poe_time.periodic_time.start.hh, poe_time.periodic_time.start.mm,
//        poe_time.periodic_time.end.hh, poe_time.periodic_time.end.mm);
//    printf("TIME_NOW: %d:%d\n", timenow->tm_hour, timenow->tm_min);
    if(POE_ABS_TIME != poe_time.abs_time.flag )
    {
        standard = (timenow->tm_hour)*60 + timenow->tm_min;
        start = (poe_time.periodic_time.start.hh)*60  
                + poe_time.periodic_time.start.mm;
        end = (poe_time.periodic_time.end.hh)*60
              + poe_time.periodic_time.end.mm;
        if(POE_TIME_EVERYDAY == poe_time.periodic_time.flag)
        {
            if(standard>=start && standard<=end)
                *op_ret = IN_TIME_RANGE;
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(POE_TIME_WORKDAY == poe_time.periodic_time.flag)
        {
            if(timenow->tm_wday <= 5 && timenow->tm_wday >= 1)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(POE_TIME_WEEKEND == poe_time.periodic_time.flag) 
        {
            if(timenow->tm_wday == 6 || timenow->tm_wday == 0)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else
        {
            syslog_ax_acl_err("Unknow Error!\n");
        }
    }
    else 
    {
        standard_day = (timenow->tm_year+1900)*372 + (timenow->tm_mon)*31 + timenow->tm_mday;
        start_day = (poe_time.abs_time.start.year)*372
                    + (poe_time.abs_time.start.month-1)*31
                    + poe_time.abs_time.start.day;
        end_day = (poe_time.abs_time.end.year)*372
                    + (poe_time.abs_time.end.month-1)*31
                    + poe_time.abs_time.end.day;

        if(standard_day < start_day || standard_day > end_day)
        {
            *op_ret = OUT_TIME_RANGE;
            return POE_RETURN_CODE_ERR_NONE;
        }

        {
            int abs_time_start = 0;
            int abs_time_end = 0;
            int current_time = 0;

            current_time = (timenow->tm_hour)*60 + timenow->tm_min;
            abs_time_start = (poe_time.abs_time.start.hh)*60  
                    + poe_time.abs_time.start.mm;
            abs_time_end = (poe_time.abs_time.end.hh)*60
                  + poe_time.abs_time.end.mm;
            
            if(standard_day == end_day)
            {
                if(current_time > abs_time_end)
                {   
                    *op_ret = OUT_TIME_RANGE;
                    return POE_RETURN_CODE_ERR_NONE;
                }
            }
            if(standard_day == start_day)
            {
                if(current_time < abs_time_start)
                {   
                    *op_ret = OUT_TIME_RANGE;
                    return POE_RETURN_CODE_ERR_NONE;
                }
            }
        }
        
        standard = (timenow->tm_hour)*60 + timenow->tm_min;
        start = (poe_time.periodic_time.start.hh)*60  
                + poe_time.periodic_time.start.mm;
        end = (poe_time.periodic_time.end.hh)*60
              + poe_time.periodic_time.end.mm;

        

        if(POE_TIME_EVERYDAY == poe_time.periodic_time.flag)
        {
            if(standard>=start && standard<=end)
                *op_ret = IN_TIME_RANGE;
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(POE_TIME_WORKDAY == poe_time.periodic_time.flag)
        {
            if(timenow->tm_wday <= 5 && timenow->tm_wday >= 1)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else if(POE_TIME_WEEKEND == poe_time.periodic_time.flag) 
        {
            if(timenow->tm_wday == 6 || timenow->tm_wday == 0)
            {
                if(standard>=start && standard<=end)
                    *op_ret = IN_TIME_RANGE;
                else
                    *op_ret = OUT_TIME_RANGE;
            }
            else
                *op_ret = OUT_TIME_RANGE;
        }
        else
        {
            *op_ret = IN_TIME_RANGE;
        }
    }
    
    return POE_RETURN_CODE_ERR_NONE;
}
void npd_poe_timer(void)
{
    unsigned int ret = 0;
    unsigned int op_ret = 0;
    int i = 0;
    int totalcount = 0;
    npd_poe_cfg_t npd_poe_cfg_get;
    struct poe_intf_db_s poe;

    if(SYS_LOCAL_MODULE_WORKMODE_ISSLAVE)
        return ;
    npd_init_tell_whoami("SysPoeBasedTime",0);
    
    while(1)
    {

        if(SYS_LOCAL_MODULE_ISMASTERACTIVE)
        {
            ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
            if(0 != npd_poe_cfg_get.poe_enable)
            {
                totalcount = dbtable_array_totalcount(poe_intfs);
                for(i = 1; i < totalcount; i++)
                {
                    ret = dbtable_array_get(poe_intfs, i, &poe);
                    if(0 != ret)
                        continue;
                    if(0 == poe.time.poe_based_time_valid)
                        continue;
                    
                    ret = npd_poe_on_time(poe.time.time_range_index, &op_ret);
                    if(MATCH_RETURN_CODE_SUCCESS != ret)
                        continue;

//                    printf("timer op_ret = %d, poe.time.is_poe_deployed = %d\n", op_ret, poe.time.is_poe_deployed);
                    if(((poe.time.is_poe_deployed == 0) && (op_ret == 1))
                        || ((poe.time.is_poe_deployed == 1) &&(op_ret == 0)))
                    {
                        
                        poe.admin = op_ret;
                        poe.time.is_poe_deployed = op_ret;
                        ret = dbtable_hash_update(poe_intfs_hash, &poe, &poe);
                    }
                }
            }
        }
        sleep(10);
    }
}
/*
  Network Platform Daemon Ethernet Port Management
*/
int npd_poe_init(void)
{
    char name[16];
	int ret;
	/*PSE 默认值需要根据实际值来修改，现在默认为0*/
	struct poe_pse_db_s poe_pse_default = {0};
	/*初始PSE*/
	int pse_total_power = MODULE_PSE_TOTAL_POWER_ON_SLOT_INDEX(SYS_LOCAL_MODULE_SLOT_INDEX);
	int pse_guard_band = MODULE_PSE_GUARD_BAND_ON_SLOT_INDEX(SYS_LOCAL_MODULE_SLOT_INDEX);
	unsigned int pse_type = MODULE_PSE_TYPE_ON_SLOT_INDEX(SYS_LOCAL_MODULE_SLOT_INDEX);
	poe_pse_default.admin = 1; /* default is enable */
	poe_pse_default.available = 0;
	poe_pse_default.current = 0;
	poe_pse_default.max_power = pse_total_power;
	poe_pse_default.mode = POE_PSE_AUTO_AND_DYNAMIC;
	poe_pse_default.pse_id = 0;/*现在盒式设备只有一个PSE，移植到机架需要修改*/
	poe_pse_default.pse_type = pse_type;
	poe_pse_default.guardBand = pse_guard_band;
	poe_pse_default.slot_no = 0;
	poe_pse_default.voltage = 0;
	
	unsigned int pse_index = 0;
	struct npd_poe_cfg_s npd_poe_cfg_default;
	
	npd_syslog_dbg("nam_poe_init \n");
	nam_poe_init();

    strcpy(name, "POE_PSE_DB");
	ret = create_dbtable( name, 1, sizeof(struct poe_pse_db_s),\
						&pse_handle_update, NULL, &pse_handle_insert, 
						&pse_handle_delete, NULL, NULL, NULL, 
						NULL, NULL, DB_SYNC_ALL, &poe_pse_db);
    if( 0 != ret )
	{
		syslog_ax_fdb_err("create npd poe pse table fail\n");
		return NPD_FAIL;
	}

	ret = dbtable_create_array_index("poe_pse_index", poe_pse_db, &poe_pse_index);
    if( 0 != ret )
	{
		syslog_ax_fdb_err("create npd poe pse table index fail\n");
		return NPD_FAIL;
	}
	
	ret = dbtable_array_insert(poe_pse_index, &pse_index, &poe_pse_default);
	if(ret != 0)
	{
		syslog_ax_fdb_err("Insert npd poe pse default configuration failed.\n");
		return NPD_FAIL;
	}	
    strcpy(name, "POE_INTFS_DB");
    create_dbtable(name, MAX_ETH_GLOBAL_INDEX, sizeof(struct poe_intf_db_s),
        &poe_intf_handle_update, NULL, &poe_intf_handle_insert, 
        &poe_intf_handle_delete, NULL, NULL, NULL, 
        NULL, NULL, DB_SYNC_ALL, &poe_intfs_db);

    dbtable_create_array_index("poe_array_index", poe_intfs_db, &poe_intfs);
	if(NULL == poe_intfs)
	{
		syslog_ax_eth_port_dbg("memory alloc error for eth port init!!!\n");
		return NPD_FAIL;
	}

    dbtable_create_hash_index("poe_netif_index", poe_intfs_db, MAX_ETH_GLOBAL_INDEX, 
          &poe_intf_hash_key, &poe_intf_hash_cmp,
          &poe_intfs_hash);

    /* register_netif_notifier(&switchport_netif_notifier); */
	/*create poe global configuration and insert default configuration*/
	ret = create_dbtable( NPD_POE_CFGTBL_NAME, 1, sizeof(struct npd_poe_cfg_s),\
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL, 
					NULL,
					NULL, NULL, NULL,
					DB_SYNC_ALL,
					&npd_poe_cfgtbl);
    if( 0 != ret )
	{
		syslog_ax_fdb_err("Create npd poe configuration table fail\n");
		return NPD_FAIL;
	}
	ret = dbtable_create_array_index("poe_cfg", npd_poe_cfgtbl, &npd_poe_cfg_index);
    if( 0 != ret )
	{
		syslog_ax_fdb_err("Create npd POE configuration table index fail\n");
		return NPD_FAIL;
	}

    ret = create_dbtable( "npdPoeTRcfgtbl", 256, sizeof(struct poe_time_range_info_s),\
					NULL, 
					NULL,
					NULL,
					NULL,
					NULL, 
					NULL,
					NULL, NULL, NULL,
					DB_SYNC_ALL,
					&npd_poe_tr_cfgtbl);

	ret = dbtable_create_array_index("poe_cfg", npd_poe_tr_cfgtbl, &npd_poe_time_range_index);
	/*init global_poe enable*/
	npd_poe_cfg_default.poe_enable = 1;
	ret = dbtable_array_insert(npd_poe_cfg_index, &poe_cfg_global_no, &npd_poe_cfg_default);
	if(ret != 0)
	{
		syslog_ax_fdb_err("Insert poe default configuration failed.\n");
		return NPD_FAIL;
	}
	register_netif_notifier(&poe_netif_notifier);
	
	npd_syslog_dbg("Create poeSock .\n");
	nam_thread_create("poeSock", (void *)npd_poe_poll_thread, NULL, NPD_TRUE, NPD_FALSE);
	nam_thread_create("SysPoeBasedTime", (void *)npd_poe_timer,NULL,NPD_TRUE,NPD_FALSE);
	return NPD_SUCCESS;
}

void npd_create_poe_intf
(
	unsigned int netif_index
) 
{
    poe_intf_db_t *poe_intf;
	unsigned int index;
	int slot = 0, sub_slot = 0;
	int port;
	
	if(PRODUCT_IS_BOX)
	{
	    slot = 0;
	}
	else
	{
	    slot = npd_netif_eth_get_slot(netif_index);
	}
	sub_slot = npd_netif_eth_get_subslot(netif_index);
	port=npd_netif_eth_get_port(netif_index);

	/* if this port is not poe port, don't create it */
	if(!IS_POE_PORT(slot, sub_slot, port))
	{
		return;
	}
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));
    if(NULL == poe_intf)
    {
        npd_syslog_err("% ERROR: SYSTEM resource exhausts\n");
        return;
    }
	memset( poe_intf, 0, sizeof(poe_intf_db_t));
    poe_intf->netif_index = netif_index;
	poe_intf->admin = 1; /* default is enable pse funtionality */
	poe_intf->poe_mode = POE_SIGNAL;
	poe_intf->detect_status = POE_DETECT_DISABLE;
	poe_intf->operate_status = POE_POWER_OFF;
	poe_intf->pd_type = NO_PD;
	poe_intf->pd_class = POE_CLASS_0;
	poe_intf->priority = POE_PRIORITY_NONE;
	poe_intf->power_up_mode = POE_POWER_UP_PRE_AT;
	poe_intf->power_user = 0;
	poe_intf->current = 0;
	poe_intf->voltage = 0;

	/*定义最大功率为用户配置，初始默认30.0*/
	poe_intf->power_threshold_type = POE_THRESHOLD_USER_DEFINE;
	poe_intf->max_power = CLASS4_POWER;
	
    dbtable_array_insert(poe_intfs, &index, (void*)poe_intf);
    
    poe_intf->poe_id = index;
	syslog_ax_eth_port_dbg("POE intf index %d for netif 0x%x\n", poe_intf->poe_id, netif_index);

    dbtable_array_update(poe_intfs, index, NULL, (void*)poe_intf);

	free(poe_intf);

	return;
}

void npd_delete_poe_intf
(
    unsigned int netif_index
) 
{
    poe_intf_db_t *poe_intf; 
	int slot, sub_slot;
	int port;
	unsigned int index;
	if(PRODUCT_IS_BOX)
	{
	    slot = 0;
	}
	else
	{
	    slot = npd_netif_eth_get_slot(netif_index);
	}
	sub_slot = npd_netif_eth_get_subslot(netif_index);
	port=npd_netif_eth_get_port(netif_index);

	/* if this port is not poe port, don't delete it */
	if(!IS_POE_PORT(slot, sub_slot, port))
	{
		return;
	}
	
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));
    if(NULL == poe_intf)
        return ;
	index = poe_intf->poe_id;
    dbtable_array_delete(poe_intfs, index, (void*)poe_intf);
    free(poe_intf);
	return;	
}


long poe_intf_handle_delete(void *newdata)
{
	return 0;
}

long poe_intf_handle_insert(void *newdata)
{
	poe_intf_db_t *poe_intf = (poe_intf_db_t*)newdata;
	int ret = 0, retval = 0;
	unsigned char poePort= 0;

	ret = npd_get_poeport_by_global_index(poe_intf->netif_index, &poePort);
	if (0!= ret)
		return -1;

//	printf("handle insert admin %d.\n", poe_intf->admin);
	{
		ret = nam_poe_port_poe_endis_set(poePort, poe_intf->admin);
		if (0 != ret)
			retval = -1;
	}

//	printf("handle insert admin %d.\n", poe_intf->priority);
	{
		ret = nam_poe_port_priority_set(poePort, poe_intf->priority);
		if (0 != ret)
			retval = -1;
	}
	
//	printf("handle insert poe_mode %d.\n", poe_intf->poe_mode);
	{
		ret = nam_poe_port_pair_set(poePort, poe_intf->poe_mode);
		if (0 != ret)
			retval = -1;
	}

/*设置端口功率分配方式*/
	{
		ret = nam_poe_power_threshold_set(poePort, poe_intf->power_threshold_type);
		if (0 != ret)
			retval = -1;
	}

/*设置端口最大功率，必须在功率分配方式为用户定义下才能执行*/
	{
		if (poe_intf->power_threshold_type == POE_THRESHOLD_USER_DEFINE)
		{
			ret = nam_poe_maxpower_threshold_set(poePort, poe_intf->max_power);
			if (0 != ret)
				retval = -1;
		}
				
	}
	{
		ret = nam_poe_power_up_mode_set(poePort, poe_intf->power_up_mode);
		if (0 != ret)
			retval = -1;
	}
	return retval;
	
}

long poe_intf_handle_update(void * newdata, void *olddata)
{
	poe_intf_db_t *new_poe_intf = (poe_intf_db_t*)newdata;
    poe_intf_db_t *old_poe_intf = (poe_intf_db_t*)olddata;
    int ret = 0, retval = 0;
	unsigned char poePort = 0;

	ret = npd_get_poeport_by_global_index(new_poe_intf->netif_index, &poePort);
	if (ret != 0)
	{
		return -1;
	}

//	printf("handle update new admin %d, old admin %d.\n", new_poe_intf->admin, old_poe_intf->admin);
	if (new_poe_intf->admin != old_poe_intf->admin)
	{
		ret = nam_poe_port_poe_endis_set(poePort, new_poe_intf->admin);
		if (0 != ret)
			retval = -1;
	}

//	printf("handle update new priority %d, old priority %d.\n", new_poe_intf->priority, old_poe_intf->priority);
	if (new_poe_intf->priority != old_poe_intf->priority)
	{
		ret = nam_poe_port_priority_set(poePort, new_poe_intf->priority);
		if (0 != ret)
			retval = -1;
	}

//	printf("handle update new poe_mode %d, old poe_mode %d.\n", new_poe_intf->poe_mode, old_poe_intf->poe_mode);
	if (new_poe_intf->poe_mode != old_poe_intf->poe_mode)
	{
		ret = nam_poe_port_pair_set(poePort, new_poe_intf->poe_mode);
		if (0 != ret)
			retval = -1;
	}
	
	if (new_poe_intf->legacy_dect != old_poe_intf->legacy_dect)
	{
		ret = nam_poe_detect_type_set(poePort, new_poe_intf->legacy_dect);
		if (0 != ret)
			retval = -1;
	}
	/*设置端口功率分配方式*/
	if (new_poe_intf->power_threshold_type != old_poe_intf->power_threshold_type)
	{
		ret = nam_poe_power_threshold_set(poePort, new_poe_intf->power_threshold_type);
		if (0 != ret)
			retval = -1;
	}

/*设置端口最大功率，必须在功率分配方式为用户定义下才能执行*/
	if (new_poe_intf->max_power != old_poe_intf->max_power)
	{
		if (new_poe_intf->power_threshold_type == POE_THRESHOLD_USER_DEFINE)
		{
			ret = nam_poe_maxpower_threshold_set(poePort, new_poe_intf->max_power);
			if (0 != ret)
				retval = -1;
		}
				
	}

	if (new_poe_intf->power_up_mode != old_poe_intf->power_up_mode)
	{
		ret = nam_poe_power_up_mode_set(poePort, new_poe_intf->power_up_mode);
		if (0 != ret)
			retval = -1;
	}

	return retval;
}

long pse_handle_delete(void *newdata)
{
	return 0;
}
long pse_handle_update(void * newdata, void *olddata)
{
	poe_pse_db_t *new_pse = (poe_pse_db_t*)newdata;
    poe_pse_db_t *old_pse = (poe_pse_db_t*)olddata;
    int ret = 0, retval = 0;

	if (new_pse->mode != old_pse->mode)
	{
		if (new_pse->mode == POE_PSE_AUTO_AND_STATIC)
		{
			ret = nam_poe_power_manage_set(1);
		}
		else if (new_pse->mode == POE_PSE_AUTO_AND_DYNAMIC)
		{
			ret = nam_poe_power_manage_set(2);
		}
		else if (new_pse->mode == POE_PSE_MANUAL_AND_STATIC)
		{
			ret = nam_poe_power_manage_set(3);
		}
		else if (new_pse->mode == POE_PSE_MANUAL_AND_DYNAMIC)
		{
			ret = nam_poe_power_manage_set(4);
		}
		else
		{
			ret = -1;
		}

		if (0 != ret)
			retval = -1;
	}
	return retval;
}
long pse_handle_insert(void *newdata)
{
	poe_pse_db_t *pse = (poe_pse_db_t*)newdata;
	int ret = 0, retval = 0;

	{
		if (pse->mode == POE_PSE_AUTO_AND_STATIC)
		{
			ret = nam_poe_power_manage_set(1);
		}
		else if (pse->mode == POE_PSE_AUTO_AND_DYNAMIC)
		{
			ret = nam_poe_power_manage_set(2);
		}
		else if (pse->mode == POE_PSE_MANUAL_AND_STATIC)
		{
			ret = nam_poe_power_manage_set(3);
		}
		else if (pse->mode == POE_PSE_MANUAL_AND_DYNAMIC)
		{
			ret = nam_poe_power_manage_set(4);
		}
		else
		{
			ret = -1;
		}
		if (0 != ret)
			retval = -1;
	}
	return retval;
}


static int npd_poe_enable_disable_config(unsigned char poe_enable)
{
	int ret = POE_RETURN_CODE_ERR_NONE;
    struct npd_poe_cfg_s npd_poe_cfg_set;
	npd_poe_cfg_set.poe_enable = poe_enable;	
    ret = dbtable_array_update(npd_poe_cfg_index, 0, &npd_poe_cfg_set, &npd_poe_cfg_set);
	return ret;
}

DBusMessage * npd_dbus_poe_create_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;

    unsigned int op_ret = GVRP_RETURN_CODE_ERR_NONE;
    int ret = 0;
    unsigned int index = 0;

    struct poe_time_range_info_s poe_tr;
    memset(&poe_tr, 0, sizeof(struct poe_time_range_info_s));
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
    
	ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_tr);
	if (ret != 0)
	{
        memset(&poe_tr, 0, sizeof(struct poe_time_range_info_s));
        poe_tr.index = index;
        ret = dbtable_array_insert_byid(npd_poe_time_range_index, index, &poe_tr);
        if (0 != ret)
            op_ret = POE_RETURN_CODE_ERR_GENERAL;
	}
    else
    {
        if(0 != poe_tr.bind_count)
        {
            op_ret = POE_RETURN_CODE_TIME_RANGE_USED;
        }
        else
        {
            op_ret = POE_RETURN_CODE_ERR_NONE;;
        }
    }
	
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
								    &op_ret);
	
	return reply;

}
DBusMessage * npd_dbus_poe_delete_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;

    unsigned int op_ret = GVRP_RETURN_CODE_ERR_NONE;
    int ret = 0;
    unsigned int index = 0;

    struct poe_time_range_info_s poe_tr;
    memset(&poe_tr, 0, sizeof(struct poe_time_range_info_s));
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_tr);
	if (ret != 0)
	{
        op_ret = POE_RETURN_CODE_NO_SUCH_TIME_RANGE;
	}
    else
    {
        if (0 != poe_tr.bind_count)
        {
            op_ret = POE_RETURN_CODE_TIME_RANGE_USED;
        }
        else
        {
            ret = dbtable_array_delete(npd_poe_time_range_index, index, &poe_tr);
            if(0 != ret)
                op_ret = POE_RETURN_CODE_ERR_GENERAL;
        }
    }
	
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
								    &op_ret);
	
	return reply;

}
int npd_poe_abs_time_create(unsigned int index, char *start_time, char *end_time)
{
    char *buf = NULL;
    unsigned int ret = 0;
    struct tm tm_start;
    struct tm tm_end;
    struct poe_time_range_info_s poe_time;

    memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));
    ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time);
    if(ret != 0)
    {
        return POE_RETURN_CODE_ERR_GENERAL;
    }

    if(0 != poe_time.bind_count)
    {
        return POE_RETURN_CODE_TIME_RANGE_USED;
    }
    
    if(1 == poe_time.abs_time.flag)
    {
        return POE_RETURN_CODE_ABS_TIME_EXIST;
    }
    
    buf = (char *)strptime(start_time, "%Y/%m/%d %H:%M:%S", &tm_start);
    if(buf != NULL)
    {
        poe_time.abs_time.start.year = tm_start.tm_year + 1900;
        poe_time.abs_time.start.month = tm_start.tm_mon + 1;
        poe_time.abs_time.start.day = tm_start.tm_mday;
        poe_time.abs_time.start.hh = tm_start.tm_hour;
        poe_time.abs_time.start.mm = tm_start.tm_min;
    }
    else
    {
        return POE_RETURN_CODE_ERR_GENERAL;
    }
    
    buf = (char *)strptime(end_time, "%Y/%m/%d %H:%M:%S", &tm_end);
    if(buf != NULL)
    {
        poe_time.abs_time.end.year = tm_end.tm_year + 1900;
        poe_time.abs_time.end.month = tm_end.tm_mon + 1;
        poe_time.abs_time.end.day = tm_end.tm_mday;
        poe_time.abs_time.end.hh = tm_end.tm_hour;
        poe_time.abs_time.end.mm = tm_end.tm_min;
    }
    else
    {
        return POE_RETURN_CODE_ERR_GENERAL;
    }
    
    poe_time.abs_time.flag = 1;
    
    ret = dbtable_array_update(npd_poe_time_range_index, index, &poe_time, &poe_time);

    return POE_RETURN_CODE_ERR_NONE;    
}
void npd_print_poe_time(unsigned int index)
{
    int ret = 0;
    struct poe_time_range_info_s poe_time1;
    memset(&poe_time1, 0, sizeof(&poe_time1));
    ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time1);
    if(0 == ret)
    {
        printf("poe_time.index = %d\n", poe_time1.index);
        printf("poe_time.periodic_time.flag = %d\n", poe_time1.periodic_time.flag);
        printf("poe_time.periodic_time.start.hh = %d\n", poe_time1.periodic_time.start.hh);
        printf("poe_time.periodic_time.start.mm = %d\n", poe_time1.periodic_time.start.mm);
        printf("poe_time.periodic_time.end.hh = %d\n", poe_time1.periodic_time.end.hh);
        printf("poe_time.periodic_time.end.mm = %d\n", poe_time1.periodic_time.end.mm);
    }
}
int npd_poe_periodic_time_create(unsigned int index, char *start_time, char *end_time, unsigned int flag)
{
    char *buf = NULL;
    unsigned int ret = 0;
    struct tm tm_start;
    struct tm tm_end;
    struct poe_time_range_info_s poe_time, poe_time1, poe_tmp;
    
    memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));
    ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time);
    if(ret != 0)
    {
        return POE_RETURN_CODE_ERR_GENERAL;
    }

    if(0 != poe_time.bind_count)
    {
        return POE_RETURN_CODE_TIME_RANGE_USED;
    }
    
    if(0 != poe_time.periodic_time.flag)
    {
        return POE_RETURN_CODE_PER_TIME_EXIST;
    }
    
    buf = (char *)strptime(start_time,"%Y/%m/%d %H:%M:%S",&tm_start);
    if(buf != NULL)
    {
        poe_time.periodic_time.start.hh = tm_start.tm_hour;
        poe_time.periodic_time.start.mm = tm_start.tm_min;

    }
    else
    {
        return POE_RETURN_CODE_ERR_GENERAL;
    }
    buf = (char *)strptime(end_time, "%Y/%m/%d %H:%M:%S", &tm_end);
    if(buf != NULL)
    {
        poe_time.periodic_time.end.hh= tm_end.tm_hour;
        poe_time.periodic_time.end.mm = tm_end.tm_min;

    }
    else
    {
        return POE_RETURN_CODE_ERR_GENERAL;
    }
    poe_time.periodic_time.flag = flag ;

    ret = dbtable_array_update(npd_poe_time_range_index, index, &poe_time, &poe_time);

    memset(&poe_time1, 0, sizeof(&poe_time1));
    ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time1);

    return POE_RETURN_CODE_ERR_NONE;   
}
DBusMessage * npd_dbus_poe_add_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = 0;
	unsigned int    index = 0;
    char*           start_time = NULL;
    char*           end_time = NULL;
	unsigned int    flag;
	
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_STRING, &start_time,
								DBUS_TYPE_STRING, &end_time,
								DBUS_TYPE_UINT32, &flag,
					            DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }
    if(flag == POE_ABS_TIME)
    {
        ret = npd_poe_abs_time_create(index, start_time, end_time);
    }
    else
    {
        ret = npd_poe_periodic_time_create(index, start_time, end_time, flag);
    }
	
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}
DBusMessage * npd_dbus_no_poe_time_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply; 
	DBusMessageIter	iter;
	DBusError		err;
	unsigned int 	ret = 0;
	unsigned int    index = 0;
	unsigned int    flag;
	struct poe_time_range_info_s poe_time;
    
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err, 
								DBUS_TYPE_UINT32, &index,
								DBUS_TYPE_UINT32, &flag,
					            DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
    }

    memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));
    ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time);
    
    if(flag == POE_ABS_TIME)
    {
        memset(&poe_time.abs_time, 0, sizeof(struct poe_time_abs_peri_s));
    }
    else
    {
        memset(&poe_time.periodic_time, 0, sizeof(struct poe_time_abs_peri_s));
    }

    ret = dbtable_array_update(npd_poe_time_range_index, index, &poe_time, &poe_time);
    
	reply = dbus_message_new_method_return(msg);
   
	dbus_message_iter_init_append (reply, &iter);
   
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}    
DBusMessage * npd_dbus_poe_on_time_deploy(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned int index = 0;	
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned int poe_status = 0;
	int ret = 0;
	unsigned int netif_index = 0;
    struct poe_time_range_info_s poe_time;
	poe_intf_db_t poe_intf = {0};

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
    memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));
    ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time);
    if(0 != ret)
    {
        op_ret = POE_RETURN_CODE_NO_SUCH_TIME_RANGE;
        goto retcode;
    }
    if((0 == poe_time.abs_time.flag) &&(0 == poe_time.periodic_time.flag))
    {
        op_ret = POE_RETURN_CODE_TIME_RANGE_INVILAID;
        goto retcode;
    }
    
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
    poe_status = npd_poe_cfg_get.poe_enable;
    
    if (poe_status == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	else
	{
		poe_intf.netif_index = netif_index;
		ret = dbtable_hash_search(poe_intfs_hash, &poe_intf, NULL, &poe_intf);
		if(ret != 0)
		{  
			ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        	goto retcode;
    	}

        if( poe_intf.time.poe_based_time_valid == 1)
        {
            ret = POE_RETURN_CODE_TIME_RANGE_BINDED;
            goto retcode;
        }
        poe_intf.time.poe_based_time_valid = 1;
        poe_intf.time.time_range_index = index;
        poe_intf.time.is_poe_deployed = poe_intf.admin;
		dbtable_hash_update(poe_intfs_hash, &poe_intf, &poe_intf);

        poe_time.bind_count++;
        ret = dbtable_array_update(npd_poe_time_range_index, index, &poe_time, &poe_time);
	}
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
								    &op_ret);
	
	return reply;

}
DBusMessage * npd_dbus_poe_show_time_range(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
 	unsigned int	name_count = 0;
	unsigned int	ret = 0;
 	char			tmp[256];
    char*           show_str = NULL;
    unsigned int    totalcount = 0;
    int             iCount = 0;
    struct poe_time_range_info_s poe_time;
	DBusMessage*	reply = NULL;    
	DBusMessageIter	iter;
    DBusError		err;

    dbus_error_init(&err);

	memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));

    show_str = (char *)malloc(POE_SHOW_TIME_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, POE_SHOW_TIME_SIZE);
        totalcount = dbtable_array_totalcount(npd_poe_time_range_index);
        for(iCount = 0; iCount < totalcount; iCount++)
        {
    	    ret = dbtable_array_get(npd_poe_time_range_index, iCount, &poe_time);
            if(0 != ret)
                continue;
    		memset(tmp, 0, sizeof(tmp));
            name_count++;
            sprintf(tmp, "%-15d%-40d\n", name_count, poe_time.index);
    		strcat(show_str, tmp);
        }
    }

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);     
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);
    if(show_str != NULL)
    {
        free(show_str);
        show_str = NULL;
    }
	return reply;
}
DBusMessage * npd_dbus_poe_show_time_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	unsigned int	uni = 0;
	unsigned int 	ret = 0;
	unsigned int    index = 0;
	char*			time_range_info_element[8];
    char            temp[33] ;

	struct poe_time_range_info_s poe_time;

	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
	
	memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));
    memset(temp, 0, 33);

    dbus_error_init(&err);
   
    if (!(dbus_message_get_args ( msg, &err,
					   DBUS_TYPE_UINT32, &index,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time);
    if(0 == ret)
    {
        for(uni=0; uni < 8; uni++)
        {
            time_range_info_element[uni] = (char* )malloc(33);
            time_range_info_element[uni][32]=0;
        }
        sprintf(time_range_info_element[0], "poe time-range %d", poe_time.index);
//        strncpy(time_range_info_element[0], time_range_info.name, 16);

        sprintf(temp, "%d", poe_time.bind_count);
        strncpy(time_range_info_element[1], temp, 33);
        
        if(1 == poe_time.abs_time.flag)
        {
            int mon1, mon2, day1, day2, hh1, hh2, mm1, mm2;
            mon1 = poe_time.abs_time.start.month/10;
            mon2 = poe_time.abs_time.start.month%10;
            day1 = poe_time.abs_time.start.day/10;
            day2 = poe_time.abs_time.start.day%10;
            hh1 = poe_time.abs_time.start.hh/10;
            hh2 = poe_time.abs_time.start.hh%10;
            mm1 = poe_time.abs_time.start.mm/10;
            mm2 = poe_time.abs_time.start.mm%10;
            strcpy(time_range_info_element[2], "YES");
            sprintf(temp, "%d-%d%d-%d%d %d%d:%d%d", poe_time.abs_time.start.year,
                                                mon1, mon2,
                                                day1, day2,
                                                hh1, hh2,
                                                mm1, mm2);
            strncpy(time_range_info_element[4], temp, 33);

            mon1 = poe_time.abs_time.end.month/10;
            mon2 = poe_time.abs_time.end.month%10;
            day1 = poe_time.abs_time.end.day/10;
            day2 = poe_time.abs_time.end.day%10;
            hh1 = poe_time.abs_time.end.hh/10;
            hh2 = poe_time.abs_time.end.hh%10;
            mm1 = poe_time.abs_time.end.mm/10;
            mm2 = poe_time.abs_time.end.mm%10;
            sprintf(temp, "%d-%d%d-%d%d %d%d:%d%d", poe_time.abs_time.end.year,
                                                mon1, mon2,
                                                day1, day2,
                                                hh1, hh2,
                                                mm1, mm2);
            strncpy(time_range_info_element[5], temp, 33);  
        }
        else
        {
            strcpy(time_range_info_element[2], "NO");
            sprintf(temp, "NOT SET");
            strncpy(time_range_info_element[4], temp, 33);
            strncpy(time_range_info_element[5], temp, 33);
        }

        switch(poe_time.periodic_time.flag)
        {
            case POE_TIME_EVERYDAY:
                strcpy(time_range_info_element[3], "everyday");break;
            case POE_TIME_WORKDAY:
                strcpy(time_range_info_element[3], "workday");break;
            case POE_TIME_WEEKEND:
                strcpy(time_range_info_element[3], "weekend");break;
            default:
                strcpy(time_range_info_element[3], "NO");
                    break;
        }

        if(0 == poe_time.periodic_time.flag)
        {
            strcpy(time_range_info_element[6], "NOT SET");
            strcpy(time_range_info_element[7], "NOT SET");
        }
        else
        {
            int hh1, hh2, mm1, mm2;
            hh1 = poe_time.periodic_time.start.hh/10;
            hh2 = poe_time.periodic_time.start.hh%10;
            mm1 = poe_time.periodic_time.start.mm/10;
            mm2 = poe_time.periodic_time.start.mm%10;
            sprintf(temp, "           %d%d:%d%d", hh1, hh2, mm1, mm2);
            strncpy(time_range_info_element[6], temp, 33);

            hh1 = poe_time.periodic_time.end.hh/10;
            hh2 = poe_time.periodic_time.end.hh%10;
            mm1 = poe_time.periodic_time.end.mm/10;
            mm2 = poe_time.periodic_time.end.mm%10;
            sprintf(temp, "           %d%d:%d%d", hh1, hh2, mm1, mm2); 
            strncpy(time_range_info_element[7], temp, 33);

        }

    }
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &ret);
    if(ret == 0)
    {
        for (uni = 0; uni < 8; uni++)
        {  
        	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &time_range_info_element[uni]);
        }

        for (uni = 0; uni < 8; uni++)
        {
        	free(time_range_info_element[uni]);
        }
    }

	return reply;
}
DBusMessage * npd_dbus_poe_show_time_bind_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage*	reply = NULL;
	DBusMessageIter	iter;
	DBusError		err;
    unsigned int    index = 0;
    unsigned int    netif_index = 0;
    char* show_str = NULL;
    char tmp[32];
    char port[32];
    int totalcount = 0;
    int uni = 0;
    int count = 0;
    int op_ret = 0;
    struct poe_intf_db_s poe_item;
    
    dbus_error_init(&err);
   
    if (!(dbus_message_get_args (msg, &err,
					   DBUS_TYPE_UINT32, &index,
					   DBUS_TYPE_INVALID))) 
	{
		syslog_ax_acl_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_acl_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
    
    show_str = (char *)malloc(POE_SHOW_TIME_SIZE);
    if(NULL != show_str)
    {
        memset(show_str, 0, POE_SHOW_TIME_SIZE);

        totalcount = dbtable_array_totalcount(poe_intfs);
        for(uni = 0; uni < totalcount; uni++)
        {
            memset(&poe_item, 0, sizeof(struct poe_intf_db_s));
            memset(port, 0, sizeof(port));
            memset(tmp, 0, sizeof(tmp));
            netif_index = 0;
            
            op_ret = dbtable_array_get(poe_intfs, uni, &poe_item);        
            if(0 != op_ret)
                continue;
            if(0 == poe_item.time.poe_based_time_valid)
                continue;
            if(index != poe_item.time.time_range_index)
                continue;

            netif_index = poe_item.netif_index;
            parse_eth_index_to_name(netif_index, port);
                
            sprintf(tmp, "%-12s", port);
            strcat(show_str, tmp);
            count++;
            if(0 == count % 5)
            {
                sprintf(tmp, "\n");
                strcat(show_str, tmp);
            }

        }
        if(0 != count % 5)
        {
            sprintf(tmp, "\n");
            strcat(show_str, tmp);  
        }
    }

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
    dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &show_str);

    if(show_str)
    {
        free(show_str);
        show_str = NULL;
    }
    return reply;
}
DBusMessage * npd_dbus_poe_config_global_endis(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	unsigned char is_enable = 0;	
	npd_poe_cfg_t npd_poe_cfg_get;
	poe_pse_db_t poe_pse;
	poe_intf_db_t poe_intf = {0};

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_BYTE, &is_enable,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args ");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}

    if(is_enable == npd_poe_cfg_get.poe_enable)
    {
		op_ret = POE_RETURN_CODE_ERR_NONE;
        goto retcode;
    }
	else
	{
		/*做POE disable操作，关闭所有PSE供电，盒式设备关闭一个即可*/
		ret = dbtable_array_get(poe_pse_index, 0, &poe_pse);
		if (ret != 0)
		{
			npd_syslog_dbg("Can not find pse.\n");
			op_ret = POE_RETURN_CODE_NO_SUCH_PSE;
			goto retcode;
		}

		poe_pse.admin = is_enable;
		dbtable_array_update(poe_pse_index, 0, &poe_pse, &poe_pse);
		ret = npd_poe_enable_disable_config(is_enable);
		if (0 == ret)
		{
			while (1)
			{
				if (0 == poe_intf.netif_index)
				{
					ret = dbtable_hash_head(poe_intfs_hash, &poe_intf, &poe_intf, NULL);	
				}
				else
				{
					ret = dbtable_hash_next(poe_intfs_hash, &poe_intf, &poe_intf, NULL);
				}
				if (0 != ret)
					break;
				poe_intf.admin = is_enable;
				dbtable_hash_update(poe_intfs_hash, &poe_intf, &poe_intf);
			}
		}
	}
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
								    &op_ret);
	
	return reply;

}

DBusMessage * npd_dbus_poe_config_port_endis(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	unsigned char is_enable = 0;	
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned int poe_status = 0;
	int ret = 0;
	unsigned int netif_index = 0;

	poe_intf_db_t poe_intf = {0};

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index,
							DBUS_TYPE_BYTE, &is_enable,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
    poe_status = npd_poe_cfg_get.poe_enable;
    
    if (poe_status == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	else
	{
		poe_intf.netif_index = netif_index;
		ret = dbtable_hash_search(poe_intfs_hash, &poe_intf, NULL, &poe_intf);
		if(ret != 0)
		{  
			ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        	goto retcode;
    	}
        if(is_enable == 1)
        {
            if(1 == poe_intf.time.poe_based_time_valid)
            {
                ret = POE_RETURN_CODE_ERR_NONE;
                goto retcode;
            }
        }
        else
        {
            unsigned int index = 0;
            struct poe_time_range_info_s poe_time;
            memset(&poe_time, 0, sizeof(&poe_time));
            
            index = poe_intf.time.time_range_index;
            ret = dbtable_array_get(npd_poe_time_range_index, index, &poe_time);
            if(0 == ret)
            {
                poe_time.bind_count--;
            }
            ret = dbtable_array_update(npd_poe_time_range_index, index, &poe_time, &poe_time);

            poe_intf.time.poe_based_time_valid = 0;
            poe_intf.time.time_range_index = 0;
        }
		if (is_enable == poe_intf.admin)
		{
			ret = POE_RETURN_CODE_ERR_NONE;
        	goto retcode;
		}
		poe_intf.admin = is_enable;
		dbtable_hash_update(poe_intfs_hash, &poe_intf, &poe_intf);
		
	}
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
								    &op_ret);
	
	return reply;

}

DBusMessage * npd_dbus_poe_config_port_max_power(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	poe_intf_db_t *poe_intf = NULL;
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned int netif_index = 0;
	unsigned int port_max_power = 0;
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));

	if (NULL == poe_intf)
	{
		npd_syslog_err("Can not create poe_intf.\n");
		ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}

	dbus_error_init(&error);
	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index,
							DBUS_TYPE_UINT32, &port_max_power,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
	if (npd_poe_cfg_get.poe_enable == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	poe_intf->netif_index = netif_index;
	ret = dbtable_hash_search(poe_intfs_hash, poe_intf, NULL, poe_intf);
	if(ret != 0)
	{  
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        goto retcode;
    }
	if (poe_intf->admin == 0)
	{
		op_ret = POE_RETURN_CODE_PORT_DISABLE;
        goto retcode;
	}
	poe_intf->max_power = port_max_power;
	/*置端口最大功率为用户管理，端口link 后不会根据检测PD等级
	分配功率*/
	poe_intf->max_power_admin_flag = 1;
	dbtable_hash_update(poe_intfs_hash, poe_intf, poe_intf);
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
    if(poe_intf)
		free(poe_intf);
	return reply;

}

DBusMessage * npd_dbus_poe_config_port_priority(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	poe_intf_db_t *poe_intf = NULL;
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned int netif_index = 0;
	unsigned char poe_priority = 0;
	unsigned char poe_status = 0;
	unsigned char pd_class = 0;
	unsigned char pd_type = 0;
	unsigned char err_type = 0;
	unsigned char poe_port = 0;
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));

	if (NULL == poe_intf)
	{
		npd_syslog_err("Can not create poe_intf.\n");
		ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	dbus_error_init(&error);
	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index,
							DBUS_TYPE_BYTE, &poe_priority,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
	if (npd_poe_cfg_get.poe_enable == 0)
    {
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	poe_intf->netif_index = netif_index;
	ret = dbtable_hash_search(poe_intfs_hash, poe_intf, NULL, poe_intf);
	if(ret != 0)
	{  
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        goto retcode;
    }
	if (poe_intf->admin == 0)
	{
		op_ret = POE_RETURN_CODE_PORT_DISABLE;
        goto retcode;
	}
	else
	{

		ret = npd_get_poeport_by_global_index(netif_index, &poe_port);
		if (0 != ret)
		{
			op_ret = POE_RETURN_CODE_ERR_GENERAL;
			goto retcode;
		}
		nam_poe_port_status_get(poe_port, &poe_status, &err_type, &pd_class, &pd_type);

		/*端口down时如果配置端口优先级为low时，默认配置为NONE*/
		if (poe_status != POE_DETECT_DELIVERING_POWER)
		{
			if (poe_priority == POE_PRIORITY_LOW)
	    		poe_priority = POE_PRIORITY_NONE;
		}
		poe_intf->priority = poe_priority;
		dbtable_hash_update(poe_intfs_hash, poe_intf, poe_intf);
		op_ret = ret;
	}
	
retcode:
	reply = dbus_message_new_method_return(msg);
		
	dbus_message_iter_init_append (reply, &iter);
		
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	if(poe_intf)
		free(poe_intf);
	return reply;
}

DBusMessage * npd_dbus_poe_config_power_manage_mode(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	npd_poe_cfg_t npd_poe_cfg_get;
	poe_pse_db_t poe_pse;
	unsigned char poe_power_manage_mode = 0;

	dbus_error_init(&error);
	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_BYTE, &poe_power_manage_mode,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
	if (npd_poe_cfg_get.poe_enable == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}

	ret = dbtable_array_get(poe_pse_index, 0, &poe_pse);
	if (ret != 0)
	{
		npd_syslog_dbg("Can not find pse.\n");
		op_ret = POE_RETURN_CODE_NO_SUCH_PSE;
		goto retcode;
	}
	if (poe_power_manage_mode == poe_pse.mode)
	{
		op_ret = 0;
		goto retcode;
	}
	poe_pse.mode = poe_power_manage_mode;
	dbtable_array_update(poe_pse_index, 0, &poe_pse, &poe_pse);
retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
			
	return reply;

}

DBusMessage * npd_dbus_poe_config_poe_interface_mode(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	poe_intf_db_t *poe_intf = NULL;
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned int netif_index = 0;
	unsigned char poe_interface_mode = 0;
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));

	if (NULL == poe_intf)
	{
		npd_syslog_err("Can not create poe_intf.\n");
		ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index, 
							DBUS_TYPE_BYTE, &poe_interface_mode,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
	if (npd_poe_cfg_get.poe_enable == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	poe_intf->netif_index = netif_index;
	ret = dbtable_hash_search(poe_intfs_hash, poe_intf, NULL, poe_intf);
	if(ret != 0)
	{  
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        goto retcode;
    }
	if (poe_intf->admin == 0)
	{
		op_ret = POE_RETURN_CODE_PORT_DISABLE;
        goto retcode;
	}
	if (poe_intf->poe_mode == poe_interface_mode)
	{
		op_ret = 0;
		goto retcode;
	}
	poe_intf->poe_mode = poe_interface_mode;
	ret = dbtable_hash_update(poe_intfs_hash, poe_intf, poe_intf);

retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	if(poe_intf)
		free(poe_intf);
	return reply;

}

DBusMessage * npd_dbus_poe_config_power_mode(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	unsigned char poePort = 0;
	poe_intf_db_t *poe_intf = NULL;
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned int netif_index = 0;
	unsigned char poe_power_mode = 0;
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));

	if (NULL == poe_intf)
	{
		npd_syslog_err("Can not create poe_intf.\n");
		ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index, 
							DBUS_TYPE_BYTE, &poe_power_mode,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
	if (npd_poe_cfg_get.poe_enable == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	poe_intf->netif_index = netif_index;
	ret = dbtable_hash_search(poe_intfs_hash, poe_intf, NULL, poe_intf);
	if(ret != 0)
	{  
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        goto retcode;
    }
	if (poe_intf->admin == 0)
	{
		op_ret = POE_RETURN_CODE_PORT_DISABLE;
        goto retcode;
	}
	ret = npd_get_poeport_by_global_index(poe_intf->netif_index, &poePort);
	ret = nam_poe_high_power_mode_set(poePort, poe_power_mode);
	if (ret != 0)
		op_ret = POE_RETURN_CODE_ERR_GENERAL;

retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	if(poe_intf)
		free(poe_intf);
	return reply;

}

DBusMessage * npd_dbus_poe_config_power_up_mode(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	unsigned char poePort = 0;
	poe_intf_db_t *poe_intf = NULL;
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned int netif_index = 0;
	unsigned char power_up_mode = 0;
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));

	if (NULL == poe_intf)
	{
		npd_syslog_err("Can not create poe_intf.\n");
		ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index, 
							DBUS_TYPE_BYTE, &power_up_mode,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
	if (npd_poe_cfg_get.poe_enable == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	poe_intf->netif_index = netif_index;
	ret = dbtable_hash_search(poe_intfs_hash, poe_intf, NULL, poe_intf);
	if(ret != 0)
	{  
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        goto retcode;
    }
	if (poe_intf->admin == 0)
	{
		op_ret = POE_RETURN_CODE_PORT_DISABLE;
        goto retcode;
	}
	if (poe_intf->power_up_mode == power_up_mode)
	{
		op_ret = 0;
		goto retcode;
	}
	poe_intf->power_up_mode = power_up_mode;
	ret = dbtable_hash_update(poe_intfs_hash, poe_intf, poe_intf);

retcode:
	reply = dbus_message_new_method_return(msg);
			
	dbus_message_iter_init_append (reply, &iter);
			
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	if(poe_intf)
		free(poe_intf);
	return reply;

}


DBusMessage * npd_dbus_poe_config_port_poe_legacy_check(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	unsigned int netif_index = 0;
	poe_intf_db_t *poe_intf = NULL;
	npd_poe_cfg_t npd_poe_cfg_get;
	unsigned char is_enable = 0;
	poe_intf = (poe_intf_db_t *)malloc(sizeof(poe_intf_db_t));

	if (NULL == poe_intf)
	{
		npd_syslog_err("Can not create poe_intf.\n");
		ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}

	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &netif_index, 
							DBUS_TYPE_BYTE, &is_enable,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable.\n");
		 op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
		 goto retcode;
	}
	if (npd_poe_cfg_get.poe_enable == 0)
    {
		npd_syslog_dbg("Global poe is disable.\n");
		op_ret = POE_RETURN_CODE_GLOBAL_DISABLE;
		goto retcode;
	}
	poe_intf->netif_index = netif_index;
	ret = dbtable_hash_search(poe_intfs_hash, poe_intf, NULL, poe_intf);
	if(ret != 0)
	{  
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        goto retcode;
    }
	if (poe_intf->admin == 0)
	{
		op_ret = POE_RETURN_CODE_PORT_DISABLE;
        goto retcode;
	}
	if (is_enable)
	{
		if ((poe_intf->legacy_dect == FOUR_PIN_DETECT_LEGACY) || (poe_intf->legacy_dect == TWO_PIN_DETECT_LEGACY))
		op_ret = 0;
		goto retcode;
	}
	else
	{
		if ((poe_intf->legacy_dect != FOUR_PIN_DETECT_LEGACY) && (poe_intf->legacy_dect != TWO_PIN_DETECT_LEGACY))
		op_ret = 0;
		goto retcode;
	}
	if (is_enable)
		poe_intf->legacy_dect = FOUR_PIN_DETECT_LEGACY;
	else
		poe_intf->legacy_dect = FOUR_PIN_DETECT;
	ret = dbtable_hash_update(poe_intfs_hash, poe_intf, poe_intf);
retcode:
		reply = dbus_message_new_method_return(msg);
				
		dbus_message_iter_init_append (reply, &iter);
			
		dbus_message_iter_append_basic (&iter,
										DBUS_TYPE_UINT32,
										&op_ret);
	if(poe_intf)
		free(poe_intf);
		return reply;

}

DBusMessage * npd_dbus_poe_get_poe_interface(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	unsigned int  port_index = 0;
	poe_intf_db_t poe_intf = {0};

	unsigned char poePort = 0;
	unsigned char portStat = 0;
	unsigned char errType = 0;
	unsigned char pdClass = 0;
	unsigned char pdType = 0;
	unsigned char isEnable = 0;
	unsigned char autoMode = 0;
	unsigned char detectType = 0;
	unsigned char classType = 0;
	unsigned char disconnType = 0;
	unsigned char pairType   = 0;
	unsigned char powerUpMode  = 0;
	unsigned char violaType = 0;
	unsigned char operateStatus = 0;
	unsigned char legacyDetect = 0;
	
	unsigned char portPriority = 0;
	int voltage = 0;
	int poeCurrent = 0;
	int temperature = 0;
	int power = 0;
	int maxPower  = 0;
	
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &port_index,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}

	poe_intf.netif_index = port_index;
	ret = dbtable_hash_search(poe_intfs_hash, &poe_intf, NULL, &poe_intf);
	if(ret != 0)
	{  
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
        goto retcode;
    }

	ret = npd_get_poeport_by_global_index(port_index, &poePort);
	if (ret != 0)
	{
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
		goto retcode;
	}

	nam_poe_port_status_get(poePort, &portStat, &errType, &pdClass, &pdType);
	nam_poe_port_config_get(poePort, &isEnable, &autoMode, &detectType, &classType, &disconnType, &pairType);
	nam_poe_port_extended_config_get(poePort, &powerUpMode, &violaType, &maxPower, &portPriority);
	nam_poe_port_measure_get(poePort, &voltage, &poeCurrent, &temperature, &power);
	
	if (0 == violaType)/*根据最大功率来*/
	{
		if (pdType == IEEE_POEPLUS)/*at pd设备*/
			maxPower = CLASS4_POWER;
		else
			maxPower = CLASS0_POWER;
	}
	else if (1 == violaType)
	{
		switch (pdClass)
		{
			case 0:
				maxPower = CLASS0_POWER;
				break;
			case 1:
				maxPower = CLASS1_POWER;
				break;
			case 2:
				maxPower = CLASS2_POWER;
				break;
			case 3:
				maxPower = CLASS3_POWER;
				break;
			case 4:
				maxPower = CLASS4_POWER;
				break;
			default:
				maxPower = CLASS0_POWER;
				break;
		}
	}
	if ((FOUR_PIN_DETECT_LEGACY == detectType) || (TWO_PIN_DETECT_LEGACY == detectType))
		legacyDetect = 1;
	else
		legacyDetect = 0;

	switch (portStat)
	{
		case POE_DETECT_DISABLE:
			operateStatus = POE_POWER_OFF;
			break;
		case POE_DETECT_DELIVERING_POWER:
			operateStatus = POE_POWER_ON;
			break;
		case POE_DETECT_FAULT:
			operateStatus = errType + 1;
			break;
		case POE_DETECT_OTHER_FAULT:
			operateStatus = errType + 1;
			break;
		default:
			operateStatus = POE_POWER_OFF;
			break;	
	}
	if (portStat != POE_DETECT_DELIVERING_POWER)
	{
		if (maxPower == CLASS4_POWER)
		{
			pdClass = POE_CLASS_4; /*如果端口DISABLE默认PD分级为CLASS_4*/

		}
		else
		{
			pdClass = POE_CLASS_0; /*如果端口DISABLE默认PD分级为CLASS_4*/
		}
	}
	
	if (portPriority <= POE_PRIORITY_LOW)
		portPriority = POE_PRIORITY_LOW;
	
	//printf("pdType %d, pdClass %d, violaType %d, MaxPower %d.\n", pdType, pdClass, violaType, maxPower);
	
retcode:
	reply = dbus_message_new_method_return(msg);
					
	dbus_message_iter_init_append (reply, &iter);
				
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	if (op_ret == 0)
	{
		 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poe_intf.poe_id)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(pairType)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(isEnable)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(portStat)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(operateStatus)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(pdType)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(pdClass)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(portPriority)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(maxPower)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(power)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poeCurrent)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(voltage)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(temperature)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE,  &(legacyDetect));
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE,  &(powerUpMode));	
	}
					
	return reply;
}

DBusMessage * npd_dbus_poe_get_next_poe_interface(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	unsigned int  port_index = 0;
	poe_intf_db_t poe_intf = {0};


	unsigned char poePort = 0;
	unsigned char portStat = 0;
	unsigned char errType = 0;
	unsigned char pdClass = 0;
	unsigned char pdType = 0;
	unsigned char isEnable = 0;
	unsigned char autoMode = 0;
	unsigned char detectType = 0;
	unsigned char classType = 0;
	unsigned char disconnType = 0;
	unsigned char pairType   = 0;
	unsigned char powerUpMode  = 0;
	unsigned char violaType = 0;	
	unsigned char portPriority = 0;
	unsigned char operateStatus = 0;
	unsigned char legacyDetect = 0;
	int voltage = 0;
	int poeCurrent = 0;
	int temperature = 0;
	int power = 0;
	int maxPower  = 0;
	
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &port_index,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}

	if (0 == port_index)
	{
		ret = dbtable_hash_head(poe_intfs_hash, 
			&poe_intf, &poe_intf, NULL);
		if (ret != 0)
		{
			op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
			goto retcode;
		}
	}
	else
	{
		poe_intf.netif_index = port_index;
		ret = dbtable_hash_next(poe_intfs_hash, &poe_intf, &poe_intf, NULL);
		if (ret != 0)
		{
			op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
			goto retcode;
		}
	}
	char port_string[10] = {0};
    parse_eth_index_to_name(poe_intf.netif_index, (unsigned char *)port_string);
	ret = npd_get_poeport_by_global_index(poe_intf.netif_index, &poePort);
	if (ret != 0)
	{
		op_ret = POE_RETURN_CODE_NO_SUCH_INTERFACE;
		goto retcode;
	}

	nam_poe_port_status_get(poePort, &portStat, &errType, &pdClass, &pdType);
	nam_poe_port_config_get(poePort, &isEnable, &autoMode, &detectType, &classType, &disconnType, &pairType);
	nam_poe_port_extended_config_get(poePort, &powerUpMode, &violaType, &maxPower, &portPriority);
	nam_poe_port_measure_get(poePort, &voltage, &poeCurrent, &temperature, &power);

	if (0 == violaType)/*根据最大功率来*/
	{
		if (pdType == IEEE_POEPLUS)/*at pd设备*/
			maxPower = CLASS4_POWER;
		else
			maxPower = CLASS0_POWER;
	}
	else if (1 == violaType)
	{
		switch (pdClass)
		{
			case 0:
				maxPower = CLASS0_POWER;
				break;
			case 1:
				maxPower = CLASS1_POWER;
				break;
			case 2:
				maxPower = CLASS2_POWER;
				break;
			case 3:
				maxPower = CLASS3_POWER;
				break;
			case 4:
				maxPower = CLASS4_POWER;
				break;
			default:
				maxPower = CLASS0_POWER;
				break;
		}
	}
	if ((3 == detectType) || (5 == detectType))
		legacyDetect = 1;
	else
		legacyDetect = 0;
	switch (portStat)
	{
		case POE_DETECT_DISABLE:
			operateStatus = POE_POWER_OFF;
			break;
		case POE_DETECT_DELIVERING_POWER:
			operateStatus = POE_POWER_ON;
			break;
		case POE_DETECT_FAULT:
			operateStatus = errType + 1;
			break;
		case POE_DETECT_OTHER_FAULT:
			operateStatus = errType + 1;
			break;
		default:
			operateStatus = POE_POWER_OFF;
			break;	
	}
	if (portStat != POE_DETECT_DELIVERING_POWER)
	{
		if (maxPower == CLASS4_POWER)
		{
			pdClass = POE_CLASS_4; /*如果端口DISABLE默认PD分级为CLASS_4*/

		}
		else
		{
			pdClass = POE_CLASS_0; /*如果端口DISABLE默认PD分级为CLASS_4*/
		}
	}
	if (portPriority <= POE_PRIORITY_LOW)
		portPriority = POE_PRIORITY_LOW;
	
retcode:
	reply = dbus_message_new_method_return(msg);
					
	dbus_message_iter_init_append (reply, &iter);
				
	dbus_message_iter_append_basic (&iter,
									DBUS_TYPE_UINT32,
									&op_ret);
	if (op_ret == 0)
	{
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poe_intf.netif_index)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poe_intf.poe_id)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(pairType)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(isEnable)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(portStat)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(operateStatus)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(pdType)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(pdClass)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(portPriority)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(maxPower)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(power)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poeCurrent)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(voltage)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(temperature)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(legacyDetect));
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE, &(powerUpMode));
	}
					
	return reply;
}

DBusMessage * npd_dbus_poe_get_pse_info(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage *reply = NULL;
	DBusError error;
	DBusMessageIter  iter;
	unsigned int op_ret = 0;
	int ret = 0;
	unsigned int  pse_id = 0;
	poe_pse_db_t poe_pse;

	unsigned char mpss = 0;
	unsigned int pse_mode = 0;
	unsigned char powerManaMode = 0;
	int totalPower = 0;
	int guardBand = 0;
	int total_power_allocate = 0;
	int available_power = 0;
	
	dbus_error_init(&error);

	if (!(dbus_message_get_args(msg, &error,
							DBUS_TYPE_UINT32, &pse_id,
							DBUS_TYPE_INVALID)))
	{
		npd_syslog_err("Unable to get input args.\n");
		if (dbus_error_is_set(&error))
		{
			npd_syslog_err("%s raised: %s",error.name,error.message);
			dbus_error_free(&error);
		}
        op_ret = POE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	
	ret = dbtable_array_get(poe_pse_index, 0, &poe_pse);
	if (ret != 0)
	{
		npd_syslog_dbg("Can not find pse.\n");
		op_ret = POE_RETURN_CODE_NO_SUCH_PSE;
		goto retcode;
	}
	mpss = MODULE_PSE_MPSS_SLOT_INDEX(0);
	nam_poe_power_manage_config_get(mpss, &powerManaMode, &totalPower, &guardBand);
	nam_poe_total_power_allocate_get(&total_power_allocate, &available_power);
	/*0表示没有电源管理方式为MANUAL*/
    if (1 == powerManaMode)
		pse_mode = POE_PSE_AUTO_AND_STATIC;
	else if(2 == powerManaMode)
		pse_mode = POE_PSE_AUTO_AND_DYNAMIC;
	else if(3 == powerManaMode)
		pse_mode = POE_PSE_MANUAL_AND_STATIC;
	else
		pse_mode = POE_PSE_MANUAL_AND_DYNAMIC;

	int pse_total_power = MODULE_PSE_TOTAL_POWER_ON_SLOT_INDEX(0);
	available_power = pse_total_power - total_power_allocate;
retcode:
	reply = dbus_message_new_method_return(msg);
					
	dbus_message_iter_init_append (reply, &iter);
				
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32, &op_ret);
	if (op_ret == 0)
	{
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poe_pse.slot_no)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_BYTE,   &(poe_pse.admin)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poe_pse.pse_type)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(totalPower)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(available_power)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poe_pse.current)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(poe_pse.voltage)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(pse_mode)); 
		dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &(guardBand)); 
	}				
	return reply;

}

DBusMessage *npd_dbus_show_poe_running_config
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL; 
	DBusMessageIter iter;

	int str_length = 0;
	char *showStr = NULL;
	char *cursor = NULL;
	/*保存端口是否配置*/
	char *configed_str = NULL;
	int configed_length = 0;
	int eth_name_length = 0;
    int totalcount = 0;
    unsigned int uni = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	poe_pse_db_t poe_pse = {0};
	poe_intf_db_t poe_intf = {0};
	npd_poe_cfg_t npd_poe_cfg_get = {0};
	char eth_name[30] = {0};

	ret = dbtable_array_get(poe_pse_index, 0, &poe_pse);
	if (ret != 0)
	{
		npd_syslog_dbg("Can not find pse.\n");
		op_ret = POE_RETURN_CODE_NO_SUCH_PSE;
		goto retcode;
	}
	
	showStr = (char *)malloc(NPD_ETHPORT_SHOWRUN_CFG_SIZE);
	if(NULL == showStr) 
	{
		npd_syslog_err("memory malloc for show gvrp running config error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_ETHPORT_SHOWRUN_CFG_SIZE);
	ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
	if(ret != 0)
	{
	     npd_syslog_dbg("Can not find the g_poe_enable");
		 op_ret = POE_RETURN_CODE_ERR_GENERAL;
		 goto retcode;
	}
	cursor = showStr;
    
    totalcount = dbtable_array_totalcount(npd_poe_time_range_index);
    for(uni = 0; uni < totalcount; uni++)
    {
        int mon1 = 0, mon2 = 0;
        int day1 = 0, day2 = 0;
        int hh1 = 0, hh2 = 0;
        int mm1 = 0, mm2 = 0;
        char start_time_str[32];
        char end_time_str[32];
        char typestr[32];
        struct poe_time_range_info_s poe_time;
        memset(&poe_time, 0, sizeof(struct poe_time_range_info_s));
        ret = dbtable_array_get(npd_poe_time_range_index, uni, &poe_time);
        if(ret != 0)
            continue;

        str_length += sprintf(cursor, "poe time-range %d\n", poe_time.index);
        cursor = showStr + str_length;

        if(1 == poe_time.abs_time.flag)
        {
            memset(start_time_str, 0, 32);
            memset(end_time_str, 0, 32);
            mon1 = poe_time.abs_time.start.month/10;
            mon2 = poe_time.abs_time.start.month%10;
            day1 = poe_time.abs_time.start.day/10;
            day2 = poe_time.abs_time.start.day%10;
            hh1 = poe_time.abs_time.start.hh/10;
            hh2 = poe_time.abs_time.start.hh%10;
            mm1 = poe_time.abs_time.start.mm/10;
            mm2 = poe_time.abs_time.start.mm%10;

            sprintf(start_time_str,"start %d/%d%d/%d%d %d%d:%d%d ", poe_time.abs_time.start.year,
                                                        mon1, mon2,
                                                        day1, day2,
                                                        hh1, hh2,
                                                        mm1, mm2);
            mon1 = poe_time.abs_time.end.month/10;
            mon2 = poe_time.abs_time.end.month%10;
            day1 = poe_time.abs_time.end.day/10;
            day2 = poe_time.abs_time.end.day%10;
            hh1 = poe_time.abs_time.end.hh/10;
            hh2 = poe_time.abs_time.end.hh%10;
            mm1 = poe_time.abs_time.end.mm/10;
            mm2 = poe_time.abs_time.end.mm%10;
            sprintf(end_time_str,"end %d/%d%d/%d%d %d%d:%d%d ", poe_time.abs_time.end.year,
                                                                    mon1, mon2,
                                                                    day1, day2,
                                                                    hh1, hh2,
                                                                    mm1, mm2);

            str_length += sprintf(cursor, " absolute %s%s\n", start_time_str, end_time_str);
            cursor = showStr + str_length;
        }

        if(0 != poe_time.periodic_time.flag)
        {
            memset(start_time_str, 0, 32);
            memset(end_time_str, 0, 32);
            memset(typestr, 0, 32);
            
            if(TIME_RANGE_EVERYDAY == poe_time.periodic_time.flag)
                sprintf(typestr, "periodic everyday ");
            else if(TIME_RANGE_WORKDAY == poe_time.periodic_time.flag)
                sprintf(typestr, "periodic workday ");
            else if(TIME_RANGE_WEEKEND == poe_time.periodic_time.flag)
                sprintf(typestr, "periodic weekend ");
            
            hh1 = poe_time.periodic_time.start.hh/10;
            hh2 = poe_time.periodic_time.start.hh%10;
            mm1 = poe_time.periodic_time.start.mm/10;
            mm2 = poe_time.periodic_time.start.mm%10;        
            sprintf(start_time_str, "start %d%d:%d%d ", hh1, hh2, mm1, mm2);

            hh1 = poe_time.periodic_time.end.hh/10;
            hh2 = poe_time.periodic_time.end.hh%10;
            mm1 = poe_time.periodic_time.end.mm/10;
            mm2 = poe_time.periodic_time.end.mm%10;
            sprintf(end_time_str, "end %d%d:%d%d", hh1, hh2, mm1, mm2);

            str_length += sprintf(cursor, " %s%s%s\n", typestr, start_time_str, end_time_str);
            cursor = showStr + str_length;
        }

        str_length += sprintf(cursor, " exit\n");
        cursor = showStr + str_length;
    }
    
	if (0 == npd_poe_cfg_get.poe_enable)
	{
		str_length += sprintf(cursor, "poe disable\n");
		cursor = showStr + str_length;
	}
	else
	{
		ret = dbtable_array_get(poe_pse_index, 0, &poe_pse);
		if (0 == ret)
		{
			if (poe_pse.mode != POE_PSE_AUTO_AND_DYNAMIC)
			{
				if (poe_pse.mode == POE_PSE_AUTO_AND_STATIC)
				{
					str_length += sprintf(cursor, "poe power-management static \n");
					cursor = showStr + str_length;
				}
				else if (poe_pse.mode == POE_PSE_MANUAL_AND_STATIC)
				{
					str_length += sprintf(cursor, "poe power-management static \npoe power-management manual \n");
					cursor = showStr + str_length;
				}
				else if (poe_pse.mode == POE_PSE_MANUAL_AND_DYNAMIC)
				{
					str_length += sprintf(cursor, "poe power-management manual \n");
					cursor = showStr + str_length;
				}
				
			}
		}
		while(1)
		{
			if (0 == poe_intf.netif_index)
			{
				ret = dbtable_hash_head(poe_intfs_hash, &poe_intf, &poe_intf, NULL);	
			}
			else
			{
				ret = dbtable_hash_next(poe_intfs_hash, &poe_intf, &poe_intf, NULL);
			}
			
			if (0 != ret)
			{
				*cursor = '\0';
				break;
			}
				

			npd_netif_index_to_user_fullname(poe_intf.netif_index, eth_name);

			configed_str = cursor;
			configed_length = str_length;
			eth_name_length = sprintf(cursor, "interface %s\n", eth_name);
			str_length += eth_name_length;
			cursor = showStr + str_length;
    		if (0 == poe_intf.admin)
    		{
    			str_length += sprintf(cursor, " poe disable\n");
				cursor = showStr + str_length;
    		}
			else
			{
                if(1 == poe_intf.time.poe_based_time_valid)
                {
        			str_length += sprintf(cursor, " poe enable poe-time-range %d\n", poe_intf.time.time_range_index);
    				cursor = showStr + str_length;
                }
				if (poe_intf.poe_mode != POE_SIGNAL)
				{
					if (poe_intf.poe_mode == POE_SPARE)
					{
						str_length += sprintf(cursor, " poe mode spare\n");
						cursor = showStr + str_length;
					}
				}
				if (poe_intf.priority != POE_PRIORITY_NONE)
				{
					if ((poe_intf.priority <= POE_PRIORITY_CRITICAL) && (poe_intf.priority >= POE_PRIORITY_HIGH))
					{
						if (poe_intf.priority == POE_PRIORITY_HIGH)
							str_length += sprintf(cursor, " poe priority high\n");
						else
							str_length += sprintf(cursor, " poe priority critical\n");
						cursor = showStr + str_length;
					}
				}
				if (poe_intf.max_power_admin_flag == 1)
				{
					unsigned char pd_class = 0;
					if (poe_intf.max_power <= 40)
						pd_class = 1;
					else if (poe_intf.max_power <= 70)
						pd_class = 2;
					else if (poe_intf.max_power <= 154)
						pd_class = 0;
					else
						pd_class = 4;
					str_length += sprintf(cursor, " poe max-power class %d\n", pd_class);
					cursor = showStr + str_length;
				}
			}
			if (str_length == (configed_length + eth_name_length))
			{
				cursor = configed_str;
				str_length = configed_length;
			}
			else
			{
				str_length += sprintf(cursor, " exit\n");
				cursor = showStr + str_length;
			}
		}
	}


	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &showStr);	

retcode:
	free(showStr);
	showStr = NULL;

	return reply;
	
}



void* npd_poe_poll_thread(void)
{	
	npd_init_tell_whoami("poeSock",0);

	int ret = 0;
	npd_syslog_dbg("Enter npd_poe_poll_thread .\n");
	while(1)
	{
		npd_poe_cfg_t npd_poe_cfg_get;
        sleep(1);

        if(!npd_startup_end)
            continue;
		ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
		if(ret != 0)
		{
	     	continue;
		}
		if(0 == npd_poe_cfg_get.poe_enable)
			continue;

		unsigned char poePort = 0;
		unsigned char portSta = 0;
		unsigned char errType = 0;
		unsigned char pdClass = 0;
		unsigned char pdType = 0;
		int ret = 0;
		poe_intf_db_t poe_intf = {0};
        unsigned char devNum, portNum;

		/* Iterate all poe interface */
		while(1)
		{
			if (0 == poe_intf.netif_index)
			{
				ret = dbtable_hash_head(poe_intfs_hash, &poe_intf, &poe_intf, NULL);	
			}
			else
			{
				ret = dbtable_hash_next(poe_intfs_hash, &poe_intf, &poe_intf, NULL);
			}
			
			if (0 != ret)
				break;

            ret = npd_get_devport_by_global_index(poe_intf.netif_index, &devNum, &portNum);
            if(NPD_SUCCESS != ret)
                continue;
            
			ret = npd_get_poeport_by_global_index(poe_intf.netif_index, &poePort);
			if (0 != ret)
				continue;

			nam_poe_port_status_get(poePort, &portSta, &errType, &pdClass, &pdType);
			if (poe_intf.detect_status != portSta)
			{
            	int total_power_allocate = 0;
            	int available_power = 0;
            	unsigned char powerMode = 0;
            	unsigned char violaType = 0;
            	int maxPower = 0;
            	unsigned char portPriority = 0;
            	int pse_total_power = 0;
            	poe_intf_db_t poe_dbg_intf = {0};
            	poe_port_t poe_port = {0};
            	int  portStatus = 0;
            	unsigned int portSpeed = 0;
            	int voltage = 0;
            	int poeCurrent = 0;
            	int temperature = 0;
            	int power = 0;
            	float poe_intf_temperature = 0.0;
                unsigned int netif_index = poe_intf.netif_index;
            
            	char port_string[30] = {0};
                parse_eth_index_to_name(poe_intf.netif_index, (unsigned char*)port_string);
            	
            	nam_poe_port_status_get(poePort, &portSta, &errType, &pdClass, &pdType);
            	nam_poe_total_power_allocate_get(&total_power_allocate, &available_power);
            	nam_poe_port_extended_config_get(poePort, &powerMode, &violaType, &maxPower, &portPriority);
            	nam_poe_port_measure_get(poePort, &voltage, &poeCurrent, &temperature, &power);
            	pse_total_power = MODULE_PSE_TOTAL_POWER_ON_SLOT_INDEX(0);
            	poe_port.port = poePort;
            	poe_intf_temperature = temperature * 0.01;
            
            	poe_dbg_intf.netif_index = netif_index;
            	poe_dbg_intf.detect_status = portSta;
            	poe_dbg_intf.operate_status = errType;
            	poe_dbg_intf.temperature = temperature;
            	poe_dbg_intf.pd_class = pdClass;
            	poe_dbg_intf.current = poeCurrent;
            	poe_dbg_intf.voltage = voltage;
            	poe_dbg_intf.power_user = power;
            		
    			if (poe_intf.detect_status != portSta)
    			{
    				npd_syslog_poe_dbg(POE_LOG_NONE, poe_intf.detect_status, poe_dbg_intf);
    				
    				if (portSta == POE_DETECT_DELIVERING_POWER)
    				{
    					if (poe_intf.priority == POE_PRIORITY_NONE)
    						poe_intf.priority = POE_PRIORITY_LOW;
    					if ((poe_intf.power_threshold_type == POE_THRESHOLD_CLASS_BASE) && (poe_intf.max_power_admin_flag == 0))
    					{
    						switch (pdClass)
    						{
    							case 0:
    								poe_intf.max_power = CLASS0_POWER;
    								break;
    							case 1:
    								poe_intf.max_power = CLASS1_POWER;
    								break;
    							case 2:
    								poe_intf.max_power = CLASS2_POWER;
    								break;
    							case 3:
    								poe_intf.max_power = CLASS3_POWER;
    								break;
    							case 4:
    								poe_intf.max_power = CLASS4_POWER;
    								break;
    							default:
    								poe_intf.max_power = CLASS4_POWER;
    								break;
    
    						}
    	
    						ret = dbtable_array_get(npd_poe_cfg_index, 0, &npd_poe_cfg_get);
    						if (0 != ret)
    							return;
    						/*剩余功率不足以支持端口功率 提升到 指定等级功率*/
    						if ((poe_intf.max_power > maxPower) && ((poe_intf.max_power - maxPower)  > (pse_total_power - total_power_allocate)))
    						{                                          
    							nam_poe_port_poe_endis_set(poePort, 0);
    							nam_poe_maxpower_threshold_set(poePort, poe_intf.max_power);
    							nam_poe_port_poe_endis_set(poePort, 1);
    						}
    					
    					//printf("poePort %d, power_threshold_type %d, pdClass %d, maxPower %d.\n", poePort, poe_intf.power_threshold_type, pdClass, poe_intf.max_power);
    					}	
    				}
    				else
    				{
    					if (poe_intf.priority == POE_PRIORITY_LOW)
    						poe_intf.priority = POE_PRIORITY_NONE;
    				}
    				switch (portSta)
    				{
    					case POE_DETECT_DISABLE:
    						poe_intf.operate_status = POE_POWER_OFF;
    						break;
    					case POE_DETECT_DELIVERING_POWER:
    						poe_intf.operate_status = POE_POWER_ON;
    						break;
    					case POE_DETECT_FAULT:
    						poe_intf.operate_status = errType + 1;
    						break;
    					case POE_DETECT_OTHER_FAULT:
    						poe_intf.operate_status = errType + 1;
    						break;
    					default:
    						poe_intf.operate_status = POE_POWER_OFF;
    						break;	
    				}
    				ret = npd_get_port_link_status(netif_index, &portStatus);
    
            		if (NPD_SUCCESS == ret)
            		{
                   	 	if (portStatus == ETH_ATTR_ON)
                    	{
                    		ret = nam_get_port_speed(netif_index, &portSpeed);
    						if (0 == ret)
    						{
    							if (portSpeed >= PORT_SPEED_1000_E)
    							{
    								poe_port.speed = POE_GE;
    							}
    							else
    							{
    								poe_port.speed = POE_100M;
    							}
    							if (portSta == POE_DETECT_DELIVERING_POWER)
    							{
    								poe_port.state = POE_NORMAL;
    							}
    							else if ((portSta == POE_DETECT_FAULT) || (portSta == POE_DETECT_OTHER_FAULT))
    							{
    								poe_port.state = POE_ALARM;
    							}
    							else
    							{
    								poe_port.state = POE_OFF;
    							}
    						}
                    	}
                    	else
                    	{
                    		poe_port.port = poePort;
    						poe_port.speed = POE_LINK_DOWN;
    						poe_port.state = POE_LINKDOWN;
                    	}
    					ret = nbm_poe_led(&poe_port);
    					if (ret != 0)
    					{
    						npd_syslog_dbg("write register error!.\n");
    					}
           	 		}
    				else
    				{
    					npd_syslog_dbg("Get port link status failed.\n");
    				}
    				poe_intf.detect_status = portSta;
    				dbtable_hash_update(poe_intfs_hash, &poe_intf, &poe_intf);
    			}
			}
			
		}
	}
}

#ifdef __cplusplus
}
#endif
#endif
