/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* npd_main.c
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		NPD module main routine
*
* DATE:
*		12/21/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.49 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/osinc.h"

#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"

#include "board/ts_product_feature.h"
#include "chasm_manage_proto.h"

#include "lib/chassis_man_app.h"

#include "npd_dbus.h"
extern int osTimerWkAfter(int mils);
/*
  Sometimes we need version information to check compatibility, so this part is not customisable.
*/
#define SW_MAJ_V 1
#define SW_MIN_V 0
#define SW_BUI_V 0
#define SW_COM_V 0

unsigned char sw_maj_v = 1;
unsigned char sw_min_v = 0;
unsigned char sw_comp_v = 0;
unsigned short sw_build_v = 0;

int system_cpu_num = 1;
/**
  *  SW version and build number now read from RAMFS /etc/version directory files 'version' and 'buildno'
  *  all version number and build number saved as ASCII code
  */
#define SW_VERSION_FILE 	"/etc/version/version"
#define SW_BUILD_NO_FILE	"/etc/version/buildno"
#define SW_VERSION_SPILTER	'.'
#define SW_VERSION_BUFSIZE	128
#define SW_VERSION_PART     5

void npd_init_sw_version(void) {
	char buffer[SW_VERSION_BUFSIZE] = {0};
	int fd = 0, length = 0, i = 0,j = 0;
	unsigned char version[SW_VERSION_PART] = {0};
	unsigned short buildno = 0;

	/* set default value */	
	sw_maj_v = SW_MAJ_V;
	sw_min_v = SW_MIN_V;
	sw_comp_v  = SW_COM_V;
	sw_build_v = SW_BUI_V;
	
	/* read SW version */
	fd = open(SW_VERSION_FILE,0);
	if(fd < 0) {
		syslog_ax_main_err("open version file %s error when query SW version.\n",SW_VERSION_FILE);
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
			sw_maj_v = version[0];
			sw_min_v = version[1];
			sw_comp_v = version[2];
			sw_build_v = version[3];
		}
		close(fd);
	}

	/* read SW build number */
	fd = open(SW_BUILD_NO_FILE,0);
	if(fd < 0) {
		syslog_ax_main_err("open build number file %s error when query SW version.\n",SW_BUILD_NO_FILE);
	}
	else {
		length = read(fd,buffer,SW_VERSION_BUFSIZE);
		if(length) {
			for(i=0;i<length;i++) {
				if('\n' == buffer[i]) {/* just read first line*/
					break;
				}
				buildno *= (10);
				buildno += (buffer[i] - '0');
			}
			sw_build_v = buildno;
		}
		close(fd);
	}

	return;
}
/*
  NOTICE: 
	1. Sub-slot in a slot information will be processed specially.
		a. slot no part of port index will be defined based on .
		b. sub-slot information will be initialized in extend_info of card_info.
*/


unsigned int npd_query_sw_version(int board_type) 
{
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

	if(board_type == PPAL_BOARD_TYPE_FW9001)
	{	
		sprintf(version_file, "%s", "/mnt/fw/tos_version");			
	}
	else
	{
		sprintf(version_file, "%s", SW_VERSION_FILE);
		version[3] = sw_build_v;
	}
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


void npd_product_init_done()
{
    productinfo.chassis_state = CHASSIS_RUNNING;
}

#define _PATH_SYS_SYSTEM	"/sys/devices/system/cpu"
#define _PATH_SYS_CPU		_PATH_SYS_SYSTEM"/cpu"

int npd_system_cpu_num()
{
    char path[256];
    int cpu_count = 0;

    while(1)
    {
      sprintf(path, "%s%d", _PATH_SYS_CPU, cpu_count);
      if(0 == access(path, F_OK))
        cpu_count++;
      else
        break;
    }
    return cpu_count;
}

extern int npd_dbtable_thread_main();

int main(int argc,char **argv)
{
/*
	Tell my thread id
*/

	app_module_inst_init();

	npd_init_tell_whoami("NpdMain",0);

    system_cpu_num = npd_system_cpu_num();

	/* TODO: how many threads should be created here need to be considered carefully later*/
/*
      Open the syslog to record the init info
*/
    npd_log_init();   

    npd_log_set_debug_value(NPD_LOG_FLAG_ALL);

    db_table_init();
	npd_dbtable_sync_startup();
/* 
	Read software version and build number
 */
 	npd_init_sw_version();
	syslog_ax_main_dbg("system software version %d.%d.%d build %d\n",sw_maj_v,sw_min_v,sw_comp_v,sw_build_v);

/*
	Read cpld and get product id and module id information.
*/	

	syslog_ax_main_dbg("init product info\n");
	product_init();


/*	
	Probe chassis slot status and build slots data
*/
	syslog_ax_main_dbg("init chassis info\n");
	npd_init_chassis_slots();

    syslog_ax_main_dbg("init ASIC chip\n");
	npd_fw_engines_init();

/*
	Wait until ASIC initialization process done
*/
	npd_fw_engine_initialization_check();
	syslog_ax_main_dbg("Local ASIC initialization done!\n");

/*
    Initialize NETIF event structure
*/
    netif_event_init();

/*
	Build vlan data
*/
	syslog_ax_main_dbg("init vlan info\n");
	npd_init_vlan();

/*
	Build port data
*/
	syslog_ax_main_dbg("init eth port info\n");
	npd_init_eth_ports();

#ifdef HAVE_POE
	syslog_ax_main_dbg("init poe info\n");
	npd_poe_init();
#endif
#ifdef HAVE_SFLOW
	npd_sflow_init();
#endif
/*
	Build trunk data
*/
	syslog_ax_main_dbg("init trunk info\n");
	npd_init_trunks();

	syslog_ax_main_dbg("init switch port info\n");
    npd_init_switch_ports();

/* 
	Init socket for packet Rx/Tx
*/
	syslog_ax_main_dbg("init packet sockets\n");
	npd_init_packet_socket();


#ifdef HAVE_PVLAN
/*
	Init private vlan
*/
	syslog_ax_main_dbg("init pvlan info\n");
	npd_pvlan_init();
#endif

#ifdef HAVE_BRIDGE_STP
    syslog_ax_main_dbg("Init stp info\n");
    npd_init_stp();
#endif


#ifdef HAVE_ACL
/*
	Init Acl Rule structure
 */
	syslog_ax_main_dbg("init acl info\n");
#ifndef HAVE_DIFF_OS
    npd_qos_init();
#endif

#endif

#ifdef HAVE_UDLD
    npd_udld_init();
#endif

#ifdef HAVE_ERPP
    npd_erpp_init();
#endif
/*
	Init static FDB database
 */
 	syslog_ax_main_dbg("init fdb info\n");
 	npd_fdb_table_init();

#ifdef HAVE_MIRROR
/*
	Init mirror init();	
*/
	syslog_ax_main_dbg("init mirror\n");
	npd_mirror_init();
#endif

#ifdef HAVE_LACP
    syslog_ax_main_dbg("Init lacp info\n");
    npd_lacp_init();
#endif

#ifdef HAVE_IGMP_SNP
	syslog_ax_main_dbg("init igmp info\n");
    igmp_snp_init();
#endif


/*
	Build L3 interface data
*/
	syslog_ax_main_dbg("init intf info\n");
	npd_intf_init();


/*
	add DHCP table
*/
#ifdef HAVE_DHCP_SNP
    syslog_ax_main_dbg("init dhcp snooping\n");
	npd_dhcp_snp_init();
	npd_ip_source_guard_init();
#endif


#ifdef HAVE_IP_TUNNEL
/*
	Init x-over-ip tunnel 
*/
	syslog_ax_main_dbg("init tunnel info\n");
	npd_tunnel_init();
#endif

/*
	Init ARP Snooping Hash table
 */
	syslog_ax_main_dbg("init arp snooping info\n");
 	npd_init_arpsnooping();
#ifdef HAVE_NPD_IPV6	
/*
	Init neighbour Snooping Hash table
 */
	syslog_ax_main_dbg("init neighbour snooping info\n");
 	npd_init_neighbour_snooping();
#endif //HAVE_NPD_IPV6

#ifdef  HAVE_VRRP
/*
    npd init tracking group
*/
    npd_tracking_init();
#endif

#ifdef HAVE_ZEBRA
/*
	add netlink sysinfo
*/
	syslog_ax_main_dbg("init route info\n");
	npd_route_init();
#endif

#ifdef HAVE_DHCP_RELAY
	npd_dhcpr_init();
#endif	

#if defined(HAVE_NPD_IPV6) && defined(HAVE_DHCPV6_RELAY)
    npd_dhcpv6_relay_table_init();
#endif
/*
	Init dbus to get ready for accept management command
*/	
	syslog_ax_main_dbg("init dbus info\n");
	npd_dbus_init();

	syslog_ax_main_dbg("init chassis probe thread\n");
	npd_init_chassis_manage_thread();	

    syslog_ax_main_dbg("wait chassis ready\n");
	npd_chassis_manage_initialization_check();
	syslog_ax_main_dbg("chassis init done!\n");

	npd_eth_port_register_notifier_hook();

#ifdef HAVE_BRIDGE_STP
	syslog_ax_main_dbg("Create rstp msg thread\n");
	npd_rstp_msg_init();
#endif
#ifdef HAVE_IGMP_SNP
	syslog_ax_main_dbg("Create IGMP snooping msg thread\n");
	npd_igmp_snp_msg_init();
#endif

#ifdef HAVE_VRRP
    syslog_ax_main_dbg("Create vrrp thread\n");
	npd_vrrp_msg_init();
#endif
#ifdef HAVE_SMART_LINK
    syslog_ax_main_dbg("Create smart-link thread\n");
	npd_smart_link_msg_init();
#endif
#ifdef HAVE_IP_TUNNEL
	/* tunnel netlink */
	syslog_ax_main_dbg("Create Tunnel Net Link thread\n");
	nam_thread_create("TunnelNetlink",(void *)npd_tunnel_recv_netlink_msg,NULL,NPD_TRUE,NPD_FALSE);
#endif
#ifdef HAVE_DLDP
	syslog_ax_main_dbg("Create DLDP msg thread\n");
	npd_dldp_msg_init();
#endif
#ifdef HAVE_AAA
	syslog_ax_main_dbg("Create ASD msg thread\n");
	npd_asd_msg_init();
#endif
#ifdef HAVE_CAPWAP_ENGINE	
	syslog_ax_main_dbg("Create CAPWAP thread\n");
    npd_capwap_init();
#endif //HAVE_CAPWAP_ENGINE
#ifdef HAVE_M4_TUNNEL
	npd_ip_tunnel_init();
#endif
/* 
	check if system has started up correctly and then open asic interrupt
*/
	syslog_ax_main_dbg("Create SFP Polling thread\n");
	nam_thread_create("SysWaitSfpPoll",(void *)npd_check_system_startup_state,NULL,NPD_TRUE,NPD_FALSE);

	syslog_ax_main_dbg("Create Attack Check Polling thread\n");
	nam_thread_create("SysAttackCheck",(void *)npd_check_system_attack,NULL,NPD_TRUE,NPD_FALSE);
#if defined(HAVE_CHASSIS_SUPPORT) || defined(HAVE_STACKING)
    if(!SYS_LOCAL_MODULE_ISMASTERACTIVE)
        nam_thread_create("NpdDBSync",(void *)npd_dbtable_thread_main,NULL,NPD_TRUE,NPD_FALSE);
#endif
    npd_app_msg_run();
#ifndef HAVE_LOGTOOL_NONE
/*
	  close the syslog 
*/

	npd_log_set_debug_value(0);
#endif
	
	syslog_ax_main_dbg("Create NPD DBUS thread\n");

	nam_thread_create("NpdDBUS",(void *)npd_dbus_thread_main,NULL,NPD_TRUE,NPD_TRUE);
	while(1)
	{
		osTimerWkAfter(10000);
	    syslog_ax_main_dbg("Mainthread is running ... \n");
	}
	return 0;
}

#ifdef __cplusplus
}
#endif
