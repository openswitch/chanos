
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
* db_app_sync.h
*
*
* CREATOR:
*		chengjun@autelan.com
*
* DESCRIPTION:
*		Database sync between ACT and SBY for application.
*
* DATE:
*		10/27/2010
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.00 $
*******************************************************************************/

#ifndef __DB_APP_SYNC_H__
#define __DB_APP_SYNC_H__

#define APP_RUNNING_ON_ACTMASTER app_running_on_actmaster

extern unsigned int dbsync_app_service_type;
extern unsigned int app_running_on_actmaster;

extern int app_act_master_running();

extern int  app_local_slot_get();

extern int app_actmaster_slot_get();

extern int app_sbymaster_slot_get();

extern int* dbsync_monitor_sock_get();

extern int* dbsync_data_sock_array_get();

extern int dbsync_chassis_switchover();

extern int dbsync_recv(int *fd);


extern  int dbtable_sync_init(
    unsigned int tipc_service_type,
    int (*log)(int level, char *fmt, ...),
    int (*db_sync_complete_handler)()
  );

extern int dbsync_monitor(int *sock);

extern int dbtable_sync_slot_event(int event, int service_type, int slot_index);

#endif

