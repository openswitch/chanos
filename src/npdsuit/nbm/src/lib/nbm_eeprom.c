
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_bmapi.h"
#include "nbm_eeprom.h"
#include "nbm_cpld.h"
#include "nbm_log.h"

#define DEFAULT_PRODUCT_NAME "CHANOS"
#define DEFAULT_SW_NAME "CHANOS"
#define DEFAULT_BASEMAC "000DEF000DEF"
#define DEFAULT_SYSSN "DEFsyssn1234"

#define DEFAULT_MODSN "DEFmodsn2222"
#define DEFAULT_MODNAME "DEFMODAX7-6GTX"

#define DEFAULT_MAINBOARD_NAME "DEFmb_MAINBOARD"
#define DEFAULT_MAINBOARD_SN "DEFmbsn33"
#define DEFAULT_AX7K_MAINBOARD_NAME	"DEFAX7K_CRSMU"
#define DEFAULT_AX7K_6GTX_NAME   "DEFAX7K_6GTX"
#define DEFAULT_AX7K_6GE_SFP_NAME   "DEFAX7K_6GE_SFP"
#define DEFAULT_AX7K_XFP_NAME   "DEFAX7K_XFP"
#define DEFAULT_AX7K_6GTX_POE_NAME   "DEFAX7K_6GTX_POE"

#define DEFAULT_AX5_NOMAINBOARD_NAME	"DEFAX5K"
#define DEFAULT_AX5I_NOMAINBOARD_NAME	"DEFAX5KI"
#define DEFAULT_AU4_NOMAINBOARD_NAME "DEFAU4K"

#define DEFAULT_BACKPLANE_NAME "DEFAX7K_BACKPLANE"
#define DEFAULT_BACKPLANE_SN "DEFbpsn23"

#define MODNAME_NULL  "null"
#define MODSN_NULL  MODNAME_NULL
#define SYSINFO_OEM_BTO 
#ifdef SYSINFO_OEM_BTO
#define SYSINFO_OEM_BTO_PRODUCT_NAME 	"/devinfo/product_name"
#define SYSINFO_OEM_BTO_SW_NAME 		"/devinfo/software_name"
#define SYSINFO_OEM_BTO_VENDOR_NAME 	"/devinfo/enterprise_name"
#define SYSINFO_OEM_BTO_VENDOR_SNMP_OID "/devinfo/enterprise_snmp_oid"
#define SYSINFO_OEM_BTO_SNMP_SYS_OID 		"/devinfo/snmp_sys_oid"
#define SYSINFO_OEM_BTO_MAC 			"/devinfo/mac"
#define SYSINFO_OEM_BTO_PROC_MAC        "/proc/sysinfo/product_base_mac_addr"
#define SYSINFO_OEM_BTO_ADMIN_USERNAME 	"/devinfo/admin_username"
#define SYSINFO_OEM_BTO_ADMIN_PASSWD 	"/devinfo/admin_passwd"

#define SYSINFO_OEM_BTO_LEN_PRODUCT_NAME 	64
#define SYSINFO_OEM_BTO_LEN_SW_NAME 		24
#define SYSINFO_OEM_BTO_LEN_VENDOR_NAME 	64
#define SYSINFO_OEM_BTO_LEN_VENDOR_SNMP_OID 128
#define SYSINFO_OEM_BTO_LEN_SNMP_SYS_OID 		128
#define SYSINFO_OEM_BTO_LEN_MAC 			12
#define SYSINFO_OEM_BTO_LEN_ADMIN_USERNAME 	32
#define SYSINFO_OEM_BTO_LEN_ADMIN_PASSWD 	32



typedef struct _sysinfo_oem_bto_info_ {
	char product_name[SYSINFO_OEM_BTO_LEN_PRODUCT_NAME];
	char sw_name[SYSINFO_OEM_BTO_LEN_SW_NAME];
	char enterprise_name[SYSINFO_OEM_BTO_LEN_VENDOR_NAME];
	char enterprise_snmp_oid[SYSINFO_OEM_BTO_LEN_VENDOR_SNMP_OID];
	char snmp_sys_oid[SYSINFO_OEM_BTO_LEN_SNMP_SYS_OID];
	char mac_addr[SYSINFO_OEM_BTO_LEN_MAC];
	char admin_username[SYSINFO_OEM_BTO_LEN_ADMIN_USERNAME];
	char admin_passwd[SYSINFO_OEM_BTO_LEN_ADMIN_PASSWD];	
}SYSINFO_OEM_BTO_INFO;

static SYSINFO_OEM_BTO_INFO g_oem_bto_sysinfo;
#endif

int nbm_read_backplane_sysinfo(struct product_sysinfo_s *product_sysinfo) {
    /* TODO read EEPROM and fill backplane sn and product sysinfo*/
	/*bm_op_backplane_sysinfo	 sysinfo;*/
	ax_product_sysinfo  sysinfo;
	int fd = -1;
	int result = 0;
	char *ptr = NULL;
	int len = 0;
    int get_base_mac_ok = 0;
	memset(&sysinfo, 0, sizeof(ax_product_sysinfo));

	if(g_bm_fd < 0)
	{
		fd = open(NPD_BM_FILE_PATH,0);
		if(fd < 0)
		{
			nbm_syslog_dbg("open dev %s error(%d) when read backplane sysinfo!\n",NPD_BM_FILE_PATH,fd);
		}
		g_bm_fd = fd;
	}
	else
	{
		result = ioctl(g_bm_fd,BM_IOC_BACKPLANE_SYSINFO_READ,&sysinfo);
		if (result != 0)
		{
			nbm_syslog_dbg("read backplane sysinfo error!\n");
		    memset(&sysinfo, 0, sizeof(ax_product_sysinfo));
		}
	}
	/* read module SN */
	ptr = (char *)malloc(SYSINFO_SN_LENGTH+1);
	if(NULL == ptr)
	{
		nbm_syslog_dbg("read backplane sysinfo alloc SN memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_SN_LENGTH+1);
	memcpy(ptr,sysinfo.ax_sysinfo_product_serial_no,SYSINFO_SN_LENGTH+1);
	ptr[SYSINFO_SN_LENGTH] = '\0';
	product_sysinfo->sn = ptr;
	/* read system base mac*/
	ptr = (char*)malloc(2*MAC_ADDRESS_LEN+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc MAC memory error\n");
		return -1;
	}
	memset(ptr,0,2*MAC_ADDRESS_LEN+1);
	memcpy(ptr,sysinfo.ax_sysinfo_product_base_mac_address,2*MAC_ADDRESS_LEN);
	ptr[2*MAC_ADDRESS_LEN] = '\0';
	product_sysinfo->basemac = ptr;

	/*read system name*/
	ptr = (char *)malloc(SYSINFO_PRODUCT_NAME+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc NAME memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_PRODUCT_NAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_product_name,SYSINFO_PRODUCT_NAME+1);
	ptr[SYSINFO_PRODUCT_NAME] = '\0';
	product_sysinfo->name = ptr;


	/*read system sw_name*/
	ptr = (char *)malloc(SYSINFO_SOFTWARE_NAME+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc sw_name memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_SOFTWARE_NAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_software_name,SYSINFO_SOFTWARE_NAME+1);
	ptr[SYSINFO_SOFTWARE_NAME] = '\0';
	product_sysinfo->sw_name = ptr;

	/*read system enterprise_name*/
	ptr = (char *)malloc(SYSINFO_ENTERPRISE_NAME+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc enterprise_name memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_ENTERPRISE_NAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_enterprise_name,SYSINFO_ENTERPRISE_NAME+1);
	ptr[SYSINFO_ENTERPRISE_NAME] = '\0';
	product_sysinfo->enterprise_name = ptr;

	/*read system enterprise_snmp_oid*/
	ptr = (char *)malloc(SYSINFO_ENTERPRISE_SNMP_OID+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc enterprise_snmp_oid memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_ENTERPRISE_SNMP_OID+1);
	memcpy(ptr,sysinfo.ax_sysinfo_enterprise_snmp_oid,SYSINFO_ENTERPRISE_SNMP_OID+1);
	ptr[SYSINFO_ENTERPRISE_SNMP_OID] = '\0';
	product_sysinfo->enterprise_snmp_oid = ptr;

	/*read system snmp_sys_oid*/
	ptr = (char *)malloc(SYSINFO_ENTERPRISE_SNMP_OID+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc enterprise_snmp_oid memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_ENTERPRISE_SNMP_OID+1);
	memcpy(ptr,sysinfo.ax_sysinfo_snmp_sys_oid,SYSINFO_ENTERPRISE_SNMP_OID+1);
	ptr[SYSINFO_ENTERPRISE_SNMP_OID] = '\0';
	product_sysinfo->snmp_sys_oid = ptr;

	/*read system built_in_admin_username*/
	ptr = (char *)malloc(SYSINFO_BUILT_IN_ADMIN_USERNAME+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc built_in_admin_username memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_BUILT_IN_ADMIN_USERNAME+1);
	memcpy(ptr,sysinfo.ax_sysinfo_built_in_admin_username,SYSINFO_BUILT_IN_ADMIN_USERNAME+1);
	ptr[SYSINFO_BUILT_IN_ADMIN_USERNAME] = '\0';
	product_sysinfo->built_in_admin_username = ptr;

	/*read system built_in_admin_passwd*/
	ptr = (char *)malloc(SYSINFO_BUILT_IN_ADMIN_PASSWORD+1);
	if(NULL == ptr) {
		nbm_syslog_dbg("read backplane sysinfo alloc built_in_admin_passwd memory error!\n");
		return -1;
	}
	memset(ptr,0,SYSINFO_BUILT_IN_ADMIN_PASSWORD+1);
	memcpy(ptr,sysinfo.ax_sysinfo_built_in_admin_password,SYSINFO_BUILT_IN_ADMIN_PASSWORD+1);
	ptr[SYSINFO_BUILT_IN_ADMIN_PASSWORD] = '\0';
	product_sysinfo->built_in_admin_passwd = ptr;

	ptr = NULL;

	/* if eeprom can not read mac, then generate it from elsewhere */
	if (strlen(product_sysinfo->basemac) == 0)
	{
		memset(&g_oem_bto_sysinfo, 0, sizeof(SYSINFO_OEM_BTO_INFO));

		/* read OEM BTO  mac address*/
		fd = open(SYSINFO_OEM_BTO_MAC,O_RDONLY);
		if(fd < 1) {
			nbm_syslog_dbg("get oem bto sysinfo mac open error\r\n");
			/* read OEM BTO  mac address form /proc/sysinfo/product_base_mac_addr*/
			fd = open(SYSINFO_OEM_BTO_PROC_MAC,O_RDONLY);
			if(fd < 1) {
				nbm_syslog_dbg("get oem bto proc/sysinfo mac open error\r\n");
			}
			else {
				read(fd, g_oem_bto_sysinfo.mac_addr,SYSINFO_OEM_BTO_LEN_MAC);
				/* override mac address read from eeprom*/
				if('\0' != g_oem_bto_sysinfo.mac_addr[0]) {
					if(((len = strlen(g_oem_bto_sysinfo.mac_addr))>0)&&(g_oem_bto_sysinfo.mac_addr[len-1]=='\n')){
	                      g_oem_bto_sysinfo.mac_addr[len-1] = '\0';
					}
					memcpy(product_sysinfo->basemac, g_oem_bto_sysinfo.mac_addr, SYSINFO_OEM_BTO_LEN_MAC);
					product_sysinfo->basemac[SYSINFO_OEM_BTO_LEN_MAC] = '\0';
					get_base_mac_ok = 1;
				}
				close(fd);
			}
		}
		else {
			read(fd, g_oem_bto_sysinfo.mac_addr,SYSINFO_OEM_BTO_LEN_MAC);
			/* override mac address read from eeprom*/
			if('\0' != g_oem_bto_sysinfo.mac_addr[0]) {
				if(((len = strlen(g_oem_bto_sysinfo.mac_addr))>0)&&(g_oem_bto_sysinfo.mac_addr[len-1]=='\n')){
	                  g_oem_bto_sysinfo.mac_addr[len-1] = '\0';
				}
				memcpy(product_sysinfo->basemac, g_oem_bto_sysinfo.mac_addr, SYSINFO_OEM_BTO_LEN_MAC);
				product_sysinfo->basemac[SYSINFO_OEM_BTO_LEN_MAC] = '\0';
				get_base_mac_ok = 1;
			}
			else{
				close(fd);
				/* read OEM BTO  mac address form /proc/sysinfo/product_base_mac_addr*/
				fd = open(SYSINFO_OEM_BTO_PROC_MAC,O_RDONLY);
				if(fd < 1) {
					nbm_syslog_dbg("get oem bto proc/sysinfo mac open error\r\n");
				}
				else {
					read(fd, g_oem_bto_sysinfo.mac_addr,SYSINFO_OEM_BTO_LEN_MAC);
					/* override mac address read from eeprom*/
					if('\0' != g_oem_bto_sysinfo.mac_addr[0]) {
						if(((len = strlen(g_oem_bto_sysinfo.mac_addr))>0)&&(g_oem_bto_sysinfo.mac_addr[len-1]=='\n')){
		                      g_oem_bto_sysinfo.mac_addr[len-1] = '\0';
						}
						memcpy(product_sysinfo->basemac, g_oem_bto_sysinfo.mac_addr, SYSINFO_OEM_BTO_LEN_MAC);
						product_sysinfo->basemac[SYSINFO_OEM_BTO_LEN_MAC] = '\0';
						get_base_mac_ok = 1;
					}
				}	
			}		
			close(fd);
		}
		if(!get_base_mac_ok)// generate random mac 
		{
			int num;
			char mac_tail[3];
				
			srand(time(NULL));
				
			num = (int)(rand()%256); 
			sprintf(mac_tail, "%.2x", num);
			mac_tail[0] = toupper(mac_tail[0]);
			mac_tail[1] = toupper(mac_tail[1]);
			product_sysinfo->basemac[SYSINFO_OEM_BTO_LEN_MAC-2] = mac_tail[0];
			product_sysinfo->basemac[SYSINFO_OEM_BTO_LEN_MAC-1] = mac_tail[1];
		}
	}

/*
#ifdef SYSINFO_OEM_BTO
#undef SYSINFO_OEM_BTO
#endif 
*/
#ifdef SYSINFO_OEM_BTO
	
	/*read OEM BTO product name */
	fd = open(SYSINFO_OEM_BTO_PRODUCT_NAME,O_RDONLY);
	if(fd < 1) {
		nbm_syslog_dbg("get oem bto sysinfo product name open error\r\n");
	}
	else {
		read(fd, g_oem_bto_sysinfo.product_name,SYSINFO_OEM_BTO_LEN_PRODUCT_NAME);
		/* override product name read from eeprom*/
		if('\0' != g_oem_bto_sysinfo.product_name[0] && strncmp(g_oem_bto_sysinfo.product_name, "BLANKDEVINFO", strlen("BLANKDEVINFO"))) {
			if(((len = strlen(g_oem_bto_sysinfo.product_name))>0)&&(g_oem_bto_sysinfo.product_name[len-1]=='\n')){
                  g_oem_bto_sysinfo.product_name[len-1] = '\0';
			}
			memcpy(product_sysinfo->name, g_oem_bto_sysinfo.product_name, SYSINFO_OEM_BTO_LEN_PRODUCT_NAME);
			product_sysinfo->name[SYSINFO_OEM_BTO_LEN_PRODUCT_NAME] = '\0';
		}
	}
	/*read OEM BTO sotfware name */
	fd = open(SYSINFO_OEM_BTO_SW_NAME,O_RDONLY);
	if(fd < 1) {
		nbm_syslog_dbg("get oem bto sysinfo software name open error\r\n");
	}
	else {
		read(fd, g_oem_bto_sysinfo.sw_name,SYSINFO_OEM_BTO_LEN_SW_NAME);
		/* override software name read from eeprom*/
		if('\0' != g_oem_bto_sysinfo.sw_name[0] && strncmp(g_oem_bto_sysinfo.sw_name, "BLANKDEVINFO", strlen("BLANKDEVINFO"))) {
			if(((len = strlen(g_oem_bto_sysinfo.sw_name))>0)&&(g_oem_bto_sysinfo.sw_name[len-1]=='\n')){
                  g_oem_bto_sysinfo.sw_name[len-1] = '\0';
			}
			memcpy(product_sysinfo->sw_name, g_oem_bto_sysinfo.sw_name, SYSINFO_OEM_BTO_LEN_SW_NAME);
			product_sysinfo->sw_name[SYSINFO_OEM_BTO_LEN_SW_NAME] = '\0';
		}
		close(fd);
	}	
	/*read OEM BTO enterprise name*/
	fd = open(SYSINFO_OEM_BTO_VENDOR_NAME,O_RDONLY);
	if(fd < 1) {
		nbm_syslog_dbg("get oem bto sysinfo vendor name open error\r\n");
	}
	else {
		read(fd, g_oem_bto_sysinfo.enterprise_name,SYSINFO_OEM_BTO_LEN_VENDOR_NAME);
		/* override enterprise name read from eeprom*/
		if('\0' != g_oem_bto_sysinfo.enterprise_name[0] && strncmp(g_oem_bto_sysinfo.enterprise_name, "BLANKDEVINFO", strlen("BLANKDEVINFO"))) {
			if(((len = strlen(g_oem_bto_sysinfo.enterprise_name))>0)&&(g_oem_bto_sysinfo.enterprise_name[len-1]=='\n')){
                  g_oem_bto_sysinfo.enterprise_name[len-1] = '\0';
			}
			memcpy(product_sysinfo->enterprise_name, g_oem_bto_sysinfo.enterprise_name, SYSINFO_OEM_BTO_LEN_VENDOR_NAME);
			product_sysinfo->enterprise_name[SYSINFO_OEM_BTO_LEN_VENDOR_NAME] = '\0';
		}
		close(fd);
	}	
	/*read OEM BTO enterprise snmp oid */
	fd = open(SYSINFO_OEM_BTO_VENDOR_SNMP_OID,O_RDONLY);
	if(fd < 1) {
		nbm_syslog_dbg("get oem bto sysinfo vendor snmp oid open error\r\n");
	}
	else {
		read(fd, g_oem_bto_sysinfo.enterprise_snmp_oid,SYSINFO_OEM_BTO_LEN_VENDOR_SNMP_OID);
		/* override enterprise snmp oid read from eeprom*/
		if('\0' != g_oem_bto_sysinfo.enterprise_snmp_oid[0] && strncmp(g_oem_bto_sysinfo.enterprise_snmp_oid, "BLANKDEVINFO", strlen("BLANKDEVINFO"))) {
			if(((len = strlen(g_oem_bto_sysinfo.enterprise_snmp_oid))>0)&&(g_oem_bto_sysinfo.enterprise_snmp_oid[len-1]=='\n')){
                  g_oem_bto_sysinfo.enterprise_snmp_oid[len-1] = '\0';
			}
			memcpy(product_sysinfo->enterprise_snmp_oid, g_oem_bto_sysinfo.enterprise_snmp_oid, SYSINFO_OEM_BTO_LEN_VENDOR_SNMP_OID);
			product_sysinfo->enterprise_snmp_oid[SYSINFO_OEM_BTO_LEN_VENDOR_SNMP_OID] = '\0';
		}
		close(fd);
	}
	/*read OEM BTO snmp sys oid */
	fd = open(SYSINFO_OEM_BTO_SNMP_SYS_OID,O_RDONLY);
	if(fd < 1) {
		nbm_syslog_dbg("get oem bto sysinfo snmp sys oid open error\r\n");
	}
	else {
		read(fd, g_oem_bto_sysinfo.snmp_sys_oid,SYSINFO_OEM_BTO_LEN_SNMP_SYS_OID);
		/* override snmp sys oid read from eeprom*/
		if('\0' != g_oem_bto_sysinfo.snmp_sys_oid[0] && strncmp(g_oem_bto_sysinfo.snmp_sys_oid, "BLANKDEVINFO", strlen("BLANKDEVINFO"))) {
			if(((len = strlen(g_oem_bto_sysinfo.snmp_sys_oid))>0)&&(g_oem_bto_sysinfo.snmp_sys_oid[len-1]=='\n')){
                  g_oem_bto_sysinfo.snmp_sys_oid[len-1] = '\0';
			}
			memcpy(product_sysinfo->snmp_sys_oid, g_oem_bto_sysinfo.snmp_sys_oid, SYSINFO_OEM_BTO_LEN_SNMP_SYS_OID);
			product_sysinfo->snmp_sys_oid[SYSINFO_OEM_BTO_LEN_SNMP_SYS_OID] = '\0';
		}
		close(fd);
	}

	/*read OEM admin user name */
	fd = open(SYSINFO_OEM_BTO_ADMIN_USERNAME,O_RDONLY);
	if(fd < 1) {
		nbm_syslog_dbg("get oem bto sysinfo admin user name open error\r\n");
	}
	else {
		read(fd, g_oem_bto_sysinfo.admin_username,SYSINFO_OEM_BTO_LEN_ADMIN_USERNAME);
		/* override snmp sys oid read from eeprom*/
		if('\0' != g_oem_bto_sysinfo.admin_username[0] && strncmp(g_oem_bto_sysinfo.admin_username, "BLANKDEVINFO", strlen("BLANKDEVINFO"))) {
			if(((len = strlen(g_oem_bto_sysinfo.admin_username))>0)&&(g_oem_bto_sysinfo.admin_username[len-1]=='\n')){
                  g_oem_bto_sysinfo.admin_username[len-1] = '\0';
			}
			/* for check the windows newline */
			if (g_oem_bto_sysinfo.admin_username[len-2] == '\r')
			{
				g_oem_bto_sysinfo.admin_username[len-2] = '\0';
			}
			memcpy(product_sysinfo->built_in_admin_username, g_oem_bto_sysinfo.admin_username, SYSINFO_OEM_BTO_LEN_ADMIN_USERNAME);
			product_sysinfo->built_in_admin_username[SYSINFO_OEM_BTO_LEN_ADMIN_USERNAME] = '\0';
		}
		close(fd);
	}

	/*read OEM admin user passwd */
	fd = open(SYSINFO_OEM_BTO_ADMIN_PASSWD,O_RDONLY);
	if(fd < 1) {
		nbm_syslog_dbg("get oem bto sysinfo admin passwd open error\r\n");
	}
	else {
		read(fd, g_oem_bto_sysinfo.admin_passwd,SYSINFO_OEM_BTO_LEN_ADMIN_PASSWD);
		/* override snmp sys oid read from eeprom*/
		if('\0' != g_oem_bto_sysinfo.admin_passwd[0] && strncmp(g_oem_bto_sysinfo.admin_passwd, "BLANKDEVINFO", strlen("BLANKDEVINFO"))) {
			if(((len = strlen(g_oem_bto_sysinfo.admin_passwd))>0)&&(g_oem_bto_sysinfo.admin_passwd[len-1]=='\n')){
                  g_oem_bto_sysinfo.admin_passwd[len-1] = '\0';
			}
			/* for check the windows newline */
			if (g_oem_bto_sysinfo.admin_passwd[len-2] == '\r')
			{
				g_oem_bto_sysinfo.admin_passwd[len-2] = '\0';
			}
			
			memcpy(product_sysinfo->built_in_admin_passwd, g_oem_bto_sysinfo.admin_passwd, SYSINFO_OEM_BTO_LEN_ADMIN_PASSWD);
			product_sysinfo->built_in_admin_passwd[SYSINFO_OEM_BTO_LEN_ADMIN_PASSWD] = '\0';
		}
		close(fd);
	}	

#endif 
	nbm_syslog_dbg("read EEPROM get sysinfo:\n");
	nbm_syslog_dbg("%-12s:%s\n","PRODUCT NAME",product_sysinfo->name);
	nbm_syslog_dbg("%-12s:%s\n","SYSTEM MAC",product_sysinfo->basemac);
	nbm_syslog_dbg("%-12s:%s\n","SERIAL No.",product_sysinfo->sn);
	nbm_syslog_dbg("%-12s:%s\n","SOFTWARE ID",product_sysinfo->sw_name);
	nbm_syslog_dbg("%-12s:%s\n","VENDOR NAME",product_sysinfo->enterprise_name);
	nbm_syslog_dbg("%-12s:%s\n","SYSTEM OID",product_sysinfo->snmp_sys_oid);
	nbm_syslog_dbg("%-12s:%s\n","SNMP OID",product_sysinfo->enterprise_snmp_oid);
	nbm_syslog_dbg("%-12s:%s\n","USERNAME",product_sysinfo->built_in_admin_username);
	nbm_syslog_dbg("%-12s:%s\n","PASSWORD",product_sysinfo->built_in_admin_passwd);

	return NPD_SUCCESS;
}


typedef struct nbm_boot_env
{
	char name[64];
	char value[128];
	int operation;
}nbm_boot_env_t;

//typedef nbm_boot_env_t boot_env_t;

int nbm_get_boot_img_name(char* imgname)
{
	int fd; 
	int retval; 
	nbm_boot_env_t env_args;	
	char *name = "bootfile";
	memset(&env_args, 0, sizeof(nbm_boot_env_t));
	sprintf(env_args.name,name);
	env_args.operation = 0;/*GET_BOOT_ENV;*/

	if(g_bm_fd < 0)
	{
		fd = open(NPD_BM_FILE_PATH,0);
		if(fd < 0)
		{
			nbm_syslog_dbg("open dev %s error(%d) when read main board sysinfo!\n",NPD_BM_FILE_PATH,fd);
			return -1;
		}
		g_bm_fd = fd;
	}	
	retval = ioctl(g_bm_fd,BM_IOC_ENV_EXCH,&env_args);

	if(retval == -1)
	{		
	
		return -2;	
	}
	else
	{		
		sprintf(imgname,env_args.value); 
	}
	return 0;
}

#ifdef __cplusplus
}
#endif
