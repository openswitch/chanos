#ifndef __MRT_DBUS_DEF_H__
#define __MRT_DBUS_DEF_H__

#define DVMRP_DBUS_BUSNAME "aw.dvmrp"
#define DVMRP_DBUS_OBJPATH "/aw/dvmrp"
#define DVMRP_DBUS_INTERFACE "aw.dvmrp"

#define DVMRP_DBUS_INTERFACE_METHOD_IPMRT_ENABLE         "ip_mrtdvmrp_enable"
#define DVMRP_DBUS_INTERFACE_METHOD_IPMRT_GETSTATE         "ip_mrtdvmrp_getstate"
#define DVMRP_DBUS_INTERFACE_METHOD_IPDVMRP_ENABLE       "ip_dvmrp_enable_in_interface"
#define DVMRP_DBUS_INTERFACE_METRIC_SET                  "ip_dvmrp_metric_in_interface"
#define DVMRP_DBUS_INTERFACE_METHOD_IPDVMRP_ENABLE_BYADDR  "ip_dvmrp_enable_byaddr"
#define DVMRP_DBUS_NBR_TIMEOUT_SET                       "ip_dvmrp_nbr_timeout_value"
#define DVMRP_DBUS_PROBE_INTERVAL_SET                    "ip_dvmrp_probe_interval_time"
#define DVMRP_DBUS_REPORT_INTERVAL_SET                   "ip_dvmrp_report_interval_time"
#define DVMRP_DBUS_ROUTE_TIMEOUT_SET                     "ip_dvmrp_route_timeout_value"
#define DVMRP_DBUS_INTERFACE_METHOD_SHOW_MRT             "ip_dvmrp_mroute_show"
#define DVMRP_DBUS_INTERFACE_METHOD_SHOW_ROUTE           "ip_dvmrp_route_show"
#define DVMRP_DBUS_INTERFACE_METHOD_SHOW_NBR             "ip_dvmrp_nbr_show"
#define DVMRP_DBUS_INTERFACE_METHOD_SHOW_VIF             "ip_dvmrp_vif_show"
#define DVMRP_DBUS_CONFIG_SHOW                           "ip_dvmrp_config_show"
#define DVMRP_DBUS_INTERFACE_METHOD_DEBUG_WRITE          "ip_dvmrp_show_running"
 
#endif

