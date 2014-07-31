#ifndef _UDLD_DBUS_H_
#define _UDLD_DBUS_H_

#define	NPD_DBUS_UDLD_OBJPATH	"/aw/npd/udld"
#define	NPD_DBUS_UDLD_INTERFACE	"aw.npd.udld"

#define UDLD_DBUS_BUSNAME		"aw.udld"
#define UDLD_DBUS_OBJPATH		"/aw/udld"
#define UDLD_DBUS_INTERFACE		"aw.udld"

#define UDLD_DBUS_METHOD_SET_GLOBAL_ENABLE "udld_set_global_enable"
#define UDLD_DBUS_METHOD_GET_GLOBAL_CONFIG "udld_get_global_config"
#define UDLD_DBUS_METHOD_SET_GLOBAL_MODE "udld_set_global_mode"
#define UDLD_DBUS_METHOD_SET_ECHO_TIMER "udld_set_ehco_timer"
#define UDLD_DBUS_METHOD_SET_ECHO_TIMEOUT "udld_set_echo_timeout"
#define UDLD_DBUS_METHOD_SET_ENHANCED_TIMER "udld_set_enhanced_timer"
#define UDLD_DBUS_METHOD_SET_ECHO_RETRY "udld_set_ehco_retry"
#define UDLD_DBUS_METHOD_SET_RECOVER_TIMER "udld_set_recover_timer"
#define UDLD_DBUS_METHOD_SET_NETIF_ENABLE "udld_set_netif_enable"
#define UDLD_DBUS_METHOD_GET_NETIF_CONFIG "udld_get_netif_config"
#define UDLD_DBUS_METHOD_SHOW_RUNNING "udld_show_running"

#define UDLD_DBUS_METHOD_SET_RECOVER_MODE "udld_set_recover_mode"
#define UDLD_DBUS_METHOD_SET_LOG_LEVEL  "udld_set_log_level"



#define UDLD_RETURN_CODE_0				(0)						/* success			*/
#define UDLD_RETURN_CODE_1				(1)						/* error				*/

#define UDLD_RETURN_CODE_BASE			(0x150000)				/* return code base 	*/
#define UDLD_RETURN_CODE_OK				(UDLD_RETURN_CODE_0)	/* success			*/
#define UDLD_RETURN_CODE_ERROR			(0x150001)				/* error 				*/
#define UDLD_RETURN_CODE_ALREADY_SET	(0x150002)				/* already been seted	*/
#define UDLD_RETURN_CODE_ENABLE_GBL		(0x150003)				/* UDLD enabled global	*/
#define UDLD_RETURN_CODE_NOT_ENABLE_GBL	(0x150004)				/* UDLD not enabled global		*/
#define UDLD_RETURN_CODE_NETIF_NOT_FOUND		(0x150005)				/* can't find the netif*/
#define UDLD_RETURN_CODE_SET_ECHO_FAILED		(0x150006)				/* faile to set UDLD echo timer interval*/
#define UDLD_RETURN_CODE_START_FAILED	(0x150007)				/* failed to start udld service	*/
#define UDLD_RETURN_CODE_SET_MODE_FAILED	(0x150008)				/* failed to set udld work mode*/
#define UDLD_RETURN_CODE_SET_TIMEOUT_FAILED	(0x150009)				/*failed to set timeout interval*/
#define UDLD_RETURN_CODE_SET_ENHANCED_FAILED (0x15000a)				/* failed to set enhanced timer interval*/
#define UDLD_RETURN_CODE_SET_RETRY_FAILED (0x15000b)				/* failed to set enhanced echo retry times*/
#define UDLD_RETURN_CODE_SET_RECOVER_FAILED		(0x15000c)				/* failed to set recover timer interval*/
#define UDLD_RETURN_CODE_HASH_TABLE_FULL (0x15000d)				/* hash table has full			*/
#define UDLD_RETURN_CODE_HASH_DUPLICATED (0x15000e)				/* hash table has duplicated	*/
#define UDLD_RETURN_CODE_HASH_NORESOURCE (0x15000f)				/* hash item alloc memory null	*/
#define UDLD_RETURN_CODE_HASH_NOTEXISTS	 (0x150010)				/* hash item not exist			*/
#define UDLD_RETURN_CODE_ALLOC_MEM_NULL  (0x150011)				/* alloc memory null			*/
#define UDLD_RETURN_CODE_HASH_FOUND		(0x150012)				/* found hash item			*/
#define UDLD_RETURN_CODE_HASH_NOTFOUND	(0x150013)				/* not found hash iteml			*/
#define UDLD_RETURN_CODE_SET_RECOVER_MODE_FAILED 	(0x150014) /* failed to set udld recover mode*/
#define UDLD_RETURN_CODE_SET_LOG_LEVEL_FAILED 	(0x150015)  /* failed to set udld log level*/

int udld_dbus_init();


#endif

