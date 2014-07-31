#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/syscall.h>


long fw9001_query_sw_version(void)
{
#define SW_VERSION_SPILTER	'.'	
#define SW_VERSION_BUFSIZE	128
#define SW_VERSION_PART     5

#define SW_MAJ_V 1
#define SW_MIN_V 0
#define SW_BUI_V 0
#define SW_COM_V 0

#define SW_INT_VER(maj,min,comp,build) 		\
	((((maj) & 0x0F) << 28) + (((min) & 0x7F) << 21) + (((comp) & 0x7F) << 14) + ((build) & 0x3FFF))


	char buffer[SW_VERSION_BUFSIZE] = {0};
	char version_file[128] = {0};
	unsigned int sw_maj_ver, sw_min_ver, sw_comp_ver,sw_build_ver;
	int fd = 0, length = 0, i = 0,j = 0;
	unsigned char version[SW_VERSION_PART] = {0};

	/* set default value */	
	sw_maj_ver = SW_MAJ_V;
	sw_min_ver = SW_MIN_V;
	sw_comp_ver  = SW_COM_V;
	sw_build_ver = SW_BUI_V;

	sprintf(version_file, "%s", "/mnt/fw/tos_version");			

	/* read SW version */
	fd = open(version_file,0);
	if(fd < 0) {
		syslog_ax_main_err("open version file %s error when query SW version.\n",version_file);
	}
	else {
		length = read(fd,buffer,SW_VERSION_BUFSIZE);
		if(length) {
			for(i = 0,j = 0; i < length; i++) {
				if(('\n' == buffer[i]) || 
					(SW_VERSION_SPILTER == buffer[i])) {
					j++;
				}
				else if(j < SW_VERSION_PART) { /* we need only 3 sub section of version number*/
					version[j] *= (10);
					version[j] += (buffer[i] - '0');
				}
			}/* end for(...)*/
			sw_maj_ver = version[0];
			sw_min_ver = version[1];
			sw_comp_ver = version[2];
			sw_build_ver = version[3];
		}
		close(fd);
	}
	
	return SW_INT_VER(sw_maj_ver,sw_min_ver,sw_comp_ver,sw_build_ver);
}

long fw9001_npd_get_image_info(char *name, char *version,unsigned int *time,char *board_type)
{

#define CHASM_DEFAULT_BUILD_TIME             1

	struct stat sb;
	unsigned int ver = 0;

    *time=CHASM_DEFAULT_BUILD_TIME;
	memset(name, 0, 1);
	
	if(stat("/mnt/fw/tos_version", &sb) < 0) /*确认是否支持Firewall*/
	{
		//memset(version, 0, 31);
		sprintf(version, "%s", "NOT_CHECK_VER");
		return NPD_SUCCESS;
	}
	ver = fw9001_query_sw_version();		
    snprintf(version, 31, "%x", ver);
    return NPD_SUCCESS;	
}

long fw9001_upgrade_thread(void *arg)
{
	unsigned int slot_index = 0;
	char thread_name[30] = {0};
	
	char img_file[256] = {0};
	char real_img_file[256] = {0};
	char remote_img_file[256] = {0};
	char cmd[300] = {0};
	char pidBuf[100] = {0};
	
	struct stat sb;
	
	if (arg == NULL)
		return NPD_SUCCESS;
	
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
	slot_index = ((unsigned int)arg) - 1;
	/* tell my thread id*/
	memset(thread_name, 0, 30);
	sprintf(thread_name, "NpdOsUpgrade.%d", slot_index+1);
		
	npd_init_tell_whoami(thread_name,0);

	memset(pidBuf, 0, 100);
    snprintf(pidBuf, 63, "%5d(tid: %d) %s %s\n", getpid(), (int)syscall(SYS_gettid), "-", thread_name);
	memset(cmd, 0, 300);
	sprintf(cmd, "echo '%s' > /var/run/%s\n", pidBuf, thread_name);
	system(cmd);

	memset(cmd, 0, 300);
	memset(img_file, 0, 256);
	memset(real_img_file, 0, 256);
	memset(remote_img_file, 0, 256);	



	if(stat("/mnt/fw/fw_upt_off", &sb) >= 0) /*确认是否支持Firewall*/
	{
		char filename[256] = {0}, real_file[256] = {0}, remote_file[256] = {0};
		
		sprintf(filename, "license.hc");
		sprintf(real_file, "/mnt/license/%s", filename);
		sprintf(remote_file, "/tmp/%s", filename);		
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, real_file, remote_file);
		system(cmd);

		sprintf(filename, "mp_hid_rand");
		sprintf(real_file, "/mnt/license/%s", filename);
		sprintf(remote_file, "/tmp/%s", filename);		
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, real_file, remote_file);
		system(cmd);

		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, "/devinfo/mac", "/tmp/mac");
		system(cmd);		
		return NPD_SUCCESS;
	}


	memset(img_file,0,256);
	sprintf(img_file, "%s", "fw_upt");
	sprintf(real_img_file, "/mnt/fw/%s", img_file);
	sprintf(remote_img_file, "/tmp/%s", img_file);
	{
		sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, real_img_file, remote_img_file);
		system(cmd);				
	}
	
	memset(img_file,0,256);
	sprintf(img_file, "%s", "fw_pkt");		
	sprintf(real_img_file, "/mnt/fw/%s", img_file);
	sprintf(remote_img_file, "/tmp/%s", img_file);
	{
		sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, real_img_file, remote_img_file);
		system(cmd);				
	}		

	sprintf(cmd, "remote_exec_client %d \"/tos/bin/update_local 1> /dev/null 2>&1\"\n", slot_index + 1);	
	system(cmd);				


	sleep(20);
	sprintf(cmd, "remote_exec_client %d reboot\n", slot_index + 1);
	system(cmd);	
	
	return NPD_SUCCESS;
}

long fw9001_npd_os_upgrade(unsigned int slot_index)
{
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
	char thread_name[30] = {0};
	
	chasm_board_cancel_timeout(slot_index);

	memset(thread_name, 0, 30);
	sprintf(thread_name, "NpdOsUpgrade.%d", slot_index+1);
	
	nam_thread_create(thread_name,
		(void *)fw9001_upgrade_thread, (void *)(slot_index+1),NPD_TRUE,NPD_FALSE);
	
	return NPD_SUCCESS;
}

long fw9001_ready_config_thread(void *arg)
{
	unsigned int slot_index = 0;
	char thread_name[30] = {0};
	char filename[256] = {0}, real_file[256], remote_file[256];
	char cmd[300] = {0};
	struct stat sb;	
	char pidBuf[100] = {0};
		
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
	slot_index = ((unsigned int)arg) - 1 ;
	/* tell my thread id*/
	memset(thread_name, 0, 30);
	sprintf(thread_name, "NpdReadyConfig.%d", slot_index+1);
	
	npd_init_tell_whoami(thread_name,0);
	
	memset(pidBuf, 0, 100);
    snprintf(pidBuf, 63, "%5d(tid: %d) %s %s\n", getpid(), (int)syscall(SYS_gettid), "-", thread_name);
	memset(cmd, 0, 300);
	sprintf(cmd, "echo '%s' > /var/run/%s\n", pidBuf, thread_name);
	system(cmd);
	
	sprintf(filename, "license.hc");
	sprintf(real_file, "/mnt/license/%s", filename);
	sprintf(remote_file, "/tmp/%s", filename);		
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, real_file, remote_file);
	system(cmd);

	sprintf(filename, "mp_hid_rand");
	sprintf(real_file, "/mnt/license/%s", filename);
	sprintf(remote_file, "/tmp/%s", filename);		
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, real_file, remote_file);
	system(cmd);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "file_client -n %d %s %s\n", slot_index + 1, "/devinfo/mac", "/tmp/mac");
	system(cmd);


	if(stat("/mnt/fw/fw_lic_off", &sb) >= 0) /* 确认是否进行防火墙板卡的licsence校验*/
	{
		return NPD_SUCCESS;
	}

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "remote_exec_client %d /tos/bin/lic_check\n", slot_index+1);
	system(cmd);
	
	return NPD_SUCCESS;
}

long fw9001_board_ready_config(unsigned int slot_index)
{	
	npd_syslog_dbg("********Entering function %s  ********\r\n", __FUNCTION__);
	
	char thread_name[30] = {0};
	memset(thread_name, 0, 30);
	sprintf(thread_name, "NpdOsUpgrade.%d", slot_index+1);
	
	nam_thread_create(thread_name,
		(void *)fw9001_ready_config_thread, (void *)(slot_index+1),NPD_TRUE,NPD_FALSE);
	
	return NPD_SUCCESS;
}

ipp_fix_param_t  fw9001_ipp_param =
{
    .ipp_portnum = 2,
    .ipp_phyport_map = {"eth0", "eth1"},
    .ipp_board_map = {-1, -2}
};

sub_board_fix_param_t fw9001_subboard_param = 
{
    .sub_slotnum = 1,     // more than 1
    .sub_slot_portnum = 0
};

temper_fix_param_t fw9001_temper_param =
{
    .num = 0,
    .name = {"", ""}
};

board_feature_t fw9001_feature = {0};

int fw9001_support_product[] =
{
    PRODUCT_T9010,
    PRODUCT_T9006,
    PRODUCT_T9003,
    PRODUCT_T9014,    
    0
};


board_fix_param_t fw9001_param =
{
    .board_code = PPAL_BOARD_HWCODE_FW9001,
    .board_type = PPAL_BOARD_TYPE_FW9001,
    .full_name = "AUTELAN AS SERIES MULTI-LAYER SWITCH FireWall BOARD",
    .short_name = "fw9001",

    .have_pp = TRUE,
    .master_flag = FALSE,
    .service_flag = SERVICE_AS_EXTERNAL_SYSTEM,
    .panel_portnum = 0,	/* */

    .ipp_fix_param = &fw9001_ipp_param,
    .subboard_fix_param = &fw9001_subboard_param,
    .temper_fix_param = &fw9001_temper_param,
    .os_upgrade = &fw9001_npd_os_upgrade,
    .board_ready_config = &fw9001_board_ready_config,
    .feature = &fw9001_feature,
    .sdk_type = SDK_TOPSEC,
    .board_support_product = (int*)fw9001_support_product,
    .get_image_info = &fw9001_npd_get_image_info,	
};

#ifdef __cplusplus
extern "C"
}
#endif

