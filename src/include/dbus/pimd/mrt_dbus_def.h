#ifndef __MRT_DBUS_DEF_H__
#define __MRT_DBUS_DEF_H__

/*added by scx 2009.12.17 for pimd dbus*/
#define PIMD_DBUS_BUSNAME "aw.pimd"
#define PIMD_DBUS_OBJPATH "/aw/pimd"
#define PIMD_DBUS_INTERFACE "aw.pimd"

#define PIMD_DBUS_INTERFACE_METHOD_SET_IGMPVER         "ip_set_igmpver"
#define PIMD_DBUS_INTERFACE_METHOD_GET_IGMPVER         "ip_get_igmpver"
#define PIMD_DBUS_INTERFACE_METHOD_PIM_SSM_RANGE_CONFIG "ip_pim_ssm_range_config"
#define PIMd_DBUG_INTERFACE_METHOD_BIDIR_PIM_ENABLE     "bidir_pim_enable"
#define PIMD_DBUS_INTERFACE_METHOD_IPMRT_ENABLE         "ip_mrt_enable"
#define PIMD_DBUS_INTERFACE_METHOD_IPMRT_GETSTATE        "ip_mrt_getstate"
#define PIMD_DBUS_INTERFACE_METHOD_IPPIM_ENABLE         "ip_pim_enable"
#define PIMD_DBUS_INTERFACE_METHOD_IPPIM_ENABLE_BYADDR    "ip_pim_enable_byaddr"
#define PIMD_DBUS_INTERFACE_METHOD_IPPIM_HELLO_OPTION     "ip_pim_hello_option"

#define PIMD_DBUS_INTERFACE_METHOD_PIMDM_STATE_REFRESH_ENABLE    "ip_pim_dm_state_refresh_enable"
#define PIMD_DBUS_INTERFACE_METHOD_PIMDM_STATE_REFRESH_INTERVAL   "ip_pim_dm_state_refresh_interval"
#define PIMD_DBUS_INTERFACE_METHOD_IPPIM_MODE_SELETE    "ip_pim_mode_selete"
#define PIMD_DBUS_INTERFACE_METHOD_IPMRT_STATIC         "ip_mrt_static"
#define PIMD_DBUS_INTERFACE_METHOD_NO_IPMRT_STATIC         "no_ip_mrt_static"
#define PIMD_DBUS_INTERFACE_METHOD_BSR_CANDIDATE         "ip_pim_bsr_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_NO_BSR_CANDIDATE         "no_ip_pim_bsr_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_BSR         "ip_pim_show_bsr"
#define PIMD_DBUS_INTERFACE_METHOD_SPT_SWITCH_THRESHOLD     "ip_spt_switch_threshold"

#define PIMD_DBUS_INTERFACE_METHOD_SHOW_PIM_INFO   "ip_pim_show_info"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_RP         "ip_pim_show_rp"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_STATIC         "ip_pim_show_rp_static"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_CANDIDATE         "ip_pim_show_rp_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_CANDIDATE_GRP     "ip_pim_show_rp_candidate_grp"

#define PIMD_DBUS_INTERFACE_METHOD_SET_RP_CANDIDATE         "ip_pim_set_rp_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_SET_RP_STATIC            "ip_pim_set_rp_static"
#define PIMD_DBUS_INTERFACE_METHOD_SET_RP_CANDIDATE_GRP     "ip_pim_set_rp_candidate_grp"


#define PIMD_DBUS_INTERFACE_METHOD_SHOW_IF         "ip_pim_show_interface"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_IF1         "ip_pim_show_interface1"

#define PIMD_DBUS_INTERFACE_METHOD_SHOW_NBR         "ip_pim_show_neighbor"

#define PIMD_DBUS_INTERFACE_METHOD_SHOW_MRT        "ip_pim_show_mrt"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_MODE       "ip_pim_show_mode"

#define PIMD_DBUS_INTERFACE_METHOD_DEBUG_MRT        "debug_ip_mrt"
#define PIMD_DBUS_INTERFACE_METHOD_DEBUG_WRITE        "ip_pimd_show_running"

#define PIMD_DBUS_INTERFACE_METHOD_QUERY_INTERVAL    "ip_pim_query_interval"

#define PIMD_DBUS_INTERFACE_METHOD_MESSAGE_INTERVAL    "ip_pim_message_interval"

#define PIMD_DBUS_INTERFACE_METHOD_CLEAR_IPMRT     "clear_ip_pim_mroute"

#define PIMD_DBUS_INTERFACE_METHOD_CLEAR_MFIB      "clear_ip_pim_mfib"

#endif /*end __MRT_DBUS_DEF_H__*/
