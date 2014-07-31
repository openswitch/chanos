#ifdef __cplusplus
extern "C"
{
#endif

#include "lib/osinc.h"


#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_bmapi.h"
#include "board/ts_product_feature.h"


board_conn_type_t *local_board_conn_type = NULL;
product_conn_type_t *backplane_type = NULL;

void ax_sal_config_init_defaults(void)
{
	if(snros_local_board_spec->asic_config_init)
	{
        (*snros_local_board_spec->asic_config_init)();
	}
}
int npd_get_peer_slot_unit_port(unsigned char myunit, unsigned char myport, int *peer_slot, unsigned char *peer_unit, unsigned char *peer_port)
{
	int my_plane_port = 0, peer_plane_port = 0;
	int peer_board_type = 0, peer_slot_no = 0;

	my_plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, myunit, myport); 
	if(my_plane_port == -1  || BOARD_INNER_CONN_PORT == my_plane_port)
	{
		npd_syslog_err("%s: Port %d:%d is not a plane port.\r\n", __func__, myunit, myport);
		return -1;
	}
	*peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, my_plane_port);
	if(*peer_slot == -1)
	{
		return -1;
	}
	peer_slot_no = CHASSIS_SLOT_INDEX2NO(*peer_slot);
	if(!CHASSIS_SLOTNO_ISLEGAL(peer_slot_no))
	{
		npd_syslog_err("%s: Can not get peer slot for local port: %d:%d.\r\n", __func__, myunit, myport);
		return -1;
	}
	npd_syslog_dbg("%s: Peer slot for local port %d:%d: %d.\r\n", __func__, myunit, myport, *peer_slot);
	if(chassis_slots[*peer_slot]->fix_param == NULL)
	{
		npd_syslog_err("%s: Can not get fixed parameter for slot %d.\r\n", __func__, *peer_slot);
		return -1;
	}
	peer_plane_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, my_plane_port);
	peer_board_type = chassis_slots[*peer_slot]->fix_param->board_type;
	npd_syslog_dbg("%s: Peer plane port for local port %d:%d: %d.\r\n", __func__, myunit, myport, peer_plane_port);
	if(peer_board_type > PPAL_BOARD_TYPE_DUMMY && peer_board_type < PPAL_BOARD_TYPE_NH_MAX)
	{
	    npd_syslog_dbg("%s: Peer board type for local port %d:%d: %s.\r\n", __func__, myunit, myport, chassis_slots[*peer_slot]->fix_param->short_name);
		*peer_unit = PPAL_PLANE_2_UNIT(peer_board_type, peer_plane_port);
		if(*peer_unit == (unsigned char)-1)
		{
		    npd_syslog_err("%s: Can not get peer unit for peer plane port %d.\r\n", __func__, peer_plane_port);
			return -1;
		}
		*peer_port = PPAL_PLANE_2_PORT(peer_board_type, peer_plane_port);
		if(*peer_port == (unsigned char)-1)
		{
		    npd_syslog_err("%s: Can not get peer port for peer plane port %d.\r\n", __func__, peer_plane_port);
			return -1;
		}
	    npd_syslog_dbg("%s: Peer slot:unit:port: %d:%d:%d.\r\n", __func__, *peer_slot, *peer_unit, *peer_port);
		return 0;
	}
	else
	{
		npd_syslog_err("%s: Can not get peer board type for local port %d:%d.\r\n", __func__, myunit, myport);
		return -1;
	}
}

int npd_get_peer_slot_unit_port_by_slot(int specified_slot, unsigned char myunit, unsigned char myport, int *peer_slot, unsigned char *peer_unit, unsigned char *peer_port, int *peer_type)
{
	int my_plane_port = 0, peer_plane_port = 0;
	int peer_board_type = 0, peer_slot_no = 0, specified_type = 0;
	
	if(!CHASSIS_SLOTNO_ISLEGAL(CHASSIS_SLOT_INDEX2NO(specified_slot)))
	{
		npd_syslog_err("%s: Specified slot %d is not legal.\r\n", __func__, specified_slot);
		return -1;
	}
	if(chassis_slots[specified_slot] == NULL)
	{
		npd_syslog_err("%s: Specified slot %d is not plugged in or configured.\r\n", __func__, specified_slot);
		return -1;
	}
	if(chassis_slots[specified_slot]->fix_param == NULL)
	{
		npd_syslog_err("%s: Specified slot %d is not plugged in or configured.\r\n", __func__, specified_slot);
		return -1;
	}
	if(chassis_slots[specified_slot]->inserted == 0 && MODULE_ONLINE_REMOVED_ON_SLOT_INDEX(specified_slot) == 0)
	{
		npd_syslog_err("%s: Specified slot %d is not plugged in or configured.\r\n", __func__, specified_slot);
		return -1;
	}
	specified_type = chassis_slots[specified_slot]->fix_param->board_type;
    if(specified_type <= PPAL_BOARD_TYPE_DUMMY || specified_type >= PPAL_BOARD_TYPE_NH_MAX)
    {
		npd_syslog_err("%s: Unkown board type: %d.\r\n", __func__, specified_type);
		return -1;
    }
	
	my_plane_port = PPAL_PHY_2_PLANE(specified_type, myunit, myport); 
	if(my_plane_port == -1 || BOARD_INNER_CONN_PORT == my_plane_port)
	{
		npd_syslog_err("%s: Port %d:%d is not a plane port.\r\n", __func__, myunit, myport);
		return -1;
	}
	*peer_slot = SLOT_PORT_PEER_SLOT(specified_slot, my_plane_port);
	if(*peer_slot == -1)
	{
		return -1;
	}
	peer_slot_no = CHASSIS_SLOT_INDEX2NO(*peer_slot);
	if(!CHASSIS_SLOTNO_ISLEGAL(peer_slot_no))
	{
		npd_syslog_err("%s: Can not get peer slot for local port: %d:%d.\r\n", __func__, myunit, myport);
		return -1;
	}
	npd_syslog_dbg("%s: Peer slot for local port %d:%d: %d.\r\n", __func__, myunit, myport, *peer_slot);
	if(chassis_slots[*peer_slot]->fix_param == NULL)
	{
		npd_syslog_err("%s: Can not get fixed parameter for slot %d.\r\n", __func__, *peer_slot);
		return -1;
	}
	peer_plane_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, my_plane_port);
	peer_board_type = chassis_slots[*peer_slot]->fix_param->board_type;
	*peer_type = peer_board_type;
	npd_syslog_dbg("%s: Peer plane port for local port %d:%d: %d.\r\n", __func__, myunit, myport, peer_plane_port);
	if(peer_board_type > PPAL_BOARD_TYPE_DUMMY && peer_board_type < PPAL_BOARD_TYPE_NH_MAX)
	{
	    npd_syslog_dbg("%s: Peer board type for local port %d:%d: %s.\r\n", __func__, myunit, myport, chassis_slots[*peer_slot]->fix_param->short_name);
		*peer_unit = PPAL_PLANE_2_UNIT(peer_board_type, peer_plane_port);
		if(*peer_unit == (unsigned char)-1)
		{
		    npd_syslog_err("%s: Can not get peer unit for peer plane port %d.\r\n", __func__, peer_plane_port);
			return -1;
		}
		*peer_port = PPAL_PLANE_2_PORT(peer_board_type, peer_plane_port);
		if(*peer_port == (unsigned char)-1)
		{
		    npd_syslog_err("%s: Can not get peer port for peer plane port %d.\r\n", __func__, peer_plane_port);
			return -1;
		}
	    npd_syslog_dbg("%s: Peer slot:unit:port: %d:%d:%d.\r\n", __func__, *peer_slot, *peer_unit, *peer_port);
		return 0;
	}
	else
	{
		npd_syslog_err("%s: Can not get peer board type for local port %d:%d.\r\n", __func__, myunit, myport);
		return -1;
	}
}

#ifdef __cplusplus
}
#endif


