
#ifdef __cplusplus
extern "C"
{
#endif

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"


#include "bcm/stat.h"
#include "bcm/stack.h"
#include "bcm/trunk.h"
#include "bcm/vlan.h"
#include "bcm/port.h"
#include "bcm/mcast.h"
#include "bcm/ipmc.h"
#include "bcm/error.h"
#include "soc/drv.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>


#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_cplddef.h"
#include "nbm/nbm_api.h"
#include "board/ts_product_feature.h"
#include "nbm/nbm_cpld.h"
#include "os.h"
#include "npd_log.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"
#include "as2k_product_feature.h"

#include "as2k_product_init.h"

#include "as2k_product_info.h"

#ifdef HAVE_POE
#ifndef INCLUDE_I2C
#define INCLUDE_I2C
#include <bcm/bcmi2c.h>
#include <soc/i2c.h>
#endif
#endif


#define PRODUCT_TYPE_NAME	"Multi-Layer Core Intelligent Switch"

#define MODULE_NAME_FILE 	"/proc/sysinfo/module_name"
#define MODULE_SN_FILE 		"/proc/sysinfo/module_sn"

#define TEMP_VARIATION	3

unsigned char *base_mac = (unsigned char *)"001F641201FF";

#ifdef HAVE_POE
/* for cpu test use */
static int m_cpu_high_use = NPD_FALSE;
static int m_fan_alarm = NPD_FALSE;

typedef struct PACKED
{
	char name[20];
	unsigned int user;
	unsigned int nice;
	unsigned int system;
	unsigned int idle;
}CPU_OCCUPY;

static 
int as2k_cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n)
{
	unsigned long od, nd;
	unsigned id, sd;
	int cpu_use = 0;
	int cpu_use_change=0;
	
	od = (unsigned long)(o->user + o->nice + o->system + o->idle);
	nd = (unsigned long)(n->user + n->nice + n->system + n->idle);
	
	id = (unsigned long)(n->user - o->user);
	sd = (unsigned long)(n->system - o->system);
	
	cpu_use_change = nd - od;
	if (cpu_use_change != 0)
	{
		cpu_use = (int)((sd+id)*10000)/cpu_use_change;
	}
	else
	{
		cpu_use = 0;
	}
	return cpu_use;
}

static
int as2k_get_cpu_occupy(CPU_OCCUPY *cpust)
{
	FILE *fd;
	char buf[256];
	CPU_OCCUPY *cpu_occupy;
	cpu_occupy = cpust;
	
	fd = fopen("/proc/stat", "r");
	fgets(buf, sizeof(buf), fd);
	//npd_syslog_dbg("buf is %s", buf);
	sscanf(buf, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, 
		&cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle);
	
	fclose(fd);
	
	//npd_syslog_dbg("cpu %s user %d nice %d system %d idle %d\n", cpu_occupy->name, 
	//	cpu_occupy->user, 
	//	cpu_occupy->nice, cpu_occupy->system, cpu_occupy->idle);
	return NPD_SUCCESS;
}

static
int as2k_get_cpu_use()
{
  CPU_OCCUPY cpu_stat1;
  CPU_OCCUPY cpu_stat2;
  
  int cpu_use;
  
  as2k_get_cpu_occupy((CPU_OCCUPY *)&cpu_stat1);
  sleep(5);
  as2k_get_cpu_occupy((CPU_OCCUPY *)&cpu_stat2);
  
  cpu_use = as2k_cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
  
  //npd_syslog_dbg("cpu_use is %d.\n", cpu_use);
  return cpu_use;
}

static long as2k_poll_pne_thread(void)
{
	unsigned int cpu_usage = 0;

	npd_init_tell_whoami("pnd_monitor",0);

	do{
		sleep(1);
		cpu_usage = as2k_get_cpu_use();
		if (cpu_usage > 9000) /* */
		{
			m_cpu_high_use = NPD_TRUE;
		}
		else
		{
			m_cpu_high_use = NPD_FALSE;
		}
		
	}while(1);
}

int as2k_is_cpu_busy()
{
	return m_cpu_high_use;
}
void as2k_start_pne_monitor(void )
{
	nam_thread_create("pnd_monitor",
		(void *)as2k_poll_pne_thread,NULL,NPD_TRUE,NPD_FALSE);
}


void as2k_3200_fan_led_light(
	unsigned char param_ON,
	unsigned char param_OFF,
	unsigned char param_MASK,
	unsigned char param_IN_MASK,
	int param_i2c_addr,
	char* param_i2c_dev,
	int led_state)
{
	unsigned char bit_FAN_LED_ON	= param_ON;
	unsigned char bit_FAN_LED_OFF	= param_OFF;
	unsigned char bit_FAN_LED_MASK	= param_MASK;
	unsigned char bit_FAN_IN_MASK	= param_IN_MASK;
	
	int fd;
	unsigned char data;
	unsigned int nbytes = 1;
	int ret;
	int i2c_addr = param_i2c_addr;
	char * i2c_dev = param_i2c_dev;

#if 0	

	fd = bcm_i2c_open(0, i2c_dev, 0, 0);
	if (fd < 0)
	{
		npd_syslog_err("open i2c device %s error.\n", i2c_dev);
	}
	
	
	ret = bcm_i2c_read(0, fd, i2c_addr,
		(uint8 *)&data, (uint32 *)(&nbytes));	
	if (ret < 0)
	{
		npd_syslog_err("read i2c device %s error.\n", i2c_dev);
		//assert(0);
		return -1;
	}	

	data &= ~(bit_FAN_LED_MASK);
	if (led_state)
	{
		data |= bit_FAN_LED_ON; /* fan led on  */
	}
	else
	{
		data |= bit_FAN_LED_OFF; /* fan led on  */
	}

	/* when write back, the input mask must set to 1 */
	data |= bit_FAN_IN_MASK;
	nbytes = 1;
	ret = bcm_i2c_write(0, fd, i2c_addr,
		(uint8 *)(&data), (uint32)1);	
	if (ret < 0)
	{
		npd_syslog_err("read i2c device %s error.\n", i2c_dev);
		//assert(0);
		return -1;
	}	
#endif    
}



unsigned int  as2k_3200_fan_present(
	unsigned char param_PRESENT,
	int param_i2c_addr,
	char *param_i2c_dev)
{
	unsigned char bit_FAN_PRESENT = param_PRESENT;
	int fd;
	unsigned char data;
	unsigned int nbytes = 1;
	int ret;
	int i2c_addr = param_i2c_addr;
	char * i2c_dev = param_i2c_dev;
#if 0
	fd = bcm_i2c_open(0, i2c_dev, 0, 0);
	if (fd < 0)
	{
		npd_syslog_err("open i2c device %s error.\n", i2c_dev);
	}
	//printf(" before read i2c addr %x fan present data is %x.\n", param_i2c_addr, data);
	
	ret = bcm_i2c_read(0, fd, i2c_addr,
		(uint8 *)&data, (uint32 *)(&nbytes));	
	if (ret < 0)
	{
		npd_syslog_err("read i2c device %s error.\n", i2c_dev);
		//assert(0);
		return NPD_FALSE;
	}	
	
	//printf("i2c addr %x fan present data is %x.\n", param_i2c_addr, data);
	
#endif
	if (data & bit_FAN_PRESENT)
	{
		return NPD_FALSE;
	}
	else /* bit is zero, then fan is present */
	{
		return NPD_TRUE;
	}
}


unsigned int  as2k_3200_fan_alarm(
	unsigned char param_ALARM,
	int param_i2c_addr,
	char *param_i2c_dev)
{
	unsigned char bit_FAN_ALARM = param_ALARM;
	int fd;
	unsigned char data;
	unsigned int nbytes = 1;
	int ret;
	int i2c_addr = param_i2c_addr;
	char * i2c_dev = param_i2c_dev;
#if 0
	fd = bcm_i2c_open(0, i2c_dev, 0, 0);
	if (fd < 0)
	{
		npd_syslog_err("open i2c device %s error.\n", i2c_dev);
	}
	
	ret = bcm_i2c_read(0, fd, i2c_addr,
		(uint8 *)&data, (uint32 *)(&nbytes));	
	if (ret < 0)
	{
		npd_syslog_err("read i2c device %s error.\n", i2c_dev);
		//assert(0);
		return -1;
	}	
#endif	
	if (data & bit_FAN_ALARM) /* bit is not zero, then fan has alarm  */
	{
		return NPD_TRUE;
	}
	else /* bit is zero, then fan is normal */
	{
		return NPD_FALSE;
	}
}

unsigned  as2k_3200_power_led_light(
	unsigned char param_LED_YELLOW,
	unsigned char param_LED_OFF,
	unsigned char param_LED_GREEN,
	unsigned char param_LED_MASK,
	unsigned char param_IN_MASK,
	int param_i2c_addr,
	char * param_i2c_dev,
	int led_on,
	int led_color)
{
	unsigned char bit_PWR_LED_YELLOW = 	param_LED_YELLOW;
	unsigned char bit_PWR_LED_CLOSE  = 	param_LED_OFF;
	unsigned char bit_PWR_LED_GREEN  = 	param_LED_GREEN;
	unsigned char bit_PWR_LED_MASK 	 = 	param_LED_MASK;
	unsigned char bit_PWR_IN_MASK 	 = 	param_IN_MASK;
	
	int fd;
	unsigned char data;
	unsigned int nbytes = 1;
	int ret;
	int i2c_addr = param_i2c_addr;
	char * i2c_dev = param_i2c_dev;	
#if 0	
	
	fd = bcm_i2c_open(0, i2c_dev, 0, 0);
	if (fd < 0)
	{
		npd_syslog_err("open i2c device %s error.\n", i2c_dev);
	}
	
	
	ret = bcm_i2c_read(0, fd, i2c_addr,
		(uint8 *)&data, (uint32 *)(&nbytes));	
	if (ret < 0)
	{
		npd_syslog_err("read i2c device %s error.\n", i2c_dev);
		//assert(0);
		return -1;
	}
	data &= ~(bit_PWR_LED_MASK);
	
	if(led_on)
	{
		if (led_color==LED_YELLOW)
		{
			data |= bit_PWR_LED_YELLOW;
		}
		else
		{
			data |= bit_PWR_LED_GREEN;
		}
		
	}
	else /* led close */
	{
		data |= bit_PWR_LED_CLOSE;
	}

	/* when write back, the input BIT must set to 1 */
	data |= bit_PWR_IN_MASK;

	nbytes = 1;
	ret = bcm_i2c_write(0, fd, i2c_addr,
		(uint8 *)(&data), (uint32)1);	
	if (ret < 0)
	{
		npd_syslog_err("read i2c device %s error.\n", i2c_dev);
		//assert(0);
		return -1;
	}	
#endif    
	return NPD_SUCCESS;
}	


long as2k_3200_i2c_sfp_operate( 
			unsigned char TX_ON_param, 
			unsigned char TX_OFF_param, 
			unsigned char TX_MASK_param, 
			unsigned char IN_MASK_param,
			char * devName, 
			int addr, 
			int tx_enable)
{
	int fd = 0;
	unsigned char bit_TX_ON_param 	= 	TX_ON_param;
	unsigned char bit_TX_OFF_param 	= 	TX_OFF_param;
	unsigned char bit_TX_MASK  		= 	TX_MASK_param;
	unsigned char bit_IN_MASK  		= 	IN_MASK_param;
		
	unsigned char data = 0;
	unsigned int nbytes = 1;
	int rv = 0;
#ifdef HAVE_MEMORY_SHORT		
	if ((fd = bcm_i2c_open(0, devName, 0, 0)) < 0) 
	{
        int count = 0;
        
        int open_fail = 1;
        int boot_log_fd = -1;

        char boot_log_buf[128];

        boot_log_fd = open("/tmp/var/run/bootlog.i2c.cnt", O_CREAT | O_TRUNC | O_WRONLY, 0644);


        /* if i2c init failed, then probe it ten times */
        while (open_fail)
        {
            count++;
            memset(boot_log_buf, 0, sizeof(boot_log_buf));
            sprintf(boot_log_buf, "bcm i2c open try count(%d)\n", count);
			write(boot_log_fd, boot_log_buf, strlen(boot_log_buf));

            if (count > 10)
            {
                break;
            }
    		npd_syslog_err("Count %d Open %s failed fd %d, %s over 10 count.\n",
                count, devName, fd, bcm_errmsg(fd));
            
            soc_i2c_probe(0);
            sleep(1);
            if (fd = bcm_i2c_open(0, devName, 0, 0) < 0)
            {
                open_fail = 1;
            }
            else
            {
                open_fail = 0;
                break;
            }
            
        }
		close(boot_log_fd);
        
        if (open_fail)
        {
    		npd_syslog_err("Open %s failed fd %d, %s over 10 count.\n", devName, fd, bcm_errmsg(fd));
    		return NPD_FAIL;
        }
        
	}
		
	if ((rv = bcm_i2c_read(0, fd, 0, &data, &nbytes)) < 0)
	{
		npd_syslog_err("ERROR: dummy read byte failed: %s\n", bcm_errmsg(rv));
		return NPD_FAIL;
	}
	
	
	if (!(bit_TX_MASK & data))
	{
		npd_syslog_dbg("device sfp value has open");
		return NPD_SUCCESS;
	}
	data &= ~(bit_TX_MASK);
	if (tx_enable)
	{
		data |= bit_TX_ON_param;
	}
	else
	{
		data |= bit_TX_OFF_param;
	}

	/* when write back, the input BIT must set to 1 */
	data |= bit_IN_MASK;
	
	if ((rv = bcm_i2c_write(0, fd, 0, &data, 1)) < 0)
	{
		npd_syslog_err("ERROR: write byte failed: %s\n",bcm_errmsg(rv));
		return NPD_FAIL;
	}	

	
	if ((rv =  bcm_i2c_read(0, fd, 0, &data, &nbytes)) < 0)
	{
		npd_syslog_err("ERROR: dummy read byte failed: %s\n",bcm_errmsg(rv));
		return NPD_FAIL;		
	}
#endif	
	return NPD_SUCCESS;
}

#endif

unsigned long as2k_family_type_get()
{
	return FAMILY_TYPE_AS2000;
}

unsigned long as2k_product_hw_code_get()
{
	return PPAL_PRODUCT_HWCODE_AS2K_3200;
}

unsigned long as2k_local_module_hw_code_get()
{
	return nbm_get_module_type();
}

unsigned char as2k_board_hw_version_get()
{
	return nbm_get_board_hw_version();
}

unsigned char as2k_board_hw_slotno_get()
{
	return 0;
}

long as2k_load_backinfo(product_man_param_t *param)
{
	char  productName[100]; 
    npd_syslog_dbg("Get product sys info\n --%s", __FUNCTION__);
	
    param->basemac = (char*)base_mac;
    param->sn = "1000";
    param->name = "AS3224";
    param->sw_name = "AuteWare OS";
    param->enterprise_name = "Autelan";
    param->enterprise_snmp_oid = "1000";
	
/*    param->built_in_admin_name = "netgear";
    param->built_in_admin_passwd = "";
    
*/
    nbm_read_backplane_sysinfo(param);

	npd_syslog_dbg("enterprise name is %s, name is %s\n --%s", 
			param->enterprise_name, param->name);
	
	memset(productName, 0, sizeof(productName));
	strcat(productName, param->enterprise_name);
	strcat(productName, " ");
	strcat(productName, param->name);
	strcat(productName, " ");
	strcat(productName, PRODUCT_TYPE_NAME);

	if (param->name != NULL)
	{
		free(param->name);
		param->name = NULL;
	}

	param->name = strdup(productName);
	npd_syslog_dbg("Get product name is %s\n ", param->name );
	
    return NPD_SUCCESS;
}

void as2k_sys_reset()
{
    npd_syslog_err("reset now not be supported.\n");
}

void as2k_chassis_show()
{
    npd_syslog_err("chassis show now not be supported.\n"); 
}

long as2k_board_online(unsigned long slot_index)
{
	return nbm_board_online(slot_index);
}

long as2k_board_detect_start()
{
    return NPD_SUCCESS;
}

long as2k_master_board_online(unsigned long slot_index)
{
    return TRUE;
}

long as2k_board_reset(unsigned long slot_index)
{
	return nbm_board_reset(slot_index);
}

long as2k_board_poweroff(unsigned long slot_index)
{
    npd_syslog_err("board power off not be supported.\n");
    return NPD_SUCCESS;
}

long as2k_slotno_get()
{
	return nbm_slotno_get();
}


int as2k_read_sysinfo_file(char * filename, char * buf, int len)
{
	int fd;
	int size = 0;

	fd = open(filename,O_RDONLY);
	if (fd <= 0)
	{
		npd_syslog_err("open file error.\n", filename);
		return NPD_FAIL;
	}
	size = read(fd,buf,len);
	if (size < 0)
	{
		return NPD_FAIL;
	}
	
	if (NULL != strstr(buf, "error"))
	{
		return NPD_FAIL;
	}
	
	close(fd);
	return NPD_SUCCESS;	
}

int as2k_board_mnparam_get(board_man_param_t *info)
{
	char mod_name[32];
	char mod_sn [32];
	char * ptr;
	int ret = 0;
	int hw_code;
	
	hw_code = nbm_get_module_hw_code();
	if (-1 == hw_code)
	{
		npd_syslog_err("get module hwcode error.\n");
		return NPD_FAIL;
	}
	info->id = hw_code;
	
	ret = as2k_read_sysinfo_file(MODULE_NAME_FILE,mod_name, 32);
	if (ret != NPD_SUCCESS)
	{
		npd_syslog_err("get module name error.\n");
		return ret;
	}
	ptr = (char *)memchr(mod_name, '\n', 32);  
	*ptr = '\0';
	strncpy(info->modname, mod_name, strlen(mod_name));
	npd_syslog_dbg("modname is %s.\n",info->modname);

	
	ret = as2k_read_sysinfo_file(MODULE_SN_FILE, mod_sn, 32);
	if (ret != NPD_SUCCESS)
	{
		npd_syslog_err("get module sn error.\n");
		return ret;
	}
	ptr = (char *)memchr(mod_sn, '\n', 32);
	*ptr = '\0';
	strncpy(info->sn, mod_sn, strlen(mod_sn));
	npd_syslog_dbg("sn is %s.\n",info->sn);

	return NPD_SUCCESS;
}

void as2k_npd_master_set(void)
{
	nbm_master_set(TRUE);
}

int npd_get_modport_tid_by_global_index(unsigned int globle_index, int *tid, 
												unsigned char *mod, unsigned char *port)
{
	int ret, i;
 	unsigned int eth_g_index[8];
	unsigned int eth_count = 0;
	int count;
	unsigned int peer_slot, peer_type;
	
	*tid = 0;
	if (NPD_NETIF_TRUNK_TYPE == npd_netif_type_get(globle_index))
	{
	    *tid = npd_netif_trunk_get_tid(globle_index);
        return 0;
	}
	else
	{
    	ret = npd_get_modport_by_global_index(globle_index, mod, port);
        if (0 != ret)
        {
            npd_syslog_dbg("eth_index %#x get asic port failed for ret %d\n",globle_index,ret);
            return -1;
        }

		*tid = 0;
    }
	
	return 0;
}

long as2k_3200_local_conn_init(int product_type)
{
	SOC_PBMP_CLEAR(PBMP_ST_ALL(0));
	NUM_ST_PORT(0) = 0;
	CMIC_PORT(0) = 0;
	SOC_PBMP_CLEAR(PBMP_CMIC(0));
    SOC_PBMP_PORT_ADD(PBMP_CMIC(0), 0);

    snros_system_param->product_pp_feature = localmoduleinfo->fix_param->feature;
    
    return BCM_E_NONE;  
}

long as2k_3200_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}


int as2k_3200_set_com_config(int fd, int baud_rate, 
		int data_bits, char parity, int stop_bits)
{
	struct termios new_cfg, old_cfg;
	int speed;
	
	/* 保存并调试现有串口参数设置， 在这里如果串口号等出错，
	会有相关的出错信息 */
	if (tcgetattr(fd, &old_cfg) != 0)
	{
		npd_syslog_dbg("tcgetattr");
		return NPD_FAIL;	
	}
	
	/*  设置字符大小 */
	memcpy(&new_cfg, &old_cfg, sizeof(struct termios));
	cfmakeraw(&new_cfg); /* 配置为原始模式 */
	new_cfg.c_cflag &= ~CSIZE;
	
	/* 设置波特率 */
	switch(baud_rate)
	{
		case 2400:
			speed = B2400;
			break;
		case 4800:
			speed = B4800;
			break;
		case 9600:
			speed = B9600;
			break;
		case 19200:
			speed = B19200;	
			break;		
		case 38400:
			speed = B38400;	
			break;
		case 115200:
		default:
			speed = B115200;
			break;				
	}
	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);
	
	/* 设置停止位 */
	switch (data_bits)
	{
		case 7:
			new_cfg.c_cflag |= CS7;
			break;
		case 8:
		default:
			new_cfg.c_cflag |= CS8;
			break;
	}
	
	/* 设置奇偶校验位 */
	switch (parity)
	{

		case 'o':
		case 'O':
			new_cfg.c_cflag |= (PARODD | PARENB);
			new_cfg.c_iflag |= INPCK;
			break;	
			
		case 'e':
		case 'E':
			new_cfg.c_cflag |= PARENB;
			new_cfg.c_cflag &= ~PARODD;
			new_cfg.c_iflag |= INPCK;
			break;
		
		case 's': /* as no parity */
		case 'S': 
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_cflag &= ~CSTOPB;
			break;

		case 'n':
		case 'N':
		default:			
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_iflag &= ~INPCK;
			break;
					
	}
	
	/* 设置停止位 */
	switch (stop_bits)
	{
		case 2:
			new_cfg.c_cflag |= CSTOPB;
			break;
		case 1:
		default:
			new_cfg.c_cflag &= ~CSTOPB;
			break;	
	}
	
	/* 设置等待时间和最小接受字符 */
	new_cfg.c_cc[VTIME] = 10;
	new_cfg.c_cc[VMIN] = 1;
	
	/* 处理未接受字符 */
	tcflush(fd, TCIFLUSH);
	/* 激活新配置 */
	if ((tcsetattr(fd, TCSANOW, &new_cfg)) != 0)
	{
		printf("AS3209GC_PWR set_com_config tcsetattr failed.\n");
		npd_syslog_dbg("AS3209GC_PWR set_com_config tcsetattr failed.\n");
		return NPD_FAIL;	
	}
	return NPD_SUCCESS;
}


#ifdef __cplusplus
}
#endif

