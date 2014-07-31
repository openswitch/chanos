#ifndef _VRRP_DBUS_H
#define _VRRP_DBUS_H

#define VRRP_DBUS_BUSNAME "aw.vrrp"
#define VRRP_DBUS_OBJPATH "/aw/vrrp"
#define VRRP_DBUS_INTERFACE "aw.vrrp"

#define VRRP_DBUS_METHOD_BRG_NO_DBUG_VRRP "vrrp_no_debug"
#define VRRP_DBUS_METHOD_BRG_DBUG_VRRP    "vrrp_debug"
#define VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE   "vrrp_service_enable"
#define VRRP_DBUS_METHOD_PROFILE_VALUE "vrrp_profile"
#define VRRP_DBUS_METHOD_PREEMPT_VALUE "vrrp_preempt"
#define VRRP_DBUS_METHOD_ADVERT_VALUE "vrrp_advert"
#define VRRP_DBUS_METHOD_VIRTUAL_MAC_VALUE "vrrp_virtual_mac"
#define VRRP_DBUS_METHOD_SHOW                "vrrp_show"
#define VRRP_DBUS_METHOD_SHOW_RUNNING                "vrrp_show_running"
#define VRRP_DBUS_METHOD_GET_GLOBAL          "vrrp_show_global"
#define VRRP_DBUS_METHOD_SET_MS_D_COUNT          "vrrp_set_ms_d_count"
#define VRRP_DBUS_METHOD_DELETE_ONE_VRRP         "vrrp_delete_one"
#define VRRP_DBUS_METHOD_ENABLE_BY_VRID   "vrrp_enable_by_vrid"
#define VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_VIP   "vrrp_link_add_del_vip"
#define VRRP_DBUS_METHOD_VRRP_LINK_ADD_DEL_TRACK	"vrrp_link_add_del_track"
#define VRRP_DBUS_METHOD_VRRP_LINK_TRACK_GROUP      "vrrp_link_track_group"

/* For wid */
#define VRRP_DBUS_METHOD_SET_TRANSFER_STATE          "vrrp_set_transfer_state"

void * vrrp_dbus_thread_main
(
	void *arg
);

#endif

