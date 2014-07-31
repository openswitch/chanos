/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		APIs used in NPD for logging process.
*
* DATE:
*		02/21/2008	
*UPDATE:
*11/26/2010              zhengzw@autelan.com          Modified for multithread-safe.
*  FILE REVISION NUMBER:
*  		$Revision: 1.24 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nam/npd_api.h"
#include "npd_dbus.h"


pthread_mutex_t npd_log_mutex = PTHREAD_MUTEX_INITIALIZER;

/* NPD startup bootlog file descriptor*/
int	npd_bootlog_fd = -1;

/* file to save system startup log */
#define NPD_SYSTEM_STARTUP_LOG_PATH	"/var/run/bootlog.npd"


int npd_syslog_lock()
{
    return pthread_mutex_lock(&npd_log_mutex);
}

int npd_syslog_unlock()
{
    return pthread_mutex_unlock(&npd_log_mutex);
}


DECLARE_LOG_MODULE_SET(all, all);
DECLARE_LOG_MODULE_SET(asic, asic);
DECLARE_LOG_MODULE_SET(lag, trunk);
DECLARE_LOG_MODULE_SET(mac-address-table, fdb);
DECLARE_LOG_MODULE_SET(ethernet, eth_port);
DECLARE_LOG_MODULE_SET(switchport, switchport);
DECLARE_LOG_MODULE_SET(vlan, vlan);
DECLARE_LOG_MODULE_SET(mirror, mirror);
DECLARE_LOG_MODULE_SET(intf, intf);
DECLARE_LOG_MODULE_SET(pvlan, pvlan);
DECLARE_LOG_MODULE_SET(l2-mcast, igmp);
DECLARE_LOG_MODULE_SET(route, route);
DECLARE_LOG_MODULE_SET(arp-snooping, arpsnooping);
DECLARE_LOG_MODULE_SET(acl, acl);
DECLARE_LOG_MODULE_SET(qos, qos);
DECLARE_LOG_MODULE_SET(mng-bus, dbus);
DECLARE_LOG_MODULE_SET(slot, cslot);
DECLARE_LOG_MODULE_SET(sub-slot, e_slot);
DECLARE_LOG_MODULE_SET(product, product);
DECLARE_LOG_MODULE_SET(main, main);
DECLARE_LOG_MODULE_SET(stp, rstp);
DECLARE_LOG_MODULE_SET(dldp, dldp);
DECLARE_LOG_MODULE_SET(dhcp, dhcp_snp);
DECLARE_LOG_MODULE_SET(dot1x, asd);


DECLARE_LOG_MODULE_SET_AX(trunk)
DECLARE_LOG_MODULE_SET_AX(fdb)
DECLARE_LOG_MODULE_SET_AX(eth_port)
DECLARE_LOG_MODULE_SET_AX(intf)
DECLARE_LOG_MODULE_SET_AX(pvlan)
DECLARE_LOG_MODULE_SET_AX(vlan)
DECLARE_LOG_MODULE_SET_AX(igmp)
DECLARE_LOG_MODULE_SET_AX(route)
DECLARE_LOG_MODULE_SET_AX(arpsnooping)
DECLARE_LOG_MODULE_SET_AX(acl)
DECLARE_LOG_MODULE_SET_AX(qos)
DECLARE_LOG_MODULE_SET_AX(e_slot)
DECLARE_LOG_MODULE_SET_AX(mirror)
DECLARE_LOG_MODULE_SET_AX(dbus)
DECLARE_LOG_MODULE_SET_AX(product)
DECLARE_LOG_MODULE_SET_AX(rstp)
DECLARE_LOG_MODULE_SET_AX(main)
DECLARE_LOG_MODULE_SET_AX(dldp)
DECLARE_LOG_MODULE_SET_AX(dhcp_snp)
DECLARE_LOG_MODULE_SET_AX(asd)

DECLARE_LOG_MODULE_SET_NAM(trunk)
DECLARE_LOG_MODULE_SET_NAM(fdb)
DECLARE_LOG_MODULE_SET_NAM(eth_port)
DECLARE_LOG_MODULE_SET_NAM(acl)
DECLARE_LOG_MODULE_SET_NAM(asic)
DECLARE_LOG_MODULE_SET_NAM(vlan)
DECLARE_LOG_MODULE_SET_NAM(mirror)
DECLARE_LOG_MODULE_SET_NAM(route)
DECLARE_LOG_MODULE_SET_NAM(intf)


npd_log_module_t *conf_npd_log_module[] =
{
    &NPD_LOG_MODULE(all),
    &NPD_LOG_MODULE(asic),
    &NPD_LOG_MODULE(trunk),
    &NPD_LOG_MODULE(fdb),
    &NPD_LOG_MODULE(eth_port),
    &NPD_LOG_MODULE(switchport),
    &NPD_LOG_MODULE(vlan),
    &NPD_LOG_MODULE(mirror),
    &NPD_LOG_MODULE(intf),
    &NPD_LOG_MODULE(pvlan),
    &NPD_LOG_MODULE(igmp),
    &NPD_LOG_MODULE(route),
    &NPD_LOG_MODULE(arpsnooping),
    &NPD_LOG_MODULE(acl),
    &NPD_LOG_MODULE(qos),
    &NPD_LOG_MODULE(dbus),
    &NPD_LOG_MODULE(cslot),
    &NPD_LOG_MODULE(e_slot),
    &NPD_LOG_MODULE(product),
    &NPD_LOG_MODULE(main),
    &NPD_LOG_MODULE(rstp),
    &NPD_LOG_MODULE(dldp),
    &NPD_LOG_MODULE(dhcp_snp),
    &NPD_LOG_MODULE(asd)
};

#define ZLOG_NPD  101
int npd_log_init()
{
  
  /* Set umask before anything for security */
  umask (0027);


  zlog_default = openzlog ("npd", ZLOG_NPD,
			   LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);

  zlog_set_level(zlog_default, ZLOG_DEST_SYSLOG, LOG_DEBUG);
    
  return 0;
}

/**********************************************************************************
 *	npd_log_set_debug_value
 * 
 *  DESCRIPTION:
 *	This function set up one npd debug level
 *
 *  INPUT:
 * 	 	val_mask - debug level value stands for one level 
 *  
 *  OUTPUT:
 * 	 NULL
 *
 *  RETURN:
 * 	 1 - if debug level has already been set before.
 *	 0 - debug level setup successfully.
 *
 **********************************************************************************/
int npd_log_set_debug_value
(
	unsigned int val_mask
)
{
    int i = 0;
    for(i = 0; i < sizeof(conf_npd_log_module)/sizeof(conf_npd_log_module[0]); i++)
        conf_npd_log_module[i]->interval_flag = val_mask;

    return 0;
}

int npd_asic_debug_set
(
	unsigned int level
)
{
	return nam_asic_log_level_set(level);
}

int npd_asic_debug_clr
(
	unsigned int level
)
{
	return nam_asic_log_level_clr(level);
}

DBusMessage * npd_system_debug_enable(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =NPD_ERR ;
	unsigned int flag = 0;
    char *user_name;
    char *flag_name;
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_STRING,&user_name,
								DBUS_TYPE_STRING,&flag_name,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{
			dbus_error_free( &err );
		}
		return NULL;	
	}

    {
        int i = 0;
        for(i = 0; i < sizeof(conf_npd_log_module)/sizeof(conf_npd_log_module[0]); i++)
        {
            if(0 == strncmp(conf_npd_log_module[i]->module_user_name, user_name, strlen(user_name)))
            {
                if(1 == flag)
                {
                    if(0 == strncmp(flag_name, "event", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag |= NPD_LOG_FLAG_EVENT;
                    }
                    if(0 == strncmp(flag_name, "error", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag |= NPD_LOG_FLAG_ERR;
                    }
                    if(0 == strncmp(flag_name, "warning", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag |= NPD_LOG_FLAG_WARNING;
                    }
                    if(0 == strncmp(flag_name, "debug", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag |= NPD_LOG_FLAG_DBG;
                    }
                    if(0 == strncmp(flag_name, "packet_rcv", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag |= NPD_LOG_FLAG_PACKET_RCV;
                    }
                    if(0 == strncmp(flag_name, "packet_snd", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag |= NPD_LOG_FLAG_PACKET_SND;
                    }
                    if(0 == strncmp(flag_name, "packet_all", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag |= NPD_LOG_FLAG_PACKET_SND|NPD_LOG_FLAG_PACKET_RCV;
                    }
                }
                if(0 == flag)
                {
                    if(0 == strncmp(flag_name, "event", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag &= ~NPD_LOG_FLAG_EVENT;
                    }
                    if(0 == strncmp(flag_name, "error", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag &= ~NPD_LOG_FLAG_ERR;
                    }
                    if(0 == strncmp(flag_name, "warning", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag &= ~NPD_LOG_FLAG_WARNING;
                    }
                    if(0 == strncmp(flag_name, "debug", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag &= ~NPD_LOG_FLAG_DBG;
                    }
                    if(0 == strncmp(flag_name, "packet_rcv", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag &= ~NPD_LOG_FLAG_PACKET_RCV;
                    }
                    if(0 == strncmp(flag_name, "packet_snd", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag &= ~NPD_LOG_FLAG_PACKET_SND;
                    }
                    if(0 == strncmp(flag_name, "packet_all", strlen(flag_name)))
                    {
                        conf_npd_log_module[i]->interval_flag &= ~(NPD_LOG_FLAG_PACKET_SND|NPD_LOG_FLAG_PACKET_RCV);
                    }
                }            
            }
        }
    }

	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
		
	return reply;
	
}

DBusMessage* npd_dbus_asic_syslog_debug(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =0 ;
	unsigned int flag = 0;
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{

			dbus_error_free( &err );
		}
		return NULL;	
	}

	npd_asic_debug_set(flag);
	
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply;
}

DBusMessage* npd_dbus_asic_syslog_no_debug(DBusConnection *conn, DBusMessage *msg, void *user_data) {
	
	DBusMessage* reply;
	DBusError err;
	unsigned int ret =0 ;
	unsigned int flag = 0;
	
	dbus_error_init( &err );
	if(!(dbus_message_get_args(msg,&err,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_INVALID)))
	{
		if(dbus_error_is_set( &err ))
		{

			dbus_error_free( &err );
		}
		return NULL;	
	}

	npd_asic_debug_clr(flag);
	
	reply = dbus_message_new_method_return(msg);
    dbus_message_append_args(reply,
							 DBUS_TYPE_UINT32,&ret,
							 DBUS_TYPE_INVALID);
	
	return reply;
}
	
void npd_syslog_official_event(char *format,...)
{
	va_list ptr;

	/* buffering log*/
	va_start(ptr, format);
    npd_syslog_lock();
    vzlog(NULL, LOG_NOTICE, format, ptr);
    npd_syslog_unlock();
	va_end(ptr);

	return;	
}

#ifdef __cplusplus
}
#endif

