/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_product.c
*
*
* CREATOR:
*		chenb@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for product related routine.
*
* DATE:
*		02/21/2008	
*UPDATE:
*06/13/2010              chengjun@autelan.com          Using CONNECTION struct.
*11/09/2010              zhengzw@autelan.com           Add CPLD asigning and CPLD interrupt handler.
*  FILE REVISION NUMBER:
*  		$Revision: 1.65 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "board/ts_product_feature.h"
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"
#include "chasm_manage_proto.h"

#include "nbm/npd_cplddef.h"
#include "nbm/nbm_cpld.h"
#include "nbm/nbm_api.h"

product_fix_param_t **product_type_array = NULL;
board_fix_param_t **module_basic_info = NULL;
board_spec_fix_param_t **module_spec_info = NULL;
product_conn_type_t **product_conn_type = NULL;
board_conn_type_t **board_conn_type = NULL;
board_conn_type_t **board_conn_fullmesh_type = NULL;
product_param_t productinfo = {0};

int	npd_env_monitor_flag = 0; //
/* NPD startup state*/
int npd_startup_end = 0;


/* NPD startup state file descriptor*/
int aw_state_fd = -1;

/* file to save system startup state */
#define NPD_SYSTEM_STARTUP_STATE_PATH	"/var/run/aw.state"

/* file to save NPD initialization stage: this is used by rc script to wait for npd initialization done. */
#define NPD_INIT_STAGE_FILE_PATH	"/var/run/asic.state"

/* file to save all npd thread pid */
#define NPD_ALL_THREAD_PID_PATH	"/var/run/npd_all.pid"

#define MANUFACTURE_TEST_REG_READ_ERROR 0x40
#define MANUFACTURE_TEST_XGE_ERROR 0x80
#define MANUFACTURE_TEST_XAUI_MUX_ERROR 0x100

int register_product_param_array(product_fix_param_t **array)
{
	product_type_array = array;
    return NPD_SUCCESS;
}

int register_board_param_array(board_fix_param_t **array)
{
    module_basic_info = array;
    return NPD_SUCCESS;
}

int register_board_spec_param_array(board_spec_fix_param_t **array)
{
    module_spec_info = array;
    return NPD_SUCCESS;
}

int register_product_conn_array(product_conn_type_t **array)
{
    product_conn_type = array;
    return NPD_SUCCESS;
}

int register_board_conn_array(board_conn_type_t **array)
{
    board_conn_type = array;
    return NPD_SUCCESS;
}

int register_board_conn_fullmesh_array(board_conn_type_t **array)
{
    board_conn_fullmesh_type = array;
    return NPD_SUCCESS;
}


int init_conn_info(unsigned long board_type, unsigned long product_type)
{
    int i;

    for( i = 0; i < PRODUCT_MAX_NUM; i++)
    {
        if(NULL == product_conn_type[i])
            continue;
        if((*product_conn_type[i]).product_type == SYS_PRODUCT_TYPE)
            backplane_type = product_conn_type[i];
    }
    if(NULL == backplane_type)
    {
        npd_syslog_err("The product connection type array defination is wrong\r\n");
        return -1;
    }

    if(backplane_type->chassis_topo == FULL_MESH)
    {
        board_conn_type = board_conn_fullmesh_type;
    }
    
    for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
    {
        if(NULL == board_conn_type[i])
            continue;
        if((*board_conn_type[i]).board_type == SYS_LOCAL_MODULE_TYPE)
            local_board_conn_type = board_conn_type[i];
    }
    if(NULL == local_board_conn_type)
    {
        npd_syslog_err("The board connection type array defination is wrong\n");
        return -1;
            
    }


    return 0;

}

unsigned long device_producttype_get( void )
{
    unsigned long code = PRODUCT_ID;
    int i;	

    for(i = 0; i < PRODUCT_MAX_NUM; i ++)
    {
        if(NULL == product_type_array[i])
            continue;
        if(code == (*product_type_array[i]).product_code)
        {
            snros_system_param = product_type_array[i];
            productinfo.fix_param = snros_system_param;
            npd_syslog_dbg("The product hw code is %x\n", PRODUCT_ID);
            npd_syslog_dbg("The product type is %x\n", SYS_PRODUCT_TYPE);
            npd_syslog_dbg("The product in %d pos of array\n", i);
                
            return (*product_type_array[i]).product_type;
        }
    }

    return PRODUCT_UNKNOWN;
}

unsigned long device_moduletype_get()
{
  int i = 0;
  int board_type = LOCAL_MODULE_ID;
  
  for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
  {
      if(NULL == module_basic_info[i])
        continue;
      //if(hwcode == (*module_basic_info[i]).board_code)
      if(board_type == (*module_basic_info[i]).board_type)
      {
        snros_local_board = module_basic_info[i];
        snros_local_board_spec = module_spec_info[i];
        npd_syslog_dbg("The board hw code is %x\n", (*module_basic_info[i]).board_code);
        npd_syslog_dbg("The board type is %x\n", SYS_LOCAL_MODULE_TYPE);
        npd_syslog_dbg("The board in %d pos of array\n", i);
        return (*module_basic_info[i]).board_type;
      }
  }

  //npd_syslog_err("Can not get board type based on module type code %d\r\n", hwcode);
  return PPAL_BOARD_TYPE_NONE;

}

unsigned long device_submoduletype_get( unsigned long hwcode )
{
  int i = 0;

  for(i = 0; i < PPAL_BOARD_TYPE_MAX; i++)
  {
      if(NULL == module_basic_info[i])
        continue;
      if(hwcode == (*module_basic_info[i]).board_code)
      {
        return (*module_basic_info[i]).board_type;
      }
  }
  npd_syslog_err("Can not get board type based on module type code %d\r\n", hwcode);
  return PPAL_BOARD_TYPE_NONE;
}

void device_product_reset(unsigned long product_type)
{
    int i;	

    if(snros_system_param)
    {
        if(SYS_PRODUCT_TYPE == product_type)
            return;
        for(i = 0; i < SYS_CHASSIS_SLOTNUM; i++)
        {
            /*keep localmoduleinfo*/
            if(localmoduleinfo == chassis_slots[i])
                continue;
            if(chassis_slots&&chassis_slots[i])
            {
                free(chassis_slots[i]);
                chassis_slots[i] = NULL;
            }
        }
    }
    

    for(i = 0; i < PRODUCT_MAX_NUM; i ++)
    {
        if(NULL == product_type_array[i])
            continue;
        if(product_type == (*product_type_array[i]).product_type)
        {
            snros_system_param = product_type_array[i];
        }
    }
}

void device_product_reset_sn(char* serial_no)
{
    int i;	

    npd_syslog_dbg("reset product type\n");
    for(i = 0; i < PRODUCT_MAX_NUM; i ++)
    {
        if(NULL == product_type_array[i])
            continue;
        if(0 == strcmp((*product_type_array[i]).serial_no, serial_no))
        {
            if(SYS_PRODUCT_TYPE == (*product_type_array[i]).product_type)
                return;
            snros_system_param = product_type_array[i];
        }
    }
}

void device_product_reset_admin(void)
{			
	char cmd[100]  = {0};
	sprintf(cmd, "echo %s > /var/run/admin_username", 
		productinfo.sys_info.built_in_admin_username);
	system(cmd);
	
	sprintf(cmd, "echo %s > /var/run/admin_passwd", 
		productinfo.sys_info.built_in_admin_passwd);
	system(cmd);	
}

unsigned char* device_product_type2name( unsigned long product_type )
{
    int i;	
    for(i = 0; i < PRODUCT_MAX_NUM; i ++)
    {
       if(NULL == product_type_array[i])
            continue;
       if(product_type == (*product_type_array[i]).product_type)
            return (unsigned char*)((*product_type_array[i]).product_short_name);
    }

    return NULL;
}

char *product_id_str
(
	unsigned int productId
)
{
    int i;	
    for(i = 0; i < PRODUCT_MAX_NUM; i ++)
    {
        if(NULL == product_type_array[i])
            continue;
        if(productId == (*product_type_array[i]).product_type)
            return (*product_type_array[i]).product_short_name;
    }

    return NULL;
}

unsigned int npd_query_product_id(void) {
	return PRODUCT_ID;
}

unsigned int npd_query_hw_version(void) {
	return productinfo.local_module_hw_version;
}

char* npd_query_product_name(void) {
	return productinfo.fix_param->product_name;
}

#if 1
void npd_reset_sysinfo(struct product_sysinfo_s * sys_info)
{
	if (sys_info->sn != NULL)
	{
		free(sys_info->sn);
		sys_info->sn = NULL;
	}

	if (sys_info->name != NULL)
	{
		free(sys_info->name);
		sys_info->name = NULL;
	}

	if (sys_info->basemac != NULL)
	{
		free(sys_info->basemac);
		sys_info->basemac = NULL;
	}

	if (sys_info->sw_name != NULL)
	{
		free(sys_info->sw_name);
		sys_info->sw_name = NULL;
	}

	if (sys_info->enterprise_name != NULL)
	{
		free(sys_info->enterprise_name);
		sys_info->enterprise_name = NULL;
	}

	if (sys_info->enterprise_snmp_oid != NULL)
	{
		free(sys_info->enterprise_snmp_oid);
		sys_info->enterprise_snmp_oid = NULL;
	}
	
	if (sys_info->snmp_sys_oid != NULL)
	{
		free(sys_info->snmp_sys_oid);
		sys_info->snmp_sys_oid = NULL;
	}

	if (sys_info->built_in_admin_username != NULL)
	{
		free(sys_info->built_in_admin_username);
		sys_info->built_in_admin_username = NULL;
	}

	if (sys_info->built_in_admin_passwd != NULL)
	{
		free(sys_info->built_in_admin_passwd);
		sys_info->built_in_admin_passwd = NULL;
	}

   (*snros_system_param->product_man_param_get)( &productinfo.sys_info);
	
}
#endif 
struct product_sysinfo_s *npd_query_sysinfo(void) {
	return &(productinfo.sys_info);
}

char *npd_query_product_serialno(void) {
	return productinfo.sys_info.sn;
}

char *npd_query_product_basemac(void) {
	return productinfo.sys_info.basemac;
}

char *npd_query_sw_name(void) {
	return productinfo.sys_info.sw_name;
}

char *npd_query_enterprise_name(void) {
	return productinfo.sys_info.enterprise_name;
}

char *npd_query_enterprise_snmp_oid(void) {
	return productinfo.sys_info.enterprise_snmp_oid;
}

char *npd_query_snmp_sysid(void) {
	return productinfo.sys_info.snmp_sys_oid;
}
/**********************************************************************************
 * npd_system_verify_basemac
 * 
 * Verify if the given mac address is the same as system base mac address
 *
 *	INPUT:
 *		macAddr - mac address will compare to system base mac address
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1 - if parameters illegall.
 *		-2 - input parameter to long.
 *		0 - if the given mac address is not the same as system base mac address.
 *		1 - if the given mac address is the same as system base mac address.
 *
 *	NOTATION:
 *		input parameter macAddr is a ASCII code formatted MAC address, such as 
 *		"001122334455" stands for mac address 00:11:22:33:44:55 or 00-11-22-33-44-55
 *		
 **********************************************************************************/
int npd_system_verify_basemac
(
	char *macAddr
)
{
    return (0 == memcmp(macAddr, PRODUCT_MAC_ADDRESS, 6));
}

/**********************************************************************************
 * npd_system_get_basemac
 * 
 * 	Get system mac address in Hex format
 *
 *	INPUT:
 *		size - macAddr buffer size
 *	
 *	OUTPUT:
 *		macAddr - mac address will be returned back
 *
 * 	RETURN:
 *		-1 - if mac address buffer size too small.
 *		-2 - illegal character found.
 *		0 - if no error occur
 *
 *	NOTATION:
 *		system mac address is a ASCII code formatted MAC address, such as 
 *		"001122334455" stands for mac address 00:11:22:33:44:55 or 00-11-22-33-44-55
 *		
 **********************************************************************************/
int npd_system_get_basemac
(
    unsigned char *macAddr,
    unsigned int  size
)
{
    memcpy(macAddr, SYS_PRODUCT_BASEMAC, 6);

	return 0;
}


/*

cmd used to test this method
dbus-send --system --dest=aw.npd --type=method_call --print-reply=literal /aw/npd aw.npd.show_ver

 arg lists for method NPD_DBUS_INTERFACE_METHOD_VER
  in arg list:
	NONE
  out arg list:  // in the order as they are appended in the dbus message.
	uint32 product_id	// unsigned int of product type. uniquely identify a product, defined in npd_sysdef.h ,
	uint32 sw_version      // bitmap definition in npd_sysdef.h
  	string product_name  // backplane info for chassis product, board info for box product
	string product_base_mac_addr  // 12 char of mac address  with no : or - spliter.
  	string product_serial_number  // 32 bytes string
	string sw_name		// software description string
 

 */

DBusMessage * npd_dbus_interface_show_ver(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	unsigned int product_id = npd_query_product_id();
	unsigned int sw_version = npd_query_sw_version(SYS_LOCAL_MODULE_TYPE);
	unsigned int hw_version = npd_query_hw_version();
	int op_ret = 0;
	
	struct product_sysinfo_s *p_sysinfo;
	char * boot_vername;
	char * base_mac;
	char * temp_src;
	char * temp_dst;
	int len;
	int i;

	boot_vername = malloc(128);
	if (NULL == boot_vername)
	{
		syslog_ax_product_err("get boot version name error.\n");
		//boot_vername = "Dummy Build.";
		return NULL;
	}
	op_ret = nbm_get_boot_version_name(boot_vername);
	if (op_ret != 0)
	{
		syslog_ax_product_err("get boot version name error.\n");
		strcpy(boot_vername, "Dummy Build");
	}

	p_sysinfo = npd_query_sysinfo();


	syslog_ax_product_dbg("sw version %#0x\n",sw_version);
	syslog_ax_product_dbg("hw version %#0x\n",hw_version);
	
	
	syslog_ax_product_dbg("Query sysinfo product name %s sn %s swname %s basemac %s\n",
			   p_sysinfo->name,
			   p_sysinfo->sn,
			   p_sysinfo->sw_name,
			   p_sysinfo->basemac);

	base_mac = malloc(20);
	if (NULL == base_mac) /* */
	{
		syslog_ax_product_err("get boot version name error.\n");
		free(boot_vername);
		return NULL;		
	}
	memset(base_mac, 0, 20);
	temp_src = p_sysinfo->basemac;
	temp_dst = base_mac;
	len = strlen(temp_src);
	for (i = 0; i < len; i++)	/* */
	{
		if (i%2 == 0 && i != 0)
		{
			*temp_dst++ = '-';
		}
		*temp_dst++ = *temp_src++;
	}
	*temp_dst = 0;
	syslog_ax_product_dbg("boot version name is %s.\n", boot_vername);
	
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&product_id,
							 DBUS_TYPE_UINT32,&sw_version,
							 DBUS_TYPE_UINT32,&hw_version,									 
							 DBUS_TYPE_STRING,&(p_sysinfo->name),
							 DBUS_TYPE_STRING,&(base_mac),
							 DBUS_TYPE_STRING,&(p_sysinfo->sn),
							 DBUS_TYPE_STRING,&(p_sysinfo->sw_name),
							 DBUS_TYPE_STRING,&(boot_vername),  
							 DBUS_TYPE_INVALID);
	if (NULL != boot_vername)
	{
		free(boot_vername);
		boot_vername = NULL;
	}

	if (NULL != base_mac)
	{
		free(base_mac);
		base_mac = NULL;		
	}
	syslog_ax_product_dbg("Show system info done.\n");
	
	return reply;
}


DBusMessage * npd_system_shut_down_enable(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	
	unsigned short	isenable  = 0;
	unsigned int	ret = NPD_DBUS_SUCCESS;
	
	DBusError err;
	dbus_error_init(&err);
	if (!(dbus_message_get_args ( msg, &err,
									DBUS_TYPE_UINT16,&isenable,
									DBUS_TYPE_INVALID))) {
		syslog_ax_product_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
				syslog_ax_product_err("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
		ret = NPD_DBUS_ERROR;
	}
    /*todo*/
    
	if(ret != 0){
		ret = NPD_DBUS_ERROR;
		syslog_ax_product_err("set system alarm state err! ");
		}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	return reply;
	
}


/*******************************************************************************
* npd_check_system_startup_state
*
* DESCRIPTION:
*      Check if system has started up correctly.
*
* INPUTS:
*	  None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*	
*
* COMMENTS:
*       
*
*******************************************************************************/
void npd_check_system_startup_state
(
	void
)
{
	unsigned char buf[4] = {0};
	int ret = 0;
	static unsigned int counter = 0;
	/*s tell my thread id*/
	npd_init_tell_whoami("SysWaitSfpPoll",0);

	while(1) {
	  if(aw_state_fd < 0) {
		  aw_state_fd = open(NPD_SYSTEM_STARTUP_STATE_PATH,O_RDONLY);
		  if(aw_state_fd < 0) {
			syslog_ax_product_dbg("open product startup state file %s error\n",NPD_SYSTEM_STARTUP_STATE_PATH);
		  }
	  }
	  else {
		  read(aw_state_fd,buf,1);
		  if('1' == buf[0]) {
		  	/* enable asic interrupts */
			nam_board_after_enable();

			/* set system init done:LED RUN blinking */
			nbm_set_system_init_stage(1);
			close(aw_state_fd);
			

			/* enable hardware watchdog */
			ret = nbm_hardware_watchdog_control_set(SYSTEM_HARDWARE_WATCHDOG_ENABLE);
			if(ret) {
				syslog_ax_product_warning("enable hardware watchdog error %d!", ret);
			}

		    /* sleep for a while */
			sleep(1);
			break;
		  }
	  }
	  
	  /* fillup hardware watchdog */
	  ret = nbm_hardware_watchdog_fillup();
	  if(ret && !counter) {
		  syslog_ax_product_warning("hardware watchdog fillup failed!\n");
	  }
	  /*
	   *  scan ethernet port link status
	   */
	  /*npd_scan_eth_ports_link_status();*/
	  
	  sleep(1);
	}

	/* do SFP port status polling*/
	while(1) {
		/* fillup hardware watchdog */
		ret = nbm_hardware_watchdog_fillup();
		if(ret && !counter) {
		   syslog_ax_product_warning("hardware watchdog fillup failed!\n");
		}
		/* wait for a while	*/	
		sleep(1);
	}
	
}

/*******************************************************************************
* npd_check_system_attack
*
* DESCRIPTION:
*      Check if system has started up correctly.
*
* INPUTS:
*	  None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*	
*
* COMMENTS:
*       
*
*******************************************************************************/
void npd_check_system_attack
(
	void
)
{
    struct sched_param param = {0};
    int policy;

	/*s tell my thread id*/
	npd_init_tell_whoami("SysCheckAttack",0);


    if (pthread_getschedparam(pthread_self(), &policy, &param) == 0) {
        /* Interrupt thread uses SCHED_RR and should be left alone */
        if (policy != SCHED_RR) {
            param.sched_priority = 50;
            pthread_setschedparam(pthread_self(), SCHED_RR, &param);
        }
    }
    npd_netif_attack_init();
    
	while(1)
	{
        npd_netif_attack_timer();

		sleep(1);
	}
}


void npd_init_packet_socket
( 
	void
)
{
	unsigned long status = 0;
	
	status = npd_packet_rx_adapter_init();
	if(status) {
		syslog_ax_product_err("init sockets for packet rx error %d\r\n", status);
	}

	status = nam_packet_tx_adapter_init();
	if(status) {
		syslog_ax_product_err("init sockets for packet tx error %d\r\n", status);
	}

	return;
}

/*******************************************************************************
* npd_init_tell_whoami
*
* DESCRIPTION:
*      This function is used by each thread to tell its name and pid to NPD_ALL_THREAD_PID_PATH
*
* INPUTS:
*	  tName - thread name.
*	  lastTeller - is this the last teller or not, pid file should be closed if true.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*	  None.
*
* COMMENTS:
*       
*
*******************************************************************************/
void npd_init_tell_whoami
(
	char *tName,
	unsigned char lastTeller
)
{	

	app_module_inst_set(tName, getpid());
	
	return;
}

/*******************************************************************************
* npd_init_tell_stage_end
*
* DESCRIPTION:
*      This function is used by NPD to tell rc script loader about initialization process done.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*	  None.
*
* COMMENTS:
*       
*
*******************************************************************************/
void npd_init_tell_stage_end
(
	void
)
{
	/* NPD initialization stage file descriptor */
	int npd_stage_fd = -1;
	unsigned char buf[16] = {0};

	npd_stage_fd = open(NPD_INIT_STAGE_FILE_PATH, O_CREAT|O_RDWR|O_TRUNC);
	if(npd_stage_fd < 0) {
		syslog_ax_product_dbg("open file %s error when tell init stage end\n",	\
				NPD_INIT_STAGE_FILE_PATH);
		return;
	}

	sprintf((char*)buf,"%d",1);
	
	write(npd_stage_fd,buf,strlen(buf));

	close(npd_stage_fd);	
	syslog_ax_product_dbg("npd tell init stage to %s ok\n", NPD_INIT_STAGE_FILE_PATH);
	return;
}

/****************************************************
 *	Description:
 *		check port's target is SPI or not
 *		SPI path with uplink   (1,26) on AX7 and
 *							(0,24) on AX5612i and
 *							(0,0) on AX5612
 *
 *	Input:
 *		unsigned char -device num of eth-port
 *		unsigned char -port num of eth-port
 *
 *	Output:
 *		NULL
 *
 *	ReturnCode:
 *		NPD_TRUE		- success
 *		NPD_FALSE		- fail
 *		
 ****************************************************/
unsigned int npd_product_check_spi_port
(
	unsigned char devNum,
	unsigned char portNum
)
{
	unsigned int result = NPD_FALSE;

	syslog_ax_product_dbg("asic port(%d, %d)\n", 
							devNum, portNum);
	
	/* target is SPI */
	switch(PRODUCT_ID) {
		default:
			result =  NPD_FALSE;
			break;
	}

	syslog_ax_product_dbg("asic port(%d,%d) is %son HSC path\n", 
							devNum, portNum, result ? "":"not ");
	return result;
}

#ifdef HAVE_POWER_MONITOR
int device_power_supply_number_check(unsigned int ps_no)
{
	int ps_index = 0;

	ps_index = ps_no - 1;
	if (ps_index < 0 || ps_index >= SYS_CHASSIS_POWER_NUM)
	{
		return DEVICE_RETURN_CODE_NO_SUCH_POWER_UNIT;
	}
	return DEVICE_RETURN_CODE_ERR_NONE;
}

/**********************************************************************************
 * npd_dbus_device_show_power_supply
 *
 * 	Give the specified power supply information .
 *
 *	INPUT:
 *		ps_no -  power supply number , assume the number is normal range
 *	
 *	OUTPUT:
 *		inserted - power supply inserted info
 		ps_type - power supply name
 		ps_state - inserted power work state
 *
 *	RETURN:
 *		0 - no hardware watchdog on this product
 *		1 - hardware watchdog is deployed on this product
 *
 **********************************************************************************/


DBusMessage * npd_dbus_device_show_power_supply(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;	
	DBusMessageIter	 iter;
	unsigned int ret = 0;
	unsigned int ps_no = 0;
	unsigned int ps_index = 0;
	unsigned int inserted = 1;
	int ps_state = 0;
	char * ps_type = "";
	power_supply_man_param_t* ps_param = NULL ;
	int op_ret = 0;
	int state = 0;

	npd_syslog_dbg("get power supply information!\n");
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&ps_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = device_power_supply_number_check(ps_no);
	if (ret != DEVICE_RETURN_CODE_ERR_NONE)
	{
		npd_syslog_err("power unit %d is not legal!\n", ps_no);
		goto retcode;
	}

	if (productinfo.power_supply_param == NULL)
	{
		npd_syslog_err("power supply param not initialize!\n");
		ret = DEVICE_RETURN_CODE_NO_SUCH_POWER_UNIT;
		goto retcode;
	}

	npd_syslog_dbg("Get power supply unit %d information \n", ps_no);
	ps_index = ps_no - 1;
	ps_param = &productinfo.power_supply_param[ps_index];
	if (ps_param == NULL)
	{
		npd_syslog_err("power supply unit %d not initialize!\n", ps_no);
		ret = DEVICE_RETURN_CODE_NO_SUCH_POWER_UNIT;
		goto retcode;
	}
	op_ret = nbm_get_power_present(ps_index, &state);
	if (op_ret != 0)
	{
		ret = DEVICE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	if (POWER_INSERT == state )
	{
		inserted = 1;
	}
	else
	{
		inserted = 0;
	}

	if (inserted)
	{

		ps_type = ps_param->name;
		if (strlen(ps_param->name) == 0)    
		{
			npd_syslog_err("Get power index %d name error \n", ps_no);
			ret = DEVICE_RETURN_CODE_ERR_GENERAL;
			goto retcode;
		}
		npd_syslog_dbg("ps unit %d onlined.\n", ps_no);
		op_ret = nbm_get_power_state(ps_index, &ps_state);
		if (op_ret != 0)
		{
			npd_syslog_err("Get power supply unit %d state error \n", ps_no);
			ret = DEVICE_RETURN_CODE_ERR_GENERAL;
			goto retcode;
		}
	}
	else
	{
		npd_syslog_dbg("Get power supply unit %x is removed\n", ps_no);
	}
	ret = BOARD_RETURN_CODE_ERR_NONE;

retcode:
	
	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &inserted);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_STRING,
									 &ps_type);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &ps_state);


	return reply;
}


DBusMessage * npd_dbus_device_get_next_psunit(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret = 0;
	unsigned int ps_no = 0;

	DBusError err;

	
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&ps_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	npd_syslog_dbg("Get next ps no from input no %d!\n", ps_no);

	ps_no++; 	//get the next ps no;

	ret = device_power_supply_number_check(ps_no);

	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ps_no);

	return reply;
	
}

#endif

#ifdef HAVE_TEMPERATURE_MONITOR
DBusMessage * npd_dbus_show_temperature(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    unsigned int slotno = 0;
	int ready = 0;
	int temp_state = 0;
    temp_info_args temp_info = {0};
	int ret;
	int op_ret;

    DBusError err;
    dbus_error_init(&err);

	syslog_ax_product_dbg("show  temperature .\n" );
    if (!(dbus_message_get_args(msg, &err,
                                DBUS_TYPE_UINT32,&slotno,
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_product_err("Unable to get input args.\n ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_product_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }

	syslog_ax_product_dbg("check slotno %d legal.\n", slotno);
	ret = chasm_local_check(slotno);
    if (BOARD_RETURN_CODE_ERR_NONE != ret)
    {
		syslog_ax_product_err("no slotno %d .\n", slotno);
        ret = BOARD_RETURN_CODE_NO_SUCH_SLOT;
        goto retcode;
    }
	else
	{
		ret = DEVICE_RETURN_CODE_ERR_NONE;
	}

	if (SYS_LOCAL_MODULE_SLOTNO != slotno)
	{
		syslog_ax_product_err("no slotno %d .\n", slotno);
		
		ret = DEVICE_RETURN_CODE_SLOT_NOT_LOCAL;
		goto retcode;
	}


    if (SYS_LOCAL_MODULE_SLOTNO == slotno)
    {
		if (SYS_LOCAL_MODULE_REMOTE_RUNSTATE >= RMT_BOARD_READY &&
			SYS_LOCAL_MODULE_REMOTE_RUNSTATE < RMT_BOARD_REMOVING)
		{
		
			ready = 1;
		}
		else
		{
			syslog_ax_product_dbg("local module slotno %d not ready.\n",  slotno);		
			ready = 0;
			ret = DEVICE_RETURN_CODE_ERR_NONE;
			goto retcode;
		}

		syslog_ax_product_dbg("local module slotno %d .\n",  slotno);
		if (SYS_LOCAL_MODULE_TEMPER_COUNT <= 0)
		{
			syslog_ax_product_err("didn't have any temper.\n");
			ret = DEVICE_RETURN_CODE_ERR_NONE;
			temp_state = TEMP_NON_EXIST;
			goto retcode;
		}

		op_ret = nbm_get_temp_info(&temp_info);
		if (op_ret != 0)
		{
			ret = DEVICE_RETURN_CODE_ERR_NONE;
			temp_state = TEMP_NON_EXIST;
			goto retcode;
		}

		op_ret = nbm_get_temp_state(&temp_state);
		if (op_ret != 0)
		{
			ret = DEVICE_RETURN_CODE_ERR_NONE;
			temp_state = TEMP_NON_EXIST;
			goto retcode;
		}
    }
	
retcode:
    reply = dbus_message_new_method_return(msg);
	
    dbus_message_iter_init_append(reply, &iter);
	
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &(ready));	
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &(temp_state));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &(temp_info.core_temp)); /* */
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &(temp_info.core_high_limit));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &(temp_info.core_low_limit));
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &temp_info.module_temp);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &temp_info.module_high_limit);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &temp_info.module_low_limit);
    return reply;

}


DBusMessage * npd_dbus_config_temper_threshold(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
	int ret;
	int op_ret;
	char * type = NULL;
	int lower_value;
	int upper_value;
	temp_op_args temp_op = {0};
	unsigned int slotno = 0;
	int ready = 0;
    temp_info_args temp_info = {0};
	int temp_state = TEMP_NORMAL;

    DBusError err;
    dbus_error_init(&err);

	syslog_ax_product_dbg("config  temperature threshold .\n");
    if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &type,
                                DBUS_TYPE_UINT32,&slotno,
                                DBUS_TYPE_INT32,&lower_value,
                                DBUS_TYPE_INT32,&upper_value,                               
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_product_err("Unable to get input args.\n ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_product_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }

        return NULL;
    }
	
	if (type == NULL)
	{
		syslog_ax_product_err("set temp threshold type error. It is NULL\n");	
		ret = DEVICE_RETURN_CODE_ERR_GENERAL;
	}

	ret = chasm_local_check(slotno);
    if (BOARD_RETURN_CODE_ERR_NONE != ret)
    {
		syslog_ax_product_err("no slotno %d .\n", slotno);
        ret = BOARD_RETURN_CODE_NO_SUCH_SLOT;
        goto retcode;
    }
	else
	{
		ret = DEVICE_RETURN_CODE_ERR_NONE;
	}

	if (SYS_LOCAL_MODULE_SLOTNO != slotno)
	{
		ret = DEVICE_RETURN_CODE_SLOT_NOT_LOCAL;
		/* */
	}

    if (SYS_LOCAL_MODULE_SLOTNO == slotno)
    {
		if (SYS_LOCAL_MODULE_REMOTE_RUNSTATE >= RMT_BOARD_READY &&
			SYS_LOCAL_MODULE_REMOTE_RUNSTATE < RMT_BOARD_REMOVING)
		{
		
			ready = 1;
		}
		else
		{
			syslog_ax_product_dbg("local module slotno %d not ready.\n",slotno);		
			ready = 0;
			ret = DEVICE_RETURN_CODE_SLOT_NO_READY;
			goto retcode;
		}

		syslog_ax_product_dbg("local module slotno %d .\n",slotno);

		if (SYS_LOCAL_MODULE_TEMPER_COUNT <= 0)
		{
			syslog_ax_product_err("didn't have any temper.\n");
			ret = DEVICE_RETURN_CODE_ERR_NONE;
			temp_state = TEMP_NON_EXIST;
			goto retcode;
		}

		/* check for the temp unit is exist */
		op_ret = nbm_get_temp_info(&temp_info);
		if (op_ret != 0)
		{
			syslog_ax_product_err("get temp info error.\n");
			ret = DEVICE_RETURN_CODE_ERR_NONE;
			temp_state = TEMP_NON_EXIST;
			goto retcode;
		}



		if (0 == strcmp(type, "core"))
		{
			temp_op.op_type = TEMP_CORE_LOLIM;
			temp_op.value = lower_value;
			op_ret = nbm_set_temp_threshold(&temp_op);
			if (op_ret != 0)
			{
				syslog_ax_product_err("set temp core lower limit error.\n");
				ret = DEVICE_RETURN_CODE_TEMP_WRITE_ERROR;
				goto retcode;
			}

			temp_op.op_type = TEMP_CORE_HILIM;
			temp_op.value = upper_value;
			op_ret = nbm_set_temp_threshold(&temp_op);
			if (op_ret != 0)
			{
				syslog_ax_product_err("set temp core upper limit error.\n");
				ret = DEVICE_RETURN_CODE_TEMP_WRITE_ERROR;
				goto retcode;
			}			
		}
		else //surface
		{
			temp_op.op_type = TEMP_MODULE_LOLIM;
			temp_op.value = lower_value;
			op_ret = nbm_set_temp_threshold(&temp_op);
			if (op_ret != 0)
			{
				syslog_ax_product_err("set temp surface lower limit error.\n");
				ret = DEVICE_RETURN_CODE_TEMP_WRITE_ERROR;
				goto retcode;
			}

			temp_op.op_type = TEMP_MODULE_HILIM;
			temp_op.value = upper_value;
			op_ret = nbm_set_temp_threshold(&temp_op);
			if (op_ret != 0)
			{
				syslog_ax_product_err("set temp surface upper limit error.\n");
				ret = DEVICE_RETURN_CODE_TEMP_WRITE_ERROR;
				goto retcode;
			}
		}
    }
retcode:
    reply = dbus_message_new_method_return(msg);
	
    dbus_message_iter_init_append(reply, &iter);
	
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_INT32,
                                   &temp_state);

    return reply;


}
#endif
#ifdef HAVE_FAN_MONITOR
int device_fan_number_check(unsigned int fan_no)
{
	int fan_index = 0;

	fan_index = fan_no - 1;
	if (fan_index < 0 || fan_index >= SYS_CHASSIS_FAN_NUM)
	{
		return DEVICE_RETURN_CODE_NO_SUCH_FAN_UNIT;
	}
	return DEVICE_RETURN_CODE_ERR_NONE;
	
}

DBusMessage * npd_dbus_device_get_next_fan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusMessageIter	 iter;
	int ret = 0;
	unsigned int fan_no = 0;

	DBusError err;
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&fan_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}
	npd_syslog_dbg("Get next ps no from input no %d!\n", fan_no);

	fan_no++; 	//get the next ps no;

	ret = device_fan_number_check(fan_no);

	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);
	

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &fan_no);

	return reply;
	
}

/**********************************************************************************
 * npd_dbus_device_show_fan
 *
 * 	Give the specified fan information .
 *
 *	INPUT:
 *		fan_no -  power supply number , assume the number is normal range
 *	
 *	OUTPUT:
 *		inserted - fan inserted info
 *		state - inserted power work state
 *		speed - fan speed 
 *
 *	RETURN:
 *		0 - no hardware watchdog on this product
 *		1 - hardware watchdog is deployed on this product
 *
 **********************************************************************************/


DBusMessage * npd_dbus_device_show_fan(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
	DBusMessage* reply;
	DBusError err;	
	DBusMessageIter	 iter;
	unsigned int ret = 0;
	unsigned int fan_no = 0;
	unsigned int fan_index = 0;
	unsigned int inserted = 0;
	int fan_state = 0;
	int speed = 0;
	fan_man_param_t* fan_param = NULL ;
	int op_ret = 0;
	int state = 0;

	npd_syslog_dbg("get power supply information!\n");
	dbus_error_init(&err);

	if (!(dbus_message_get_args ( msg, &err,
		DBUS_TYPE_UINT32,&fan_no,
		DBUS_TYPE_INVALID))) 
	{
		npd_syslog_err("Unable to get input args\n ");
		if (dbus_error_is_set(&err)) 
		{
			syslog_ax_rstp_err("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return NULL;
	}

	ret = device_fan_number_check(fan_no);
	if (ret != DEVICE_RETURN_CODE_ERR_NONE)
	{
		npd_syslog_err("fan %d is not legal!\n", fan_no);
		goto retcode;
	}

	if (productinfo.fan_param== NULL)
	{
		npd_syslog_err("fan param not initialize!\n");
		ret = DEVICE_RETURN_CODE_NO_SUCH_POWER_UNIT;
		goto retcode;
	}

	npd_syslog_dbg("Get fan unit information %d\n", fan_no);
	fan_index = fan_no - 1;
	fan_param = &productinfo.fan_param[fan_index];
	if (fan_param == NULL)
	{
		npd_syslog_err("power supply unit %d not initialize!\n", fan_no);
		ret = DEVICE_RETURN_CODE_NO_SUCH_FAN_UNIT;
		goto retcode;
	}

	op_ret = nbm_get_fan_present(fan_index, &state);
	if (op_ret != 0)
	{
		ret = DEVICE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	if (FAN_INSERT == state )
	{
		inserted = 1;
	}
	else
	{
		inserted = 0;
	}

	if (inserted)
	{
		op_ret = nbm_get_fan_state(fan_index, &fan_state);
		if (op_ret != 0)
		{
			npd_syslog_err("Get power supply unit %d state error \n", fan_no);
			ret = DEVICE_RETURN_CODE_ERR_GENERAL;
			goto retcode;
		}

		op_ret = nbm_get_fan_speed(fan_index, &speed);
		if (op_ret != 0)
		{
			npd_syslog_err("Get fan %d state error \n", fan_no);
			ret = DEVICE_RETURN_CODE_ERR_GENERAL;
			goto retcode;
		}
	}
	else
	{
		npd_syslog_dbg("Get power supply unit %x is removed\n", fan_no);
	}
	ret = BOARD_RETURN_CODE_ERR_NONE;



retcode:
	
	reply = dbus_message_new_method_return(msg);

	
	dbus_message_iter_init_append (reply, &iter);
	
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);		
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &inserted);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &fan_state);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_INT32,
									 &speed);


	return reply;
}

DBusMessage * npd_dbus_device_config_fan_speed(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
	int ret;
	int op_ret;
	int speed_value;
	int state = 0;
	int fan_no = 0;
	int fan_index = 0;
	fan_man_param_t * fan_param = NULL ;
	
    DBusError err;
    dbus_error_init(&err);

	syslog_ax_product_dbg("config fan speed .\n");
    if (!(dbus_message_get_args(msg, &err,
	                           	DBUS_TYPE_UINT32,&fan_no,                               							
                                DBUS_TYPE_INT32,&speed_value,                               
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_product_err("Unable to get input args.\n ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_product_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }
	
	ret = device_fan_number_check(fan_no);
	if (ret != DEVICE_RETURN_CODE_ERR_NONE)
	{
		npd_syslog_err("fan %d is not legal!\n", fan_no);
		goto retcode;
	}

	if (productinfo.fan_param== NULL)
	{
		npd_syslog_err("fan param not initialize!\n");
		ret = DEVICE_RETURN_CODE_NO_SUCH_POWER_UNIT;
		goto retcode;
	}

	npd_syslog_dbg("Get fan unit information %d\n", fan_no);
	fan_index = fan_no - 1;
	fan_param = &productinfo.fan_param[fan_index];
	if (fan_param == NULL)
	{
		npd_syslog_err("power supply unit %d not initialize!\n", fan_no);
		ret = DEVICE_RETURN_CODE_NO_SUCH_FAN_UNIT;
		goto retcode;
	}

	op_ret = nbm_get_fan_present(fan_index, &state);
	if (op_ret != 0)
	{
		ret = DEVICE_RETURN_CODE_ERR_GENERAL;
		goto retcode;
	}
	if (FAN_INSERT == state )
	{
		op_ret = nbm_set_fan_speed(fan_index, speed_value);
		if (op_ret != 0)
		{
			npd_syslog_err("set fan %d speed error!\n", fan_no);
			
			ret = DEVICE_RETURN_CODE_ERR_GENERAL;
			goto retcode;
		}
		ret = DEVICE_RETURN_CODE_ERR_NONE;
		
	}
	else
	{
		ret = DEVICE_RETURN_CODE_FAN_NOT_PRESENT;
		goto retcode;
	}

retcode:
    reply = dbus_message_new_method_return(msg);
	
    dbus_message_iter_init_append(reply, &iter);
	
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    return reply;

}
#endif
#define MANUFACTURE_TEST_CREATE_VLAN_ERROR    0x1
#define MANUFACTURE_TEST_DELPORT_ERROR   0x2
#define MANUFACTURE_TEST_ADDPORT_ERROR   0x4
#define MANUFACTURE_TEST_PORTAPI_ERROR   0x8
#define MANUFACTURE_TEST_PORTLINKDOWN_ERROR  0x10
#define MANUFACTURE_TEST_PORTLOSTPACKET_ERROR  0x20
#define MANUFACTURE_TEST_COMBOSET_ERROR 0X200
#define MANUFACTURE_TEST_SUBBOARD_ERROR 0X400
int test_led_op(int slot, unsigned int value)
{
    wled8713_port_args wled_port_data = {0};
	poe_port_t poe_port = {0};
	int op_ret;
	int i;
	int poe_num = 0;
	if(value == 1)
	{
	    wled_port_data.led_mode = 2;
		poe_port.state = POE_ALARM;
	}
	else
	{
	    wled_port_data.led_mode = 1;
		poe_port.state = POE_OFF;
	}
    for(i = 0; i < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0); i++)
    {
        wled_port_data.led_index = i;
	   /* wled_port_data.led_mode = value;1--off , 2--ligntened*/
	 
	    op_ret = nbm_wireless_led_operate((wled_port_args*)&wled_port_data);
	    if (op_ret != 0)
	    {
		    //printf("test_led_op wireless error!.\n");
		    return -1;
	    }
		usleep(100000);
	}

    if(localmoduleinfo->fix_spec_param->poe_module_fix_param)
    {
        if(localmoduleinfo->fix_spec_param->poe_module_fix_param->poe_ports[0] == 0xffffff)
            poe_num = 24;
	    else if(localmoduleinfo->fix_spec_param->poe_module_fix_param->poe_ports[0] == 0xff)
            poe_num = 8;	
	    for(i = 0; i < poe_num; i++)
	    {
		    if((localmoduleinfo->fix_spec_param->poe_module_fix_param->poe_ports[i/32] & (1 << i)) == 0)
		    {
		        continue;
		    }
		    poe_port.port = i;
			poe_port.speed = POE_GE;
    		
		    op_ret = nbm_poe_led(&poe_port);
			if (op_ret != 0)
		    {
			    //printf("test_led_op wireless error!.\n");
			    return -1;
		    }
			usleep(100000);
	    }     
    }
	
	return 0;
		
}


int test_add_port_to_vlan(int slot, FILE *fp)
{
    int j = 0;
    unsigned int eth_index;
    unsigned short vid;
    int ret;
    char line[256];
    int en = 0;

	if(!SYS_MODULE_ISHAVEPP(SYS_LOCAL_MODULE_TYPE))
	{
	    return 0;/*ASX9604L and ASX6602L doesn't have switch chip*/
	}

    for(j = 0; j < 127+16; j++)
    {
        nam_asic_trunk_delete(j);
    }

	for(vid = 2; vid <= 200; vid++)
	{
	   ret = nam_test_vlan_entry_active(vid);
	   if(0 != ret)
	   {
		   sprintf(line, "!Failed create vlan %d failed, test stop.\n", vid);
		   fputs(line, fp);
		   return MANUFACTURE_TEST_CREATE_VLAN_ERROR;
	   }
	}

    if(SYS_LOCAL_MODULE_SLOT_INDEX == slot)
    {
        for (j = 0; j < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0); j++)
        {
            eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 
            ret = nam_asic_vlan_entry_ports_del(1, eth_index, 0);
            if(0 != ret)
            {
               sprintf(line, "!Failed delete port %d/%d from vlan 1 error, test stop.\n", slot, j+1);
               fputs(line, fp);
               return MANUFACTURE_TEST_DELPORT_ERROR;
            }
            ret = nam_asic_vlan_entry_ports_add(eth_index, j+2, 0);
            ret = nam_asic_set_port_pvid(eth_index, j+2);
            if(0 != ret)
            {
               sprintf(line, "!Failed add port %d/%d to vlan %d error, test stop.\n", slot, j+1, j/2+2);
               fputs(line, fp);
               return MANUFACTURE_TEST_ADDPORT_ERROR;
            }
			/* 测试面板端口不再先关闭端口再打开
            		ret = nam_set_ethport_enable(eth_index, 0);
            		*/
            
        }
    }
    else
        en = 1;
if (!app_box_state_get())
{
    for (j = 0; j < local_board_conn_type->plane_portnum; j++)
    {
         int unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, j);
         int  port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, j);
    
        if(-1 != unit)
        {
            int peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, j);
			if(peer_slot == -1)
			{
				continue;
			}
            if(slot == SYS_LOCAL_MODULE_SLOT_INDEX)
            {
				nam_test_set_stack_eth_port(unit, port);
                nam_test_set_port_endis(unit, port, en);
			
				nam_test_vlan_entry_ports_adddel(1, unit, port, 0);
				nam_test_vlan_entry_ports_adddel(4095, unit, port, 0);
				nam_test_vlan_entry_ports_adddel(100-j, unit, port, 1);
				nam_asic_test_set_port_pvid(unit, port, 100-j);
		      
            }  
            if(peer_slot == slot)
            {
                /*
                char mac[6] = {0};
                int mod = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX,
                    unit, port);
                int mod_port = UNIT_PORT_2_MODULE_PORT(SYS_LOCAL_MODULE_TYPE, unit, port);
                */
                nam_test_set_port_endis(unit, port, en);
                nam_test_set_stack_eth_port(unit, port);
                nam_test_set_port_localswitch(unit, port, 1, 100-j);
                /*
                mac[5] = (char)peer_port;
                nam_static_fdb_entry_mac_test_set(mac, 1, unit, mod, mod_port);
                */
                nam_test_vlan_entry_ports_adddel(1, unit, port, 0);
                nam_test_vlan_entry_ports_adddel(4095, unit, port, 0);
                nam_test_vlan_entry_ports_adddel(100-j, unit, port, 1);
                nam_asic_test_set_port_pvid(unit, port, 100-j);
                
            }
            else
                nam_test_set_port_endis(unit, port, 0);
        }
    }

    {
       unsigned char i;
       for(i = 0; i < nam_asic_get_instance_num(); i++)
       {
           unsigned char port;
           for(port = 0; ;port++)
           {
               if(PPAL_PHY_EXIST(SYS_LOCAL_MODULE_TYPE, i, port))
               {
                   unsigned int peer_unit;
                   peer_unit = PPAL_PHY_2_PEERUNIT(SYS_LOCAL_MODULE_TYPE, i, port);
                   if(-1 != peer_unit)
                   {
                       nam_test_set_port_endis(i, port, 0);
                       nam_test_set_stack_eth_port(i, port);
                       nam_test_vlan_entry_ports_adddel(1, i, port, 0);
                       nam_test_vlan_entry_ports_adddel(4095, i, port, 0);
                       nam_test_vlan_entry_ports_adddel(150+i*50-port, i, port, 1);
                       nam_asic_test_set_port_pvid(i, port, 150+i*50-port);
                   }
               }
               else
                break;
           }
       }
    }
}

    return 0;
    
}

char test_buffer[1400] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff,0xff,0xff,0xff,0xff,0xff,
    0x00,0x00,0x00,0x00,0x00,0xe0,
    0x08,0x06,0x00,0x01,0x08,0x00,
    0x06,0x04,0x00,0x01,0x00,0x00,
    0x00,0x00,0x00,0xe0,0x10,0xc0,
    0xa8,0x03,0xea,0x00,0x00,0x00,
    0x00,0x00,0x00,0xc0,0xa8,0x00,
    0x01,
};

int eth_port_fiber_type_check(unsigned int eth_g_index)
{
	struct eth_port_s *portInfo = NULL;
	portInfo = npd_get_port_by_index(eth_g_index);
	if (portInfo == NULL)
		return -1;
	if (portInfo->port_type == ETH_FE_FIBER 
		|| portInfo->port_type == ETH_GE_FIBER
		|| portInfo->port_type == ETH_GE_SFP
		|| portInfo->port_type == ETH_XGE_XFP
		|| portInfo->port_type == ETH_XGE_FIBER
		|| portInfo->port_type == ETH_XGE_SFPPLUS)
	{
		free(portInfo);
		return 1;
	}
    free(portInfo);
	return 0;
}


int manu_testing = 0;
int test_all_panel_odd_port(int slot, FILE *fp, unsigned char odd_flag, unsigned char odd_port)
{

    int ret = 0;
    int all_ret = 0;
    char line[256];
    eth_port_stats_t tx;
	eth_port_stats_t rx;
	int odd_test_counter = 0;
	unsigned long link_status = 0;
	unsigned long odd_link_status = 0;
	unsigned long odd_peer_link_status = 0;
	unsigned int odd_index = 0;
	unsigned int odd_peer_index = 0;
	/*处理最后的单数端口*/
	//printf("odd_port_test, odd_flag %d, odd_port %d.\n", odd_flag, odd_port);
	if (odd_flag == 1)
	{
		odd_index = eth_port_generate_ifindex(0, slot, 0, odd_port, 0);
		/*最后一个端口为光口*/
		if (1 == eth_port_fiber_type_check(odd_index))
		{
			ret = nam_get_port_link_state(odd_index, &link_status);
        	if(0 != ret)
            	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        	if(!link_status)
        	{
            	sprintf(line, "!Failed the fiber port %d/%d is link down.\n", slot+1, odd_port + 1);
            	fputs(line, fp);
            	if(0 == all_ret)
                	all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
                return all_ret;
        	}
			else
			{
				ret = nam_asic_clear_port_pkt_stat(odd_index);
        		if((0 != ret) && (0 == all_ret))
            		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;  
            	int i;
            	for (i = 0; i < 100; i++)
            	{
                	nam_packet_tx_unicast_by_netif(0, odd_index, odd_port+2, 0, (test_buffer + 64), 60);
            	}
        		ret = nam_asic_port_pkt_statistic(odd_index, &tx);
        		if((0 != ret) && (0 == all_ret))
            		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
				if(tx.snmp_stats.obytes == 0)
        		{
            		sprintf(line, "!Failed the fiber port %d/%d send no packets.\n", slot+1, odd_port + 1);
            		fputs(line, fp);
            		if(0 == all_ret)
                		all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
            
        		}
				if((tx.snmp_stats.obytes != tx.snmp_stats.ibytes) 
					|| (tx.snmp_stats.etheronoerrors != tx.snmp_stats.etherinoerrors))
				{
            		sprintf(line, "!Failed the fiber port %d/%d test lost packets.\n",  slot+1, odd_port + 1);
            		fputs(line, fp);
            		if(0 == all_ret)
                		all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;                                                      
        		}
			}
		}
		else
		{
			if (odd_port == 0)/*板子上 只有一个端口，而且不是光口类型，无法测试，报错。*/
			{
				sprintf(line, "!Failed there is only one port on panel, and this port is not fiber type, can not test.\n");
            	fputs(line, fp);
			}
			else
			{
				odd_index = eth_port_generate_ifindex(0, slot, 0, odd_port, 0);
				odd_peer_index = eth_port_generate_ifindex(0, slot, 0, odd_port - 1, 0);
				
				if (0 == nam_combo_ethport_check(odd_index))
				{
					/*测试最后COMBO口的光口*/
					{
						odd_index = eth_port_generate_ifindex(0, slot, 0, odd_port, 0);
						ret = nam_get_port_link_state(odd_index, &link_status);
        				if(0 != ret)
            			all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        				if(!link_status)
        				{
            				sprintf(line, "!Failed the fiber port %d/%d is link down.\n", slot+1, odd_port + 1);
            				fputs(line, fp);
            				if(0 == all_ret)
                				all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;\
                			return all_ret;
        				}
						else
						{
							ret = nam_asic_clear_port_pkt_stat(odd_index);
        					if((0 != ret) && (0 == all_ret))
            					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;  
            				int i;
            				for (i = 0; i < 100; i++)
            				{
                				nam_packet_tx_unicast_by_netif(0, odd_index, odd_port+2, 0, (test_buffer + 64), 60);
            				}
        					ret = nam_asic_port_pkt_statistic(odd_index, &tx);
        					if((0 != ret) && (0 == all_ret))
            					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
							if(tx.snmp_stats.obytes == 0)
        					{
            					sprintf(line, "!Failed the fiber port %d/%d send no packets.\n", slot+1, odd_port + 1);
            					fputs(line, fp);
            					if(0 == all_ret)
                					all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
            
        					}
        					if((tx.snmp_stats.obytes != tx.snmp_stats.ibytes) 
					              || (tx.snmp_stats.etheronoerrors != tx.snmp_stats.etherinoerrors))
        					{
            					sprintf(line, "!Failed the fiber port %d/%d test lost packets.\n",  slot+1, odd_port + 1);
            					fputs(line, fp);
            					if(0 == all_ret)
                				all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
        					}
						}
					}
					/*测试最后COMBO口的电口*/
					{
						nam_asic_vlan_entry_ports_del(odd_port + 1, odd_peer_index, 1);
						nam_asic_vlan_entry_ports_add(odd_peer_index, odd_port + 2, 1);

						printf("Please link the port %d/%d and %d/%d.\n", slot+1, odd_port, slot+1, odd_port+1);
						{
							nam_set_ethport_enable(odd_index, 0);
							nam_set_ethport_enable(odd_peer_index, 0);
							ret = nam_set_eth_port_trans_media(odd_index, COMBO_PHY_MEDIA_PREFER_COPPER);
							if (ret != 0)
							{
								sprintf(line, "!Failed set combo port %d/%d prefer media %d failed, ret %d.\n", slot+1, odd_port+1, COMBO_PHY_MEDIA_PREFER_COPPER, ret);
            					fputs(line, fp);
								if(0 != ret)
            	                all_ret |= MANUFACTURE_TEST_COMBOSET_ERROR; 
							}
							ret = nam_set_eth_port_trans_media(odd_peer_index, COMBO_PHY_MEDIA_PREFER_COPPER);
							if (ret != 0)
							{
								sprintf(line, "!Failed set combo port %d/%d prefer media %d failed, ret %d.\n", slot+1, odd_port, COMBO_PHY_MEDIA_PREFER_COPPER, ret);
            					fputs(line, fp);
								if(0 != ret)
            	                all_ret |= MANUFACTURE_TEST_COMBOSET_ERROR; 
							}
						}
						
						
						for (odd_test_counter = 0; odd_test_counter < 12; odd_test_counter++)
						{
							sleep(1);
							ret = nam_get_port_link_state(odd_index, &odd_link_status);
							ret = nam_get_port_link_state(odd_peer_index, &odd_peer_link_status);
							if ((1 == odd_link_status) && (1 == odd_peer_link_status))
								break;
							printf("Please link the port %d/%d and %d/%d.\n", slot+1, odd_port, slot+1, odd_port+1);
						}
						if(!odd_link_status || !odd_peer_link_status)
						{
							sprintf(line, "!Failed last ports  %d/%d or %d/%d is link down.\n", slot+1, odd_port, slot+1, odd_port+1);
							fputs(line, fp);
							if(0 == all_ret)
								all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
							return all_ret;
						}
						ret = nam_asic_clear_port_pkt_stat(odd_index);
						if((0 != ret) && (0 == all_ret))
							all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
						ret = nam_asic_clear_port_pkt_stat(odd_peer_index);
						if((0 != ret) && (0 == all_ret))
							all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
						
						int i;
						for (i = 0; i < 100; i++)
						{
							nam_packet_tx_unicast_by_netif(0, odd_peer_index, odd_port+2, 0, (test_buffer + 64), 60);
						}
						ret = nam_asic_port_pkt_statistic(odd_peer_index, &tx);
						if((0 != ret) && (0 == all_ret))
							all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
						
						ret = nam_asic_port_pkt_statistic(odd_index, &rx);
						if((0 != ret) && (0 == all_ret))
							all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
						
						if(tx.snmp_stats.obytes == 0)
						{
							sprintf(line, "!Failed the port %d/%d send no packets.\n", slot+1, odd_port);
							fputs(line, fp);
							if(0 == all_ret)
								all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
						
						}
						if((tx.snmp_stats.obytes != rx.snmp_stats.ibytes)
							|| (tx.snmp_stats.etheronoerrors != rx.snmp_stats.etherinoerrors))
						{
							sprintf(line, "!Failed the port %d/%d and %d/%d test lost packets.\n", slot+1, odd_port, slot+1, odd_port+1);
							fputs(line, fp);
							if(0 == all_ret)
								all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
						}
					}
				}
				else
				{
					nam_asic_vlan_entry_ports_del(odd_port + 1, odd_peer_index, 1);
					nam_asic_vlan_entry_ports_add(odd_peer_index, odd_port + 2, 1);
					printf("Please link the port %d/%d and %d/%d.\n", slot+1, odd_port, slot+1, odd_port+1);
					for (odd_test_counter = 0; odd_test_counter < 12; odd_test_counter++)
					{
						sleep(1);
						ret = nam_get_port_link_state(odd_index, &odd_link_status);		
        				ret = nam_get_port_link_state(odd_peer_index, &odd_peer_link_status);
        				if ((1 == odd_link_status) && (1 == odd_peer_link_status))
        					break;
						printf("Please link the port %d/%d and %d/%d.\n", slot+1, odd_port, slot+1, odd_port+1);
					}
					if(!odd_link_status || !odd_peer_link_status)
					{
						sprintf(line, "!Failed last ports  %d/%d or %d/%d is link down.\n", slot+1, odd_port, slot+1, odd_port+1);
            			fputs(line, fp);
            			if(0 == all_ret)
                			all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
						return all_ret;
					}
					ret = nam_asic_clear_port_pkt_stat(odd_index);
        			if((0 != ret) && (0 == all_ret))
            			all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        			ret = nam_asic_clear_port_pkt_stat(odd_peer_index);
       	 			if((0 != ret) && (0 == all_ret))
            			all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
       
            		int i;
            		for (i = 0; i < 100; i++)
            		{
                		nam_packet_tx_unicast_by_netif(0, odd_peer_index, odd_port+2, 0, (test_buffer + 64), 60);
            		}
        			ret = nam_asic_port_pkt_statistic(odd_peer_index, &tx);
        			if((0 != ret) && (0 == all_ret))
            			all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;

        			ret = nam_asic_port_pkt_statistic(odd_index, &rx);
        			if((0 != ret) && (0 == all_ret))
            			all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;

        			if(tx.snmp_stats.obytes == 0)
        			{
            			sprintf(line, "!Failed the port %d/%d send no packets.\n", slot+1, odd_port);
            			fputs(line, fp);
            			if(0 == all_ret)
                			all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
            
        			}
        			if((tx.snmp_stats.obytes != rx.snmp_stats.ibytes)
            			|| (tx.snmp_stats.etheronoerrors != rx.snmp_stats.etherinoerrors))
        			{
            			sprintf(line, "!Failed the port %d/%d and %d/%d test lost packets.\n", slot+1, odd_port, slot+1, odd_port+1);
            			fputs(line, fp);
            			if(0 == all_ret)
                			all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
        			}
				}
			}
		}
	}
	return all_ret;
}

int test_all_panel_combo_port(int slot, FILE *fp, int is_combo_board, int is_copper_prefered, int combo_first_port, int combo_last_port)
{
    int j = 0;
    unsigned int eth_index;
    unsigned int peer_index;
    int ret;
    int all_ret = 0;
    char line[256];
	char media[10] = {0};
	int global_media_prefered = 0;
	int local_media_prefered = 0;

    eth_port_stats_t tx;
    eth_port_stats_t rx;
	if (1 == is_combo_board)
	{
		if (1 == is_copper_prefered)
		{
			global_media_prefered = COMBO_PHY_MEDIA_PREFER_FIBER;/*媒体介质设置为相反*/
			strncpy(media, "fiber", sizeof("fiber"));
		}
		else if (0 == is_copper_prefered)
		{
			global_media_prefered = COMBO_PHY_MEDIA_PREFER_COPPER;/*媒体介质设置为相反*/
			strncpy(media, "copper", sizeof("copper"));
		}
		else
		{
			sprintf(line, "!Failed the combo ports media is error, stop test next combo ports.\n");
            fputs(line, fp);
			return all_ret;
		}
		for (j = combo_first_port; j <= combo_last_port; j++)
		{ 
			eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 
			nam_set_ethport_enable(eth_index, 0);
		}
		sleep(2);/*等待端口down*/
		for (j = combo_first_port; j <= combo_last_port; j++)
		{
			eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 
			nam_set_ethport_enable(eth_index, 0);
			nam_set_eth_port_trans_media(eth_index, global_media_prefered);
		}
		for (j = combo_first_port; j <= combo_last_port; j++)
		{ 
			eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 
			nam_set_ethport_enable(eth_index, 1);
		}
		sleep(8);/*等待设置媒体介质*/
		for (j = combo_first_port; j < combo_last_port; j+=2)
		{
			unsigned long link_status;
        	unsigned long peer_link_status;
			eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 

        	eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 
        	peer_index = eth_port_generate_ifindex(0, slot, 0, j+1, 0); 
        	ret = nam_set_ethport_enable(eth_index, 1);
        	ret = nam_set_ethport_enable(peer_index, 1);

			ret = combo_port_active_medium_get(eth_index, &local_media_prefered);
			if (local_media_prefered != global_media_prefered)
			{
				sprintf(line, "!Failed the combo port %d/%d or %d/%d %s port is not link.\n", slot+1, j+1, slot+1, j+2, media);
            	fputs(line, fp);
				if(0 == all_ret)
                	all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
				continue;
			}
			else
			{
				ret = combo_port_active_medium_get(peer_index, &local_media_prefered);
				if (local_media_prefered != global_media_prefered)
				{
					sprintf(line, "!Failed the combo port %d/%d or %d/%d %s port is  not link.\n", slot+1, j+1, slot+1, j+2, media);
            		fputs(line, fp);
					if(0 == all_ret)
                	all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
					continue;
				} 
			}
        	ret = nam_get_port_link_state(eth_index, &link_status);
        	if(0 != ret)
            	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        	ret = nam_get_port_link_state(peer_index, &peer_link_status);
        	if((0 != ret) && (0 == all_ret))
            	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        	if(!link_status || !peer_link_status)
        	{
            	sprintf(line, "!Failed the combo ports %s media %d/%d or %d/%d is link down.\n", media, slot+1, j+1, slot+1, j+2);
            	fputs(line, fp);
            	if(0 == all_ret)
                	all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
            	continue;
        	}
        	ret = nam_asic_clear_port_pkt_stat(eth_index);
        	if((0 != ret) && (0 == all_ret))
            	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        	ret = nam_asic_clear_port_pkt_stat(peer_index);
       	 	if((0 != ret) && (0 == all_ret))
            	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        
        	if(0 == j%2)
        	{
            	int i;
            	for (i = 0; i < 100; i++)
            	{
                	nam_packet_tx_unicast_by_netif(0, eth_index, j+2, 0, (test_buffer + 64), 60);
					nam_packet_tx_unicast_by_netif(0, peer_index, j+3, 0, (test_buffer + 64), 60);
            	}
        	}
			memset(&tx, 0,  sizeof(tx));
			memset(&rx, 0,  sizeof(rx));
        	ret = nam_asic_port_pkt_statistic( eth_index, &tx);
        	if((0 != ret) && (0 == all_ret))
            	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
			ret = nam_asic_port_pkt_statistic( peer_index, &rx);
        	if((0 != ret) && (0 == all_ret))
            	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
			
        	if((tx.snmp_stats.obytes == 0) || (rx.snmp_stats.obytes == 0))
        	{
            	sprintf(line, "!Failed the combo ports %s media %d/%d send no packets.\n", media, slot+1, j+1);
            	fputs(line, fp);
            	if(0 == all_ret)
                	all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
            
        	}
        	if((tx.snmp_stats.obytes != rx.snmp_stats.ibytes)
            	|| (tx.snmp_stats.etheronoerrors != rx.snmp_stats.etherinoerrors)
            	|| (rx.snmp_stats.obytes != tx.snmp_stats.ibytes)
            	|| (rx.snmp_stats.etheronoerrors != tx.snmp_stats.etherinoerrors)
            	)
        	{
            	sprintf(line, "!Failed the combo ports %s media %d/%d and %d/%d test lost packets.\n", media, slot+1, j+1, slot+1, j+2);
            	fputs(line, fp);
            	if(0 == all_ret)
                	all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
        	}
		}
	}
	return all_ret;
}

int test_all_panel_xge_port(int slot, FILE *fp)
{
    char line[256];
    unsigned int panel_portnum;
	int all_ret = 0;
    int ret, j, i;
	
	panel_portnum = ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0);
	syslog_ax_product_dbg("\nTest panel port !\n");
	for(i = 0; i < 2; i++)
	{
	    for(j = 0;j < panel_portnum; j++)
	    {
			if(i == 0)
			{
			    /*this function is used to test the panel port of ASX6602 and ASX9604L*/
	            ret = nam_test_set_port_endis(0, j, panel_portnum);
			}
			else
			{
			    /*This function is used to check the result of test register and recover 
			          the state of register.
			          The parameter 1 for panel port.
			       */
			    ret = nam_test_set_port_localswitch(panel_portnum, j, 1, 0);
			}
			
			if(ret == -1) 
			{
			    syslog_ax_product_err("Failed to read panel port%d Checker register!\n", j+1);
				sprintf(line, "!Failed to read the panel port%d's Checker register normally.\n", j+1);
	            fputs(line, fp);
				if(0 == all_ret)
				{
				    all_ret |= MANUFACTURE_TEST_REG_READ_ERROR;
				}
				continue;
				
				
			}
			else if((ret != 0) && (i == 1))
			{
			    if(ret == 0xaaaa)
			    {
			        syslog_ax_product_err("Panel port%d is link down!\n", j+1);
				    sprintf(line, "!Failed the panel port%d is link down!\n", j+1);
	                fputs(line, fp);
					if(0 == all_ret)
					{
					    all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
					}
					continue;
			    }
				
			    syslog_ax_product_err("Failed the panel port%d is error! The value of checker regester is 0x%x\n", j+1,ret);
				sprintf(line, "!Failed the panel port%d exists some error! \n", j+1);
	            fputs(line, fp);
				if(0 == all_ret)
				{
				    all_ret |= MANUFACTURE_TEST_XGE_ERROR;
				}
				continue;
				
			}
				
	    }
		
		if(i == 0)
		{
		    syslog_ax_product_dbg("\n sleep for 10s! \n");
		    /*wait 15s for check register result of test*/
	        sleep(10);
		}
	
	}
	
	return all_ret;
}

int test_panel_port_with_subboard(int slot, FILE *fp)
{
    int j = 0;
    int ret = 0;
    int all_ret = 0;
    unsigned int eth_index;
    unsigned int peer_index;
	char line[256];
	eth_port_stats_t tx;
    eth_port_stats_t rx;
	unsigned long link_status;
    unsigned long peer_link_status;
	unsigned short vid_base = 0;

	if(1 == localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum)
		return 0;
    for(j = 1; j < localmoduleinfo->fix_param->subboard_fix_param->sub_slotnum; j++)
	{
		if((localmoduleinfo->sub_board[j] == NULL)
			||(localmoduleinfo->sub_board[j]->fix_param == NULL)
			|| (localmoduleinfo->sub_board[j]->inserted == 0))
		{
			sprintf(line, "!Failed the subboard %d is absence.\n", j);
		    fputs(line, fp);
			all_ret |= MANUFACTURE_TEST_SUBBOARD_ERROR;
			return all_ret;
		}
		
		if(localmoduleinfo->sub_board[j]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_4SFP)
		{
		    sprintf(line, "!Failed the subboard %d should be SFP+ subboard for board test.\n", j);
		    fputs(line, fp);
			all_ret |= MANUFACTURE_TEST_SUBBOARD_ERROR;
			return all_ret;
		}
		
	}
	vid_base = ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0) + 2;
	/*add port to vlan*/
	for (j = 1; j < 3; j++)
	{
		eth_index = eth_port_generate_ifindex(0, j, 0, 0, 0); 
		ret = nam_asic_vlan_entry_ports_del(1, eth_index, 0);
		if(0 != ret)
		{
			sprintf(line, "!Failed delete port 1/%d/1 from vlan 1 error, test stop.\n", j);
			fputs(line, fp);
			return MANUFACTURE_TEST_DELPORT_ERROR;
		}
		ret = nam_asic_vlan_entry_ports_add(eth_index, vid_base+j, 0);
		ret = nam_asic_set_port_pvid(eth_index, vid_base+j);
		if(0 != ret)
		{
		   sprintf(line, "!Failed add port 1/%d/1 to vlan %d error, test stop.\n", j,vid_base+j);
		   fputs(line, fp);
		   return MANUFACTURE_TEST_ADDPORT_ERROR;
		}
		
	}
    eth_index = eth_port_generate_ifindex(0, 1, 0, 0, 0); 
	peer_index = eth_port_generate_ifindex(0, 2, 0, 0, 0); 
	ret = nam_set_ethport_enable(eth_index, 1);
	ret = nam_set_ethport_enable(peer_index, 1);
	ret = nam_get_port_link_state(eth_index, &link_status);
	if(0 != ret)
		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
	ret = nam_get_port_link_state(peer_index, &peer_link_status);
	if((0 != ret) && (0 == all_ret))
		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
	if(!link_status || !peer_link_status)
	{
		
		sprintf(line, "!Failed the subboard port 1/1/1 or 1/2/1 is link down.\n");
		fputs(line, fp);
		if(0 == all_ret)
			all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
		return all_ret;
	}
	ret = nam_asic_clear_port_pkt_stat(eth_index);
	if((0 != ret) && (0 == all_ret))
	all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
    ret = nam_asic_clear_port_pkt_stat(peer_index);
	if((0 != ret) && (0 == all_ret))
		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
		
	int i;
	for (i = 0; i < 100; i++)
	{
		nam_packet_tx_unicast_by_netif(0, eth_index,vid_base+1, 0, (test_buffer + 64), 60);
		/*发包不再等待
		usleep(300);
		*/		
		nam_packet_tx_unicast_by_netif(0, peer_index, vid_base+2, 0, (test_buffer + 64), 60);
	}
	
	memset(&tx, 0,  sizeof(tx));
	memset(&rx, 0,  sizeof(rx));
	ret = nam_asic_port_pkt_statistic( eth_index, &tx);
	if((0 != ret) && (0 == all_ret))
		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
	ret = nam_asic_port_pkt_statistic( peer_index, &rx);
	if((0 != ret) && (0 == all_ret))
		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;

	if((tx.snmp_stats.obytes == 0)||(rx.snmp_stats.obytes == 0))
	{
		sprintf(line, "!Failed the subboard ports 1/1/1 or 1/2/1 send no packets.\n");
		fputs(line, fp);
		if(0 == all_ret)
			all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
		
	}
	if((tx.snmp_stats.obytes != rx.snmp_stats.ibytes)
		|| (tx.snmp_stats.etheronoerrors != rx.snmp_stats.etherinoerrors)
		|| (rx.snmp_stats.obytes != tx.snmp_stats.ibytes)
		|| (rx.snmp_stats.etheronoerrors != tx.snmp_stats.etherinoerrors)
		)
	{
		sprintf(line, "!Failed the subboard ports 1/1/1 and 1/2/1 test lost packets.\n");
		fputs(line, fp);
		if(0 == all_ret)
			all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
	}
	return all_ret;
				
}

int test_all_panel_port(int slot, FILE *fp)
{
    int j = 0;
    unsigned int eth_index;
    unsigned int peer_index;
    int ret = 0;
    int all_ret = 0;
    char line[256];
	char media[10] = {0};
	int is_combo_board = -1;
	int is_combo_port = 0;
	int is_copper_prefered  = -1;
	int global_media_prefered = 0;
	int local_media_prefered = 0;
	unsigned int combo_first_port = -1;
	unsigned int combo_last_port = -1;
    eth_port_stats_t tx;
    eth_port_stats_t rx;

	unsigned char odd_flag = 0;/*奇数端口标志*/
	unsigned char odd_port = 0;
	unsigned char port_type = 0;
	unsigned long board_type = 0 ;
	
    if(slot != SYS_LOCAL_MODULE_SLOT_INDEX)
        return 0;
	
	/*
	ASX9604L and ASX6602L manufacture test use register of phy，
	and both of them don't have switch chip 
	*/
	if(!SYS_MODULE_ISHAVEPP(SYS_LOCAL_MODULE_TYPE))
	{
	    all_ret = test_all_panel_xge_port(slot, fp);
	    return all_ret;
	}
	/*test led of panel ,include wireless led and poe led */
	board_type = MODULE_TYPE_ON_SLOT_INDEX(SYS_LOCAL_MODULE_SLOT_INDEX);
	if((board_type >= PPAL_BOARD_TYPE_US_4628GS) && (board_type <= PPAL_BOARD_TYPE_US_4629GX_PWR))
	{   
	    ret = test_led_op(slot,1);
		if(0 != ret)
		{
		    sprintf(line, "!Failed test_led_op error!.\n");
	        fputs(line, fp);
	        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
		}
	    sleep(5);
		ret = test_led_op(slot,0);
		if(0 != ret)
		{
		    sprintf(line, "!Failed test_led_op error!.\n");
	        fputs(line, fp);
	        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
		}
	}	
	
	if (!app_box_state_get())
	{
    	for (j = 0; j < local_board_conn_type->plane_portnum; j++)
    	{
        	unsigned char unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, j);
        	unsigned char port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, j);

        	nam_test_set_port_endis(unit, port, 0);
    	}
	}
/* 面板端口现在不再linkdown后再等待linkup了，增加延时为了防止产测在插入板子
端口没有linkup起来时就开始测试*/
	sleep(8);
    for (j = 0; j < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0); j+=2)
    {
        unsigned long link_status;
        unsigned long peer_link_status;

		if (j == (ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0) - 1))
		{
			port_type = npd_get_port_type(MODULE_TYPE_ON_SLOT_INDEX(slot), ETH_LOCAL_INDEX2NO(slot,j));
			if (ETH_MNG != port_type)
			{
				odd_flag = 1;
				odd_port = j;
			}
			continue;
		}

        eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 
        peer_index = eth_port_generate_ifindex(0, slot, 0, j+1, 0); 
        ret = nam_set_ethport_enable(eth_index, 1);
        ret = nam_set_ethport_enable(peer_index, 1);
		if ((0 == nam_combo_ethport_check(eth_index)) && (0 == nam_combo_ethport_check(peer_index)))
		{
			is_combo_port = 1;
			if (1 == is_combo_board)
			{
				combo_last_port = j+1; /*记录combo端口范围*/
			}
			else
			{
				nam_get_eth_port_trans_media(eth_index, &global_media_prefered);
				/*现在是取第一个combo口介质属性作为所有combo口介质属性
				下次会将所有combo口属性设置为相对的属性再测一次
				假如combo口属性有些不同会有问题，考虑到时产测所有combo口介质设置应该一致*/
				if (global_media_prefered == COMBO_PHY_MEDIA_PREFER_COPPER)
				{
					is_copper_prefered = 1;
					strncpy(media, "copper", sizeof("copper"));
				}
				else if (global_media_prefered == COMBO_PHY_MEDIA_PREFER_FIBER)
				{
					is_copper_prefered = 0;
					strncpy(media, "fiber", sizeof("fiber"));
				}
				else
				{
					is_copper_prefered = -1;
				}
				is_combo_board = 1;
				combo_first_port = j;
				combo_last_port = j+1;/*记录combo端口范围*/
			}
		}
		else
		{
			is_combo_port = 0;
		}
        if (is_combo_port == 1)
        {
			ret = combo_port_active_medium_get(eth_index, &local_media_prefered);
			if (local_media_prefered != global_media_prefered)
			{
				sprintf(line, "!Failed the combo port %d/%d or %d/%d %s is not link.\n", slot+1, j+1, slot+1, j+2, media);
            	fputs(line, fp);
				if(0 == all_ret)
                all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
				continue;
			}
			ret = combo_port_active_medium_get(peer_index, &local_media_prefered);
			if (local_media_prefered != global_media_prefered)
			{
				sprintf(line, "!Failed the combo port %d/%d or %d/%d %s is not link.\n", slot+1, j+1, slot+1, j+2, media);
            	fputs(line, fp);
				if(0 == all_ret)
                all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
				continue;
			}
		}
        ret = nam_get_port_link_state(eth_index, &link_status);
        if(0 != ret)
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        ret = nam_get_port_link_state(peer_index, &peer_link_status);
        if((0 != ret) && (0 == all_ret))
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        if(!link_status || !peer_link_status)
        {
			if (is_combo_port == 1)
			{
				sprintf(line, "!Failed the combo port %s media %d/%d or %d/%d is link down.\n", media, slot+1, j+1, slot+1, j+2);
			}
			else
			{
            	sprintf(line, "!Failed the port %d/%d or %d/%d is link down.\n", slot+1, j+1, slot+1, j+2);
			}
            fputs(line, fp);
            if(0 == all_ret)
                all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
            continue;
        }
        ret = nam_asic_clear_port_pkt_stat(eth_index);
        if((0 != ret) && (0 == all_ret))
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        ret = nam_asic_clear_port_pkt_stat(peer_index);
        if((0 != ret) && (0 == all_ret))
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        if(0 == j%2)
        {
            int i;
            for (i = 0; i < 100; i++)
            {
                nam_packet_tx_unicast_by_netif(0, eth_index, j+2, 0, (test_buffer + 64), 60);
				/*发包不再等待
                usleep(300);
                */		
                nam_packet_tx_unicast_by_netif(0, peer_index, j+3, 0, (test_buffer + 64), 60);
            }
        }
		memset(&tx, 0,  sizeof(tx));
		memset(&rx, 0,  sizeof(rx));
        ret = nam_asic_port_pkt_statistic( eth_index, &tx);
        if((0 != ret) && (0 == all_ret))
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
		ret = nam_asic_port_pkt_statistic( peer_index, &rx);
        if((0 != ret) && (0 == all_ret))
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;

        if((tx.snmp_stats.obytes == 0)||(rx.snmp_stats.obytes == 0))
        {
			if (is_combo_port == 1)
			{
				sprintf(line, "!Failed the combo port %s media %d/%d send no packets.\n", media, slot+1, j+1);
			}
			else
			{
            	sprintf(line, "!Failed the ports %d/%d send no packets.\n", slot+1, j+1);
			}
            fputs(line, fp);
            if(0 == all_ret)
                all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
            
        }
        if((tx.snmp_stats.obytes != rx.snmp_stats.ibytes)
            || (tx.snmp_stats.etheronoerrors != rx.snmp_stats.etherinoerrors)
            || (rx.snmp_stats.obytes != tx.snmp_stats.ibytes)
            || (rx.snmp_stats.etheronoerrors != tx.snmp_stats.etherinoerrors)
            )
        {
			if (is_combo_port == 1)
			{
				sprintf(line, "!Failed the combo port %s media %d/%d and %d/%d test lost packets.\n", media, slot+1, j+1, slot, j+2);
			}
			else
			{
            	sprintf(line, "!Failed the ports %d/%d and %d/%d test lost packets.\n", slot+1, j+1, slot+1, j+2);
			}
            fputs(line, fp);
            if(0 == all_ret)
                all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
        }
    }   
	ret = test_all_panel_combo_port(slot, fp, is_combo_board, is_copper_prefered, combo_first_port, combo_last_port);
	if((0 != ret)&& (0 == all_ret))
        all_ret |= ret;
	ret = test_all_panel_odd_port(slot, fp, odd_flag, odd_port);
	if((0 != ret)&& (0 == all_ret))
        all_ret |= ret;
	ret = test_panel_port_with_subboard(slot, fp);
	if((0 != ret)&& (0 == all_ret))
        all_ret |= ret;
    return all_ret;
}

int test_all_xaui_mux_port(int slot, FILE *fp)
{
    char line[256];
    int ret, j, i, k;
	int all_ret = 0;
	unsigned int plane_portnum;
	cpld_mux_args cpld_mux_param;
	
	syslog_ax_product_dbg("\nTest plane port !\n");

	for(i = 0; i < 2; i++)
	{
	    cpld_mux_param.master_slot = i;
	    ret = nbm_xaui_switch(&cpld_mux_param);
	    if (ret < 0)
	    {
		    npd_syslog_err("XAUI channel switch failed.\n");
			sprintf(line, "!Failed XAUI channel switch.\n");
            fputs(line, fp);
			if(0 == all_ret)
			    all_ret |= MANUFACTURE_TEST_XAUI_MUX_ERROR;
			continue;
	    }
		
		/*for ASX9604L and ASX6602 ,the numbers of  plane port and panel port are the same*/
        plane_portnum = ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0);
	    
        for (k = 0; k < 2; k++)
        {
		    for (j = 0; j < plane_portnum; j++)
		    {
                if(0 == k)
                {
				    /* this function is used to test the plane port of ASX6602 and ASX9604L*/
				    ret = nam_test_set_stack_eth_port(plane_portnum, j);
				}
				else
				{
                    /*This function is used to check the result of test register and recover 
			                 the state of register.
			                 The parameter first 0 for plane port.
			             */
			         ret = nam_test_set_port_localswitch(plane_portnum, j, 0, 0);
			       
				}
				
				if(ret == -1) 
			    {
			        syslog_ax_product_err("Failed to read plane port %d Checker register!\n", j+1);
					sprintf(line, "!Failed to read the plane port%d's Checker register normally.\n", j+1);
	                fputs(line, fp);
					if(0 == all_ret)
				        all_ret |= MANUFACTURE_TEST_REG_READ_ERROR;
				    continue;
				
			    }
			    else if((ret != 0) &&(k == 1))
			    {
			        if(ret == 0xf)
			        {
			            syslog_ax_product_err("MUX %d :Failed the plane port%d is link down!\n", i,j+1);
				        sprintf(line, "!MUX %d :Failed the plane port%d is link down!\n",i, j+1);
	                    fputs(line, fp);
						if(0 == all_ret)
					        all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
					    continue;
			        }
					
					syslog_ax_product_err("MUX %d : Failed the plane port%d is error! The value of checker regester is 0x%x\n", i, j+1,ret);
				    sprintf(line, "!MUX %d : Failed the plane port%d exists error! T\n", i, j+1);
					fputs(line, fp);
					if(ret & 0x01)
					{
					    sprintf(line, "Lane0 of plane port%d exits some error! \n", j+1);
	                    fputs(line, fp);
					}
					if(ret & 0x02)
					{
					    sprintf(line, "Lane1 of plane port%d exits some error! \n", j+1);
	                    fputs(line, fp);
					}
					if(ret & 0x04)
					{
					    sprintf(line, "Lane2 of plane port%d exits some error! \n", j+1);
	                    fputs(line, fp);
					}
					if(ret & 0x08)
					{
					    sprintf(line, "Lane3 of plane port%d exits some error! \n", j+1);
	                    fputs(line, fp);
					}
					if(0 == all_ret)
				        all_ret |= MANUFACTURE_TEST_XGE_ERROR;
				    continue;
				
			     }
		    }

			if(0 == k)
			{
			    syslog_ax_product_dbg("\n sleep for 10s! \n");
			    sleep(10);
			}
			
		}
		
		
	}
	
	return all_ret;
}
int test_all_xaui_port(int slot, FILE *fp)
{
    int j = 0;
    unsigned int eth_index;
    int ret;
    int all_ret = 0;
    char line[256];
    struct eth_port_counter_s tx;
    struct eth_port_counter_s rx;

    if(slot != SYS_LOCAL_MODULE_SLOT_INDEX)
    {
        return 0;
    }
	
    /*
	ASX9604L and ASX6602L manufacture test use register of phy，
	and both of them don't have switch chip 
	*/
    if(!SYS_MODULE_ISHAVEPP(SYS_LOCAL_MODULE_TYPE))
    {
        ret = test_all_xaui_mux_port(slot, fp);
		return ret;
    }


    for (j = 0; j < ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0); j++)
    {
        eth_index = eth_port_generate_ifindex(0, slot, 0, j, 0); 
        ret = nam_set_ethport_enable(eth_index, 0);
    }   
    for (j = 0; j < local_board_conn_type->plane_portnum; j++)
    {
        int unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, j);
        int port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, j);

        if(-1 != unit)
            nam_test_set_port_endis(unit, port, 0);
    }

    {
       unsigned char i;
       for(i = 0; i < nam_asic_get_instance_num(); i++)
       {
           unsigned char port;
           for(port = 0; ;port++)
           {
               if(PPAL_PHY_EXIST(SYS_LOCAL_MODULE_TYPE, i, port))
               {
                   unsigned int peer_unit;
                   peer_unit = PPAL_PHY_2_PEERUNIT(SYS_LOCAL_MODULE_TYPE, i, port);
                   if(-1 != peer_unit)
                   {
                       /*
                       char mac[6] = {0};
                       int mod = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE, SYS_LOCAL_MODULE_SLOT_INDEX,
                           i, port);
                       int mod_port = UNIT_PORT_2_MODULE_PORT(SYS_LOCAL_MODULE_TYPE, i, port);
                        */
                     
					   nam_test_set_stack_eth_port(i, port);
                       nam_test_set_port_endis(i, port, 0);/*
                       mac[5] = (char)port+16;
                       nam_static_fdb_entry_mac_test_set(mac, 1, i, mod, mod_port);
                       */
                   }
               }
               else
                break;
           }
       }
    }
    
    for (j = 0; j < local_board_conn_type->plane_portnum; j++)
    {
        unsigned long link_status = 0;
        int unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, j);
        int port = PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, j);
        if(-1 == unit)
            continue;
		ret = nam_test_set_stack_eth_port(unit, port);

        ret = nam_test_set_port_endis(unit, port, 1);
        if(0 != ret)
        {
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        }
        sleep(5);
        ret = nam_test_get_port_link_state(unit,port, &link_status);
        if(1 != link_status)
        {
            sprintf(line, "!Failed the XAUI port(%d, %d) of %s is down.\n", unit, port,
                 localmoduleinfo->fix_param->short_name);
            fputs(line, fp);
            all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
			continue;
        }

        ret = nam_asic_clear_stack_port_pkt_stat(unit, port);
        if(0 != ret)
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        
        {
            int i;
            /*
            memset(test_buffer, 0, 5);
            test_buffer[5] = peer_unit_port;
            */
            for (i = 0; i < 100; i++)
            {
                ret = nam_packet_tx_test_unicast(0, unit, port, 100-j, 0, (test_buffer + 64), 60);
				if(0 != ret)
				{
    				sprintf(line, "!Failed the plane port(%d, %d) send packets errcode %d.\n", unit, port, ret);
    				fputs(line, fp);
				}
				/*发包延时取消
                usleep(300);
				*/
            }
            /*
            memset(test_buffer, 0xff, 6);
            */
        }
        ret = nam_asic_stack_port_pkt_statistic( unit, port, &tx);
        if(0 != ret)
            all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;

        if(tx.tx.totalbyte == 0)
        {
            sprintf(line, "!Failed the plane ports(%d, %d) send no packets.\n", unit, port);
            fputs(line, fp);
            all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
            
        }
        if((tx.tx.totalbyte != tx.rx.totalbytes)
            || (tx.tx.goodframe != tx.rx.goodframes))
        {
            sprintf(line, "!Failed the plane ports(%d, %d) test lost packets.\n", unit, port);
            fputs(line, fp);
            all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
        }    
        ret = nam_test_set_port_endis(unit, port, 0);
    }

    {
       unsigned char i;
       for(i = 0; i < nam_asic_get_instance_num(); i++)
       {
           unsigned char port;
           for(port = 0; ;port++)
           {
               if(PPAL_PHY_EXIST(SYS_LOCAL_MODULE_TYPE, i, port))
               {
                   unsigned int peer_unit;
                   unsigned int peer_port;
                   unsigned long link_status;
                   peer_unit = PPAL_PHY_2_PEERUNIT(SYS_LOCAL_MODULE_TYPE, i, port);
                   peer_port = PPAL_PHY_2_PEERPORT(SYS_LOCAL_MODULE_TYPE, i, port);

                   if(-1 == peer_unit)
                    continue;

                   nam_test_set_port_endis(peer_unit, peer_port, 1);

                    ret = nam_test_set_port_endis(i, port, 1);
                    if(0 != ret)
                    {
                        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
						sprintf(line, "Test the XAUI port between ASICS api error\n");
						fputs(line, fp);
						
                        continue;
                    }
                    nam_test_set_port_localswitch(i, port, 0, 150+i*50-port);
                    nam_test_set_port_localswitch(peer_unit, peer_port, 1, 150+peer_unit*50-peer_port);
                    sleep(5);
                    ret = nam_test_get_port_link_state(i, port, &link_status);
                    if(1 != link_status)
                    {
                        sprintf(line, "!Failed the XAUI port(%d, %d) of %s is down.\n", i, port,
                             localmoduleinfo->fix_param->short_name);
                        fputs(line, fp);
                        all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
                    }
                    ret = nam_test_get_port_link_state(peer_unit, peer_port, &link_status);
                    if(1 != link_status)
                    {
                        sprintf(line, "!Failed the XAUI port(%d, %d) of %s is down.\n", peer_unit, peer_port,
                             localmoduleinfo->fix_param->short_name);
                        fputs(line, fp);
                        all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
                    }

                    
                    ret = nam_asic_clear_stack_port_pkt_stat(i, port);
                    if(0 != ret)
                        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
                    ret = nam_asic_clear_stack_port_pkt_stat(peer_unit, peer_port);
                    if(0 != ret)
                        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
                    
                    
                    {
                        int count;
                        /*
                        memset(test_buffer, 0, 5);
                        test_buffer[5] = peer_port+16;
                        */
                        for (count = 0; count < 100; count++)
                        {
                            ret = nam_packet_tx_test_unicast(0, i, port, 150+i*50-port, 0, (test_buffer + 64), 60);
							if(ret != 0)
							{
								sprintf(line, "!Failed the internal ports(%d, %d) send packets errcode %d.\n", i, port, ret);
								fputs(line, fp);
								
							}
							/*发包延时取消
                            usleep(300);
                            */
                        }
                        /*
                        memset(test_buffer, 0xff, 6);
                        */
                    }
                    sleep(1);
                    ret = nam_asic_stack_port_pkt_statistic( i, port, &tx);
                    if(0 != ret)
                        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;

                    ret = nam_asic_stack_port_pkt_statistic( peer_unit, peer_port, &rx);
                    if(0 != ret)
                        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
                   
                    if(tx.tx.totalbyte == 0)
                    {
                        sprintf(line, "!Failed the internal ports(%d, %d) send no packets.\n", i, port);
                        fputs(line, fp);
                        all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
                        
                    }
                    if((tx.tx.totalbyte != rx.rx.totalbytes)
                        || (tx.tx.goodframe != rx.rx.goodframes))
                    {
                        sprintf(line, "!Failed the internal ports(%d, %d) (%d, %d) test lost packets.\n", 
                            i, port, peer_unit,peer_port);
                        fputs(line, fp);
                        all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
                    }    
                    ret = nam_test_set_port_endis(i, port, 0);
                    ret = nam_test_set_port_endis(peer_unit, peer_port, 0);
                   
               }
               else
                break;
           }
       }
    }    
    return all_ret;
}
int test_subboard_port(int slot, int subslot, FILE *fp)
{
    int j = 0;
    int ret = 0;
    int all_ret = 0;
    unsigned short vid_base = 0;
	unsigned int sub_portnum  = 0;
	unsigned int eth_index;
    unsigned int peer_index;
	char line[256];
	eth_port_stats_t tx;
    eth_port_stats_t rx;
	unsigned long link_status = 0;
    unsigned long peer_link_status = 0;
	
	
    if(localmoduleinfo->sub_board[subslot] != NULL)
	{
	    sub_portnum = localmoduleinfo->fix_param->subboard_fix_param->sub_slot_portnum;
	    vid_base = ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0) + 2;
		if(localmoduleinfo->sub_board[subslot]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_4SFP)
		{
		    vid_base = ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0) + 2;
			for (j = 0; j < 4; j++)
			{
				eth_index = eth_port_generate_ifindex(0, subslot, 0, j, 0); 
				ret = nam_asic_vlan_entry_ports_del(1, eth_index, 0);
				if(0 != ret)
				{
					sprintf(line, "!Failed delete port 1/%d/%d from vlan 1 error, test stop.\n", subslot, j+1);
					fputs(line, fp);
					return MANUFACTURE_TEST_DELPORT_ERROR;
				}
				ret = nam_asic_vlan_entry_ports_add(eth_index, vid_base+j, 0);
				ret = nam_asic_set_port_pvid(eth_index, vid_base+j);
				if(0 != ret)
				{
				   sprintf(line, "!Failed add port 1/%d/%d to vlan %d error, test stop.\n", subslot, j+1, vid_base+j);
				   fputs(line, fp);
				   return MANUFACTURE_TEST_ADDPORT_ERROR;
				}
				
			}
			sleep(4);
			for (j = 0; j < 4; j+=2)
			{
				eth_index = eth_port_generate_ifindex(0, subslot, 0, j, 0); 
				peer_index = eth_port_generate_ifindex(0, subslot, 0, j+1, 0);		
				ret = nam_set_ethport_enable(eth_index, 1);
				ret = nam_set_ethport_enable(peer_index, 1);
				ret = nam_get_port_link_state(eth_index, &link_status);
				if(0 != ret)
					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
				ret = nam_get_port_link_state(peer_index, &peer_link_status);
				if((0 != ret) && (0 == all_ret))
					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
				if(!link_status || !peer_link_status)
				{
					
					sprintf(line, "!Failed the subboard %d port 1/%d/%d or 1/%d/%d is link down.\n",subslot, subslot, j+1, subslot, j+2);
					fputs(line, fp);
					if(0 == all_ret)
						all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
					continue;
				}
				ret = nam_asic_clear_port_pkt_stat(eth_index);
				if((0 != ret) && (0 == all_ret))
					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
				ret = nam_asic_clear_port_pkt_stat(peer_index);
				if((0 != ret) && (0 == all_ret))
					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
				if(0 == j%2)
				{
					int i;
					for (i = 0; i < 100; i++)
					{
						ret = nam_packet_tx_unicast_by_netif(0, eth_index, vid_base+j, 0, (test_buffer + 64), 60);
						/*发包不再等待
						usleep(300);
						*/
						if((0 != ret) && (0 == all_ret))
					        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
						
						ret = nam_packet_tx_unicast_by_netif(0, peer_index, vid_base+j+1, 0, (test_buffer + 64), 60);
						if((0 != ret) && (0 == all_ret))
					        all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
					}
				}
				memset(&tx, 0,  sizeof(tx));
				memset(&rx, 0,  sizeof(rx));
				ret = nam_asic_port_pkt_statistic( eth_index, &tx);
				if((0 != ret) && (0 == all_ret))
					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
				ret = nam_asic_port_pkt_statistic( peer_index, &rx);
				if((0 != ret) && (0 == all_ret))
					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;

				if((tx.snmp_stats.obytes == 0)||(rx.snmp_stats.obytes == 0))
				{
					sprintf(line, "!Failed the subboard %d ports 1/%d/%d send no packets.\n", subslot,subslot, j+1);
					fputs(line, fp);
					if(0 == all_ret)
						all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
					
				}
				if((tx.snmp_stats.obytes != rx.snmp_stats.ibytes)
					|| (tx.snmp_stats.etheronoerrors != rx.snmp_stats.etherinoerrors)
					|| (rx.snmp_stats.obytes != tx.snmp_stats.ibytes)
					|| (rx.snmp_stats.etheronoerrors != tx.snmp_stats.etherinoerrors)
					)
				{
					sprintf(line, "!Failed the subboard %d ports 1/%d/%d and 1/%d/%d test lost packets.\n",subslot, subslot, j+1, subslot, j+2);
					fputs(line, fp);
					if(0 == all_ret)
						all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
				}
			}   
		}
		else if(localmoduleinfo->sub_board[subslot]->fix_param->board_type == PPAL_BOARD_TYPE_US_SUB_SFP_PLUS)
		{
		
		   vid_base = ETH_LOCAL_PORT_SUBSLOT_COUNT(slot, 0) + sub_portnum + 2;
		   eth_index = eth_port_generate_ifindex(0, subslot, 0, 0, 0);
			
			ret = nam_asic_vlan_entry_ports_del(1, eth_index, 0);
			if(0 != ret)
			{
			   sprintf(line, "!Failed delete port 1/%d/1 from vlan 1 error, test stop.\n", subslot);
			   fputs(line, fp);
			   return MANUFACTURE_TEST_DELPORT_ERROR;
			}
            ret = nam_asic_vlan_entry_ports_add(eth_index, vid_base+subslot, 0);
            ret = nam_asic_set_port_pvid(eth_index, vid_base+subslot);
			if(0 != ret)
			{
			   sprintf(line, "!Failed add port 1/%d/1 to vlan %d error, test stop.\n", subslot,vid_base+subslot);
			   fputs(line, fp);
			   return MANUFACTURE_TEST_ADDPORT_ERROR;
			}
			sleep(4);
			
			ret = nam_set_ethport_enable(eth_index, 1);
			ret = nam_get_port_link_state(eth_index, &link_status);
			if(0 != ret)
				all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
			if(!link_status )
			{
				sprintf(line, "!Failed the subboard %d port 1/%d/1 is link down.\n", subslot,subslot);
				fputs(line, fp);
				if(0 == all_ret)
					all_ret |= MANUFACTURE_TEST_PORTLINKDOWN_ERROR;
				return all_ret;
			}
			ret = nam_asic_clear_port_pkt_stat(eth_index);
				if((0 != ret) && (0 == all_ret))
					all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
        	int i;
        	for (i = 0; i < 100; i++)
        	{
            	nam_packet_tx_unicast_by_netif(0, eth_index, vid_base+subslot, 0, (test_buffer + 64), 60);
        	}
    		ret = nam_asic_port_pkt_statistic(eth_index, &tx);
    		if((0 != ret) && (0 == all_ret))
        		all_ret |= MANUFACTURE_TEST_PORTAPI_ERROR;
			if(tx.snmp_stats.obytes == 0)
    		{
        		sprintf(line, "!Failed the subboard %d ports 1/%d/1 send no packets.\n", subslot,subslot);
        		fputs(line, fp);
        		if(0 == all_ret)
            		all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;
        
    		}
			if((tx.snmp_stats.obytes != tx.snmp_stats.ibytes) 
				|| (tx.snmp_stats.etheronoerrors != tx.snmp_stats.etherinoerrors))
			{
        		sprintf(line, "!Failed the subboard %d ports 1/%d/1 test lost packets.\n", subslot, subslot);
        		fputs(line, fp);
        		if(0 == all_ret)
            		all_ret |= MANUFACTURE_TEST_PORTLOSTPACKET_ERROR;                                                      
    		}
		}
	    
		
    }
	else
	{
	        sprintf(line, "!Failed the subboard %d is absence.\n", subslot);
		    fputs(line, fp);
			all_ret |= MANUFACTURE_TEST_SUBBOARD_ERROR;
			return all_ret;
	}
	return all_ret;
}

DBusMessage * npd_dbus_manufacture_board(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    FILE *fp;
	unsigned int ret;
	unsigned int op_ret = 0;
    unsigned int slot = 0;
    char path[32];
    char line[256];

    manu_testing = 1;
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                           	DBUS_TYPE_UINT32,&slot,                               							
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_product_err("Unable to get input args.\n ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_product_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        manu_testing = 0;

        return NULL;
    }

    sprintf(path, "/var/run/test.board%d", slot);
    fp = fopen(path, "a");
    if(NULL == fp)
    {
        syslog_ax_product_err("Unable to open file.\n ");
        ret = -2;
        goto retcode;
    }
    sprintf(line, "Beginning manufacture testing for board %d.\n", slot);
    fputs(line, fp);

    if(SYS_LOCAL_MODULE_SLOT_INDEX == slot-1)
    {
        ret = test_add_port_to_vlan(slot-1, fp);
        op_ret = ret;
    }
    ret = test_all_panel_port(slot-1, fp);
    op_ret |= ret;
	if (!app_box_state_get())
	{
		ret = test_all_xaui_port(slot-1,fp);
    	op_ret |= ret;
	}
	
    if(SYS_LOCAL_MODULE_SLOT_INDEX != slot-1)
        ret = -1;

retcode:
    fclose(fp);
    reply = dbus_message_new_method_return(msg);
	
    dbus_message_iter_init_append(reply, &iter);
	
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &op_ret);
    manu_testing = 0;
    
    return reply;

}

DBusMessage * npd_dbus_pre_manufacture_board(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    FILE *fp;
	unsigned int ret;
	unsigned int op_ret = 0;
    unsigned int slot = 0;
    char path[32];
    char line[256];
	
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                           	DBUS_TYPE_UINT32,&slot,                               							
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_product_err("Unable to get input args.\n ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_product_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        return NULL;
    }

    sprintf(path, "/var/run/test.board%d", slot);
    fp = fopen(path, "w+");
    if(NULL == fp)
    {
        syslog_ax_product_err("Unable to open file.\n ");
        ret = -2;
        goto retcode;
    }
    sprintf(line, "Pre beginning manufacture testing for board %d:\n", slot);
    fputs(line, fp);

	system("sudo /etc/init.d/syslog-ng stop");
    ret = test_add_port_to_vlan(slot-1, fp);
    op_ret = ret;
    if(SYS_LOCAL_MODULE_SLOT_INDEX != slot-1)
        ret = -1;

retcode:
    fclose(fp);
    reply = dbus_message_new_method_return(msg);
	
    dbus_message_iter_init_append(reply, &iter);
	
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &op_ret);
    
    return reply;

}
DBusMessage * npd_dbus_manufacture_subboard(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    DBusMessage* reply;
    DBusMessageIter  iter;
    FILE *fp;
	unsigned int ret;
	unsigned int op_ret = 0;
	unsigned int slot = 1;
    unsigned int subslot = 0;
    char path[32];
    char line[256];

    manu_testing = 1;
    DBusError err;
    dbus_error_init(&err);

    if (!(dbus_message_get_args(msg, &err,
	                           	DBUS_TYPE_UINT32,&subslot, 
                                DBUS_TYPE_INVALID)))
    {
        syslog_ax_product_err("Unable to get input args.\n ");
        if (dbus_error_is_set(&err))
        {
            syslog_ax_product_err("%s raised: %s",err.name,err.message);
            dbus_error_free(&err);
        }
        manu_testing = 0;

        return NULL;
    }

    sprintf(path, "/var/run/test.board%d", slot);
    fp = fopen(path, "w+");
    if(NULL == fp)
    {
        syslog_ax_product_err("Unable to open file.\n ");
        ret = -2;
        goto retcode;
    }
    sprintf(line, "Beginning manufacture testing for subboard %d.\n", subslot);
    fputs(line, fp);

    if(SYS_LOCAL_MODULE_SLOT_INDEX == slot-1)
    {
        ret = test_subboard_port(slot-1,subslot, fp);
        op_ret = ret;
    }
    if(SYS_LOCAL_MODULE_SLOT_INDEX != slot-1)
        ret = -1;

retcode:
    fclose(fp);
    reply = dbus_message_new_method_return(msg);
	
    dbus_message_iter_init_append(reply, &iter);
	
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &ret);
    dbus_message_iter_append_basic(&iter,
                                   DBUS_TYPE_UINT32,
                                   &op_ret);
    manu_testing = 0;
    
    return reply;

}

#ifdef __cplusplus
}
#endif
