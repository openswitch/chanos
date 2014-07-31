
#ifdef HAVE_ERPP
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "util/npd_list.h"
#include "lib/npd_bitop.h"
#include "quagga/thread.h"
#include "tipc_api/tipc_api.h"
#include <linux/tipc.h>
#include "nam/nam_rx.h"

#include <erpp/erpp_main.h>
#include <erpp/erpp_dbus.h>
#include <erpp/erpp_event.h>
#include <erpp/erpp_packet.h>
#include <erpp/erpp_timer.h>
#include <erpp/erpp_log.h>

int ERPP_LOCAL_MODULE_ISMASTERACTIVE = 0;
int ERPP_LOCAL_MODULE_ISMASTERSTANDBY = 1;
int ERPP_LOCAL_MODULE_SLOT_INDEX = 0;
unsigned char SysMac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int erpp_get_system_mac()
{
    int fd = -1;
    char buf[32];
    char tmp_buf[16];
    unsigned int tmp_sys_mac[6];
    int ret = -1;
    int ni = 0;
    int nj = 0;

    fd = open("/devinfo/mac", O_RDONLY);
    if (fd < 0) 
    {
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    memset(tmp_buf, 0, sizeof(tmp_buf));
    ret = read(fd, tmp_buf, 12);
    if (ret > 0)
    {
        for (ni = 0, nj = 0; ni < 12;)
        {
            buf[nj++] = tmp_buf[ni++];
            buf[nj++] = tmp_buf[ni++];
            buf[nj++] = ' ';
        }
        sscanf(buf, "%02x %02x %02x %02x %02x %02x", \
            &tmp_sys_mac[0], &tmp_sys_mac[1], &tmp_sys_mac[2], \
            &tmp_sys_mac[3], &tmp_sys_mac[4], &tmp_sys_mac[5]);
        
        SysMac[0] = (unsigned char)(tmp_sys_mac[0] & 0xff);
        SysMac[1] = (unsigned char)(tmp_sys_mac[1] & 0xff);
        SysMac[2] = (unsigned char)(tmp_sys_mac[2] & 0xff);
        SysMac[3] = (unsigned char)(tmp_sys_mac[3] & 0xff);
        SysMac[4] = (unsigned char)(tmp_sys_mac[4] & 0xff);
        SysMac[5] = (unsigned char)(tmp_sys_mac[5] & 0xff);
    }
    else
    {
        close(fd);
        return -1;
    }
    close(fd);
    
    return 0;
}
int erpp_dbtable_recv(int fd, char* buf, int len, void *private_data)
{
    if(len <= 0)
    {
		ERPP_LOCAL_MODULE_ISMASTERACTIVE = app_act_master_running();
        if(ERPP_LOCAL_MODULE_ISMASTERACTIVE == 0)
        {
            printf("standby becomes active \n");
            ERPP_LOCAL_MODULE_ISMASTERACTIVE = 1;
            ERPP_LOCAL_MODULE_ISMASTERSTANDBY = 0;
            return erpp_dbtable_init();
        }

        return 0;
    }

    dbtable_recv(fd, buf, len, private_data);
    return 0;
}

int erpp_dbtable_slot_event(int event, int service_type, int instance)
{
	int slot_index = INSTANCE_TO_SLOT(instance);
    int ret;

    if(event == TIPC_PUBLISHED)
    {
        printf("published \r\n");

        if(service_type != ERPP_SERVICE_MAGIC)
        {
            return -1;
        }

        if(ERPP_LOCAL_MODULE_ISMASTERACTIVE)
        {
            ret = dbtable_slot_online_insert(slot_index);
        }
    }
    else if(event == TIPC_WITHDRAWN)
    {
        printf("withdraw \r\n");
        return 0;
    }

    return 0;
}

int erpp_dbtable_main()
{    
    int ret = -1;
    ERPP_LOCAL_MODULE_SLOT_INDEX = app_local_slot_get();

    if(ERPP_LOCAL_MODULE_ISMASTERACTIVE)
    {
        ret = tipc_client_init(ERPP_SERVICE_MAGIC, erpp_dbtable_slot_event);
        if (ret == 0)
        {
            printf("sync master init:start client \r\n");
        }
        ret = tipc_server_init(ERPP_SERVICE_MAGIC, ERPP_LOCAL_MODULE_SLOT_INDEX, erpp_dbtable_recv, NULL, NULL);


        if (ret == 0)
        {
            printf("sync master init:start server \r\n");
        }
    }
    else if(ERPP_LOCAL_MODULE_ISMASTERSTANDBY)
    {
        ret = tipc_server_init(ERPP_SERVICE_MAGIC, ERPP_LOCAL_MODULE_SLOT_INDEX, erpp_dbtable_recv, NULL, NULL);

        if(ret == 0)
        {
            printf("sync standby init:start server \r\n");
        }
        ret = tipc_client_init(ERPP_SERVICE_MAGIC, erpp_dbtable_slot_event);
        if (ret == 0)
        {
            printf("sync standby init:start client \r\n");
        } 
    }
    else
    {
        ret = tipc_server_init(ERPP_SERVICE_MAGIC, ERPP_LOCAL_MODULE_SLOT_INDEX, erpp_dbtable_recv, NULL, NULL);
        if (ret == 0)
        {
            printf("sync service init:start server \r\n");
        }
    }
	osal_thread_read_buffer_length_set(ERPP_SERVICE_MAGIC, 2000);
    osal_thread_master_run(ERPP_SERVICE_MAGIC);
    return 0;
}

int main()
{

    if(app_slot_work_mode_get() != 1)
	{
		return 0;
	}
	
    ERPP_LOCAL_MODULE_ISMASTERACTIVE = app_act_master_running();

    if (ERPP_LOCAL_MODULE_ISMASTERACTIVE == 1)
    {
        ERPP_LOCAL_MODULE_ISMASTERSTANDBY = 0;
    }
    else
    {
        ERPP_LOCAL_MODULE_ISMASTERACTIVE = 0;
        ERPP_LOCAL_MODULE_ISMASTERSTANDBY = 1;
    }

    if (ERPP_LOCAL_MODULE_ISMASTERACTIVE)
    {
        erpp_syslog_dbg("ERPP: Service is running in active master mode.\n");
    }
    erpp_get_system_mac();
	erpp_syslog_init();
    if (0 != erpp_db_init())
    {
        erpp_syslog_dbg("ERPP: Failed to create database.\n");
        return -1;
    }

    if (0 != erpp_app_event_init())
    {
		erpp_syslog_dbg("ERPP: Failed to create event task.\n");
        return -1;
    }

    if (0 != erpp_msg_sock_init())
    {
		erpp_syslog_dbg("ERPP: Failed to create msg socket.\n");
        return -1;
    }

    if (0 != erpp_packet_socket_init())
    {
		erpp_syslog_dbg("ERPP: Failed to create packet socket.\n");
        return -1;
    }
        
    if (0 != erpp_dbus_init())
    {
		erpp_syslog_dbg("ERPP: Failed to create DBUS connection.\n");
        return -1;
    }

    if (0 != erpp_get_system_mac())
    {
		erpp_syslog_dbg("ERPP: Failed to get system mac.\n");
        return -1;
    }
	
    if (0 != erpp_timer_init())
    {
		erpp_syslog_dbg("ERPP: Failed to init timer.\n");
        return -1;
	}
    erpp_dbtable_init();
    app_module_inst_set("erpp",getpid());
    erpp_dbtable_main();

    return 0;
}

#endif

