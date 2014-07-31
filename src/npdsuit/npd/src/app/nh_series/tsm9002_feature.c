#ifdef __cplusplus
extern "C"
{
#endif
#include "tsm9002_led_code.c"
#include <linux/mii.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <unistd.h>


extern int  nam_port_tos_block_set(unsigned char unit, unsigned char portnum);
extern int  nam_port_tos_block_restore(unsigned char unit, unsigned char portnum);
extern int chassis_only_one_mcu_running();


long tsm9002_sfp_online(unsigned long panel_port)
{
    return NPD_SUCCESS;
}

long tsm9002_sfp_detect_start()
{
    return NPD_SUCCESS;    
}

long tsm9002_sfp_info_get(unsigned long panel_port, fiber_module_man_param_t *param)
{
    return NPD_SUCCESS;
}

long tsm9002_slotno_get()
{
	return nbm_slotno_get();
}

long tsm9002_local_reset()
{
    nbm_local_reset();
	system("reboot");
	return NPD_SUCCESS;
}

long tsm9002_reset_type()
{
    npd_syslog_err("NH MCU reset type get not be supported.\n");
    return NPD_SUCCESS;
}

long tsm9002_led_lighting(unsigned long status)
{
    npd_syslog_err("NH TGM9002 led lighting not be supported.\n");
    return NPD_SUCCESS;
}

void tsm9002_mng_port_led_init()
{
	struct mii_ioctl_data mii_ioctl_data_t = {0};
	struct ifreq ifr;
	int fd;
	int err;
    int i = 0;
	for(i = 0; i < 2; i++)
	{
    	/* Setup our control structures. */
    	memset(&ifr, 0, sizeof(ifr));
    	sprintf(ifr.ifr_name, "%s%d", "mng", i);
    
    	/* Open control socket. */
    	fd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (fd < 0)
    	{
    		perror("Cannot get control socket");
    		return;
    	}
		if(i == 0)
		{
		    mii_ioctl_data_t.phy_id = 1;
		}
		else
		{
		    mii_ioctl_data_t.phy_id = 0;
		}
		mii_ioctl_data_t.reg_num = 0x1C;
		mii_ioctl_data_t.val_in = 0xA410;
		memcpy(&ifr.ifr_ifru, &mii_ioctl_data_t, sizeof(struct mii_ioctl_data));
		err = ioctl(fd, SIOCSMIIREG, &ifr);
		close(fd);
	}

}

long tsm9002_asic_led_proc(int unit)
{
    npd_syslog_dbg("NH TGM9002 led code loading.\n");
	tsm9002_mng_port_led_init();
	return soc_ledproc_config(unit, ledproc_tsm9002, sizeof(ledproc_tsm9002));
}

long tsm9002_pne_mon_start()
{
    npd_syslog_err("NH MCU p&e monitor not be supported.\n");
    return NPD_SUCCESS;
}

long tsm9002_mnparam_get(board_man_param_t *info)
{
	npd_syslog_dbg("TSM9002 man param get.\n");
	
    info->id = PPAL_BOARD_HWCODE_TSM9002;
	strncpy(info->sn, "1000", strlen("1000"));

	return t9000_board_mnparam_get(info);
}

#include <sal/appl/config.h>
long tsm9002_sal_config_init_defaults(void)
{
	sal_config_set("os", "unix");
	sal_config_set("pbmp_xport_xe.0","0x00000006");
    sal_config_set("port_phy_addr_1.0","64");
    sal_config_set("port_phy_addr_2.0","65");
	sal_config_set("bcm56821_20x12.0","1");
	sal_config_set("trunk_extend.0","1");
    return 0;
}
static bcm_pbmp_t tsm9002_internal_pbmp[MAX_ASIC_NUM_PER_BOARD];
static bcm_pbmp_t tsm9002_fabric2fabric_pbmp[MAX_ASIC_NUM_PER_BOARD];
static bcm_pbmp_t tsm9002_fabric2line_pbmp[MAX_ASIC_NUM_PER_BOARD];
static bcm_pbmp_t tsm9002_panel_pbmp[MAX_ASIC_NUM_PER_BOARD];
static bcm_pbmp_t tsm9002_fabric2extserv_pbmp[MAX_ASIC_NUM_PER_BOARD];

static int tsm9002_pbmp_has_set[MAX_ASIC_NUM_PER_BOARD] = {0};



long tsm9002_local_pbmp_get(int unit, bcm_pbmp_t* int_pbmp, 
	bcm_pbmp_t* f2f_pbmp, bcm_pbmp_t* f2l_pbmp, bcm_pbmp_t* panel_pbmp)
{
	if (0 == tsm9002_pbmp_has_set[unit])
	{
		bcm_pbmp_t internal_pbmp;
		bcm_pbmp_t zero_pbmp;
		bcm_pbmp_t fabric2fabric;
		bcm_pbmp_t fabric2line;
		bcm_pbmp_t panel;
		int peer_slot = 0;
		int peer_plane_port = 0;		
		int plane_port = 0;
		int port = 0;
		
		
	    BCM_PBMP_CLEAR(internal_pbmp);
	    BCM_PBMP_CLEAR(zero_pbmp);
	    BCM_PBMP_CLEAR(fabric2fabric);
	    BCM_PBMP_CLEAR(fabric2line);
	    BCM_PBMP_CLEAR(panel);

	    BCM_PBMP_ASSIGN(internal_pbmp, PBMP_ALL(unit));
	    BCM_PBMP_ASSIGN(fabric2fabric, PBMP_ALL(unit));
	    BCM_PBMP_ASSIGN(fabric2line, PBMP_ALL(unit));

	    PBMP_ALL_ITER(unit, port)
	    {
	        if(-1 != PHY_2_PANEL(unit,port))
	        {
	            BCM_PBMP_PORT_ADD(panel, port);
	            BCM_PBMP_PORT_REMOVE(internal_pbmp, port);
	            BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
	            BCM_PBMP_PORT_REMOVE(fabric2line, port);
				continue;
	        }
	        /*if the port connect to nothing, remove it*/
	        if (PPAL_PHY_2_NONE(SYS_LOCAL_MODULE_TYPE, unit, port))
	        {
	            BCM_PBMP_PORT_REMOVE(internal_pbmp, port);
	            BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
	            BCM_PBMP_PORT_REMOVE(fabric2line, port);
				continue;
	        }
	        plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			if (-1 == plane_port)
			{
	            BCM_PBMP_PORT_REMOVE(internal_pbmp, port);
	            BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
	            BCM_PBMP_PORT_REMOVE(fabric2line, port);
				continue;				
			}
            peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
			if(peer_slot == -1)
			{
                BCM_PBMP_PORT_REMOVE(internal_pbmp, port);
				BCM_PBMP_PORT_REMOVE(fabric2line, port);
				BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
				continue;
			}
            peer_plane_port = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
            if(-1 == peer_plane_port)
            {
                BCM_PBMP_PORT_REMOVE(internal_pbmp, port);
				BCM_PBMP_PORT_REMOVE(fabric2line, port);
				BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
                continue;
            }
			
            if(FULL_MESH == SYS_PRODUCT_TOPO)
            {
                BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
            }
            else if(SYS_CHASSIS_SLOTNO_ISFABRIC(peer_slot)
                && SYS_CHASSIS_SLOTNO_ISFABRIC(SYS_LOCAL_MODULE_SLOT_INDEX))
            {
                BCM_PBMP_PORT_REMOVE(fabric2line, port);
            }
            else
            {
                BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
            }
	    }	
			
		BCM_PBMP_ASSIGN(tsm9002_internal_pbmp[unit], internal_pbmp);
		BCM_PBMP_ASSIGN(tsm9002_fabric2fabric_pbmp[unit], fabric2fabric);
		BCM_PBMP_ASSIGN(tsm9002_fabric2line_pbmp[unit], fabric2line);
		BCM_PBMP_ASSIGN(tsm9002_panel_pbmp[unit], panel);			
		tsm9002_pbmp_has_set[unit] = 1;
	}

	BCM_PBMP_ASSIGN(*int_pbmp, tsm9002_internal_pbmp[unit]);
	BCM_PBMP_ASSIGN(*f2f_pbmp, tsm9002_fabric2fabric_pbmp[unit]);
	BCM_PBMP_ASSIGN(*f2l_pbmp, tsm9002_fabric2line_pbmp[unit]);
	BCM_PBMP_ASSIGN(*panel_pbmp, tsm9002_panel_pbmp[unit]);			

	return 0;
}

long tsm9002_local_conn_standby()
{
	if (FALSE == chassis_only_one_mcu_running())
	{
		if (SYS_LOCAL_MODULE_SLOT_INDEX != SYS_CHASSIS_MASTER_SLOT_INDEX(0))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		/* only one standy , so don't need the block the flooded packetes */
		return FALSE;
	}
}

long tsm9002_get_stk_trunk_by_peer_slotunit(unsigned char unit, 
		unsigned int slot_id, unsigned char peer_unit, int *trunkId)
{
	int rv = 0;
	bcm_trunk_chip_info_t trunk_info;
	int tid_id_min = 0, tid_id_max, tid_id_num, tid_cur;
	int tid_base, trunk_slot_base;
	int module_type;

	rv = bcm_trunk_chip_info_get(unit, &trunk_info);
	if (rv < 0)
	{
		npd_syslog_err("Crossbar init error: fail to get trunk chip info unit %d, error %s",
					   unit, bcm_errmsg(rv));
		return rv;
	}

	module_type = MODULE_TYPE_ON_SLOT_INDEX(SYS_CHASSIS_SLOT_NO2INDEX(slot_id));
	if(!module_type)
	{
		return -1;
	}
	

	tid_id_min = trunk_info.trunk_fabric_id_min;
	tid_id_max = trunk_info.trunk_fabric_id_max;
	tid_id_num = tid_id_max - tid_id_min + 1;

	
	tid_base = tid_id_num/SYS_CHASSIS_SLAVE_SLOTNUM;
	if (tid_base <= 0)
	{
		tid_base = 1;
	}
	if (peer_unit >= tid_base)
	{
		return -1;
	}
	
	trunk_slot_base = SYS_CHASSIS_SLOT_NO2INDEX(slot_id) - SYS_CHASSIS_MASTER_SLOT_INDEX(0);
	if(trunk_slot_base < 0)
		trunk_slot_base = SYS_CHASSIS_SLOT_NO2INDEX(slot_id);
	else
		trunk_slot_base = SYS_CHASSIS_SLOT_NO2INDEX(slot_id) - SYS_CHASSIS_MASTER_SLOTNUM;

	tid_base = trunk_slot_base * tid_base;
	tid_cur = tid_base + tid_id_min + peer_unit;

	*trunkId = tid_cur;

	return 0;
	
}

long tsm9002_local_conn_init(int product_type)
{
    int unit;
    int port;
    int rv;
    int module_id, modnum;
    int plane_port = -1;
    int peer_slot;

	int hash_control;

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        module_id = UNIT_2_MODULE(SYS_LOCAL_MODULE_TYPE,
             SYS_LOCAL_MODULE_SLOT_INDEX, unit, 0);

        /*set modid*/
        rv = bcm_stk_modid_count(unit, &modnum);
        
        rv = bcm_stk_my_modid_set(unit, module_id);
        if(BCM_E_NONE != rv)
        {
            npd_syslog_dbg("MODID set error, msg %s\n",
                bcm_errmsg(rv));
        }

        /*Then set forward between stack ports*/
        {
            bcm_pbmp_t internal_pbmp, zero_pbmp;
            bcm_pbmp_t fabric2fabric, fabric2line,panel;
            
            BCM_PBMP_CLEAR(internal_pbmp);
            BCM_PBMP_CLEAR(zero_pbmp);
            BCM_PBMP_CLEAR(fabric2fabric);
            BCM_PBMP_CLEAR(fabric2line);
            BCM_PBMP_CLEAR(panel);

            BCM_PBMP_ASSIGN(internal_pbmp, PBMP_HG_ALL(unit));
            BCM_PBMP_ASSIGN(fabric2fabric, PBMP_HG_ALL(unit));
            BCM_PBMP_ASSIGN(fabric2line, PBMP_ST_ALL(unit));

            PBMP_HG_ITER(unit, port)
            {
				bcm_port_ifg_set(unit, port, 12000, BCM_PORT_DUPLEX_FULL, 8*8);
                if(-1 != PHY_2_PANEL(unit,port))
                {
                    BCM_PBMP_PORT_ADD(panel, port);
                    BCM_PBMP_PORT_REMOVE(internal_pbmp, port);
                    BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
                    BCM_PBMP_PORT_REMOVE(fabric2line, port);
                }
                /*if the port connect to nothing, remove it*/
                if (PPAL_PHY_2_NONE(SYS_LOCAL_MODULE_TYPE, unit, port))
                {
                    BCM_PBMP_PORT_REMOVE(internal_pbmp, port);
                    BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
                    BCM_PBMP_PORT_REMOVE(fabric2line, port);
                }
                plane_port = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
                if(-1 != plane_port)
                {
                    peer_slot = SLOT_PORT_PEER_SLOT(SYS_LOCAL_MODULE_SLOT_INDEX, plane_port);
					if(peer_slot == -1)
					{
						continue;
					}
                    if(FULL_MESH == SYS_PRODUCT_TOPO)
                    {
                        BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
                    }
                    else if(SYS_CHASSIS_SLOTNO_ISFABRIC(peer_slot)
                        && SYS_CHASSIS_SLOTNO_ISFABRIC(SYS_LOCAL_MODULE_SLOT_INDEX))
                    {
                        BCM_PBMP_PORT_REMOVE(fabric2line, port);
                    }
                    else
                    {
                        BCM_PBMP_PORT_REMOVE(fabric2fabric, port);
                    }
                }
            }

            npd_syslog_dbg("%s %d: internal ports 0x%x, between fabrics ports 0x%x\r\n"
                "between fabric and line ports 0x%x\n", __func__, __LINE__, internal_pbmp.pbits[0], fabric2fabric.pbits[0], fabric2line.pbits[0]);

            /*cpu only receive local module packets*/
            {
                bcm_pbmp_t allow_ports;
                BCM_PBMP_ASSIGN(allow_ports, PBMP_ALL(unit));
                BCM_PBMP_PORT_REMOVE(allow_ports, CMIC_PORT(unit));
                npd_syslog_dbg("%s %d port egress set: port 0x%x, modid: %d, egress ports 0x%x\r\n", 
						__func__, __LINE__, -1, -1, allow_ports.pbits[0]);

                rv = bcm_port_egress_set(unit, -1, -1, allow_ports);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Crossbar init: cpu port egress set error for all module, err %s\n",
                        bcm_errmsg(rv));
                }
                npd_syslog_dbg("%s %d port egress set: port 0x%x, modid: %d, egress ports 0x%x\r\n", 
						__func__, __LINE__, -1, module_id, PBMP_ALL(unit));
                rv = bcm_port_egress_set(unit, -1, module_id, PBMP_ALL(unit));
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Crossbar init: cpu port egress set error for module %d err %s\n",
                        module_id,bcm_errmsg(rv));
                }
                if(modnum > 1)
                {
                    rv = bcm_port_egress_set(unit, -1, module_id+1, PBMP_ALL(unit));
                    if(BCM_E_NONE != rv)
                    {
                        npd_syslog_err("Crossbar init: cpu port egress set error for module %d err %s\n",
                            module_id+1,bcm_errmsg(rv));
                    }
                }
                
            }

            /*the packets for fabric should not be forwarded to line card*/
            {
                bcm_pbmp_t allow_ports;
                bcm_port_t fabric_port;
                
                BCM_PBMP_ASSIGN(allow_ports, PBMP_ALL(unit));
                BCM_PBMP_PORT_REMOVE(allow_ports, CMIC_PORT(unit));
                BCM_PBMP_REMOVE(allow_ports, fabric2line);
                BCM_PBMP_REMOVE(allow_ports, fabric2fabric);

                BCM_PBMP_ITER(fabric2fabric, fabric_port)
                {
                    npd_syslog_dbg("%s %d port egress set: port 0x%x(%s), modid: %d, egress ports 0x%x\r\n", 
						__func__, __LINE__, fabric_port, SOC_PORT_NAME(unit, fabric_port), -1, allow_ports.pbits[0]);
                    rv = bcm_port_modid_egress_set(unit, fabric_port, -1, allow_ports);
					/**all  flood block the hg  this is done in the system_conn_init */
					
                    if(BCM_E_NONE != rv)
                    {
                        npd_syslog_err("Crossbar init: unit %d fabric port %d allow 0x%x egress set error for all module, err %s\n",
                            unit, fabric_port, allow_ports, bcm_errmsg(rv));
                    }
					
					/* higig passage don't need acl ingress filter */
					rv = bcm_port_control_set(unit, fabric_port, bcmPortControlFilterIngress, 0);
                }
            }
            
         }
        /*rtag7 port trunking load banlance id set*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP4Field1, 
                BCM_HASH_FIELD_SRCMOD|BCM_HASH_FIELD_SRCPORT);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP4Field0, 
                0xfc0fff);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP6Field1, 
                BCM_HASH_FIELD_SRCMOD|BCM_HASH_FIELD_SRCPORT);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashIP6Field0, 
                0xfcf0ff);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashL2Field1, 
                BCM_HASH_FIELD_SRCMOD|BCM_HASH_FIELD_SRCPORT);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashL2Field0, 
                0xfe000f);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashSeed0, sal_rand());
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashSeed1, (unsigned int)-1 - sal_rand());
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashField0Config, BCM_HASH_FIELD_CONFIG_CRC16XOR8);
        bcm_switch_control_port_set(unit, 0, bcmSwitchHashField1Config, BCM_HASH_FIELD_CONFIG_CRC16XOR8);
        bcm_switch_control_port_set(unit, 0, bcmSwitchFabricTrunkHashSet0UnicastOffset, 0);/*hash A, 5bit*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchFabricTrunkHashSet1UnicastOffset, 16); /*hash B, 0bit*/
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet0UnicastOffset, 0);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet1UnicastOffset, 16);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet0NonUnicastOffset, 0);
        bcm_switch_control_port_set(unit, 0, bcmSwitchTrunkHashSet1NonUnicastOffset, 16);
        bcm_switch_control_port_set(unit, 0, bcmSwitchECMPHashSet0Offset, 0);
        bcm_switch_control_port_set(unit, 0, bcmSwitchECMPHashSet1Offset, 32);
        bcm_switch_control_port_set(unit, 0, bcmSwitchL2PortBlocking, 1);

		bcm_switch_control_port_get(unit, 0, bcmSwitchHashControl, &hash_control);
		hash_control |= BCM_HASH_CONTROL_MULTIPATH_L4PORTS;
		bcm_switch_control_port_set(unit, 0, bcmSwitchHashControl, hash_control);		

		PBMP_E_ITER(unit, port)
        {
            bcm_port_control_set(unit, port, bcmPortControlFabricTrunkHashSet, 1);
            bcm_port_control_set(unit, port, bcmPortControlTrunkHashSet, 1);
			
        }
        PBMP_HG_ITER(unit, port)
        {
            bcm_port_control_set(unit, port, bcmPortControlFabricTrunkHashSet, 1);
            bcm_port_control_set(unit, port, bcmPortControlTrunkHashSet, 1);


			bcm_port_control_set(unit, port, bcmPortControlL2Learn, 0x5);			
			bcm_port_control_set(unit, port, bcmPortControlL2Move, 0x5);
			bcm_port_control_set(unit, port, bcmPortControlForwardStaticL2MovePkt, 1);

			bcm_port_control_set(unit, port, bcmPortControlFilterIngress, 0);
			bcm_port_control_set(unit, port, bcmPortControlFilterLookup, 0);				

			bcm_port_flood_block_set(unit, port, CMIC_PORT(unit), 0x8);			

		}       
    }
    return BCM_E_NONE;   
}

long tsm9002_system_sdkdiff_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int unit = 0;
    int port = 0;
	
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	int modid = 0;
	int modport = 0;
	bcm_trunk_t tid = 0;
	bcm_pbmp_t conn_pbmp;
	bcm_pbmp_t trunk_egress_pbmp;
	
    bcm_trunk_add_info_t trunk_data;
    int tid_base;
	int i = 0;

    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t panel;
	bcm_pbmp_t allow_ports;
	bcm_pbmp_t block_ports;

	int f2f_port = 0;
	int cross_dport = 0;
	int blk_port = 0;
	int blk_flag = 0;
	
	bcm_pbmp_t data_pbmp;	
	int data_port = 0;
	int conn_port_count = 0;

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);

		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}	
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
		PBMP_ITER(conn_pbmp, port)
		{
            modid = module_id;
			modport = port;
            if(port >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid = module_id + 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }		
            rv = bcm_trunk_find(unit, modid, modport, &tid);         
            if(BCM_E_NONE == rv)
            {
                rv = bcm_trunk_destroy(unit, tid);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Can not delete trunk when init system conn, err msg %s\n",
                        bcm_errmsg(rv));
                }
            }			
		}
		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/
		PBMP_ITER(conn_pbmp, port)
		{
			rv = bcm_port_encap_set(unit, port, BCM_PORT_ENCAP_IEEE);
			rv = bcm_port_ifg_set(unit, port, 10000, BCM_PORT_DUPLEX_FULL, 12*8);
			rv = bcm_port_learn_set(unit, port, BCM_PORT_LEARN_FWD);
			rv = bcm_port_control_set(unit, port, bcmPortControlL2Move, BCM_PORT_LEARN_FWD);
			rv = bcm_port_control_set(unit, port, bcmPortControlForwardStaticL2MovePkt, 1);
		
			/* 将该端口加入到级联端口的位图里 */
			BCM_PBMP_PORT_ADD(PBMP_ST_ALL(unit), port);
			//rv = bcm_port_l3_enable_set(unit, port, 1);			
		}

		/* PHASE 3 - End*/
		
		/* PHASE 4 - Rebuild Trunk of the Connected Port*/

		/* 4.1 create the trunk */
		bcm_trunk_add_info_t_init(&trunk_data);
		trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
		trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
		trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
		trunk_data.psc = BCM_TRUNK_PSC_SRCMAC;

		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), unit, &tid_base);
		if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid_base, &trunk_data))
		{
			rv = bcm_trunk_create_id(unit, tid_base);
			if (rv < 0)
			{
				npd_syslog_err("Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
								   unit, tid_base, bcm_errmsg(rv));
			}
		}		

		/* 4.2 add the connected port to the trunk  */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_t trunk_port = 0;
			bcm_module_t trunk_modid = 0;
			int find_port = FALSE;
			int j = 0;
			
			trunk_port = port;
			trunk_modid = module_id;
	        if (port >= TSERIES_PORT_PER_ASICMODULE)
	        {
	            trunk_modid += 1;
				trunk_port = port - TSERIES_PORT_PER_ASICMODULE;
	        }
            for (j = 0; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == trunk_port &&
					trunk_data.tm[j] == trunk_modid)
                {
                    find_port = TRUE;
                    break;
                }
            }			
			if (!find_port)
			{
		        trunk_data.tp[trunk_data.num_ports] = trunk_port;
		        trunk_data.tm[trunk_data.num_ports] = trunk_modid;
				trunk_data.num_ports++;
				rv = bcm_port_enable_set(unit, port, 0);
                rv = bcm_trunk_set(unit, tid_base, &trunk_data);				
			}
		}
		/* PHASE 4 - End */
		
		/* PHASE 5 - Set the Egress Forward Table */
        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);

		/* PHASE 5.1 - Set Allow Egress Ports of Connected port */
		BCM_PBMP_ASSIGN(allow_ports, fabric2line);
		BCM_PBMP_OR(allow_ports, panel);

		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
	         npd_syslog_dbg("%s %d port egress set: port 0x%x(%s), modid: %d, egress ports 0x%x\r\n", 
					__func__, __LINE__, port, SOC_PORT_NAME(unit, port), -1, allow_ports.pbits[0]);
             rv = bcm_port_modid_egress_set(unit, port, -1, allow_ports);
		}
		/* PHASE 5.2 - For Flooed Traffic block the port */
		BCM_PBMP_CLEAR(block_ports);
		BCM_PBMP_ASSIGN(block_ports, fabric2line);
		
		blk_flag = 0x0;

		BCM_PBMP_ITER(conn_pbmp, port)
		{
			BCM_PBMP_ITER(block_ports, blk_port)
			{
				bcm_port_flood_block_set(unit, port, blk_port, 0);
			}
		}
		
		/* conn port to the fabric2fbric */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			BCM_PBMP_ITER(fabric2fabric, f2f_port)
			{
				cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, port);
				if (cross_dport == f2f_port)
				{
					bcm_port_flood_block_set(unit, port, f2f_port, 0);
				}
				else
				{
					bcm_port_flood_block_set(unit, port, f2f_port, BCM_PORT_FLOOD_BLOCK_ALL);
				}
			}
		}
		
		/* PHASE 5.3 - Front panel trunk egress portbitmap */
		BCM_PBMP_CLEAR(trunk_egress_pbmp);		
		rv = bcm_trunk_egress_get(unit, tid_base, &trunk_egress_pbmp);
		if (rv < 0)
		{
            npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d\n err %s",
                               unit, tid_base, bcm_errmsg(rv));				
		}
		else
		{
			BCM_PBMP_PORT_REMOVE(trunk_egress_pbmp, CMIC_PORT(unit));
			BCM_PBMP_REMOVE(trunk_egress_pbmp, conn_pbmp);
			rv = bcm_trunk_egress_set(unit, tid_base, trunk_egress_pbmp);
            npd_syslog_err("Crossbar init: fail to set trunk of unit %d tid %d egress port bitmap\n err %s",
                   unit, tid_base, bcm_errmsg(rv));				
		}
				
		/* PHASE 5 - End */
		

		/* PHASE 6 - Setting the Forwarding Table : Mod+port */

		BCM_PBMP_CLEAR(data_pbmp);
		BCM_PBMP_ASSIGN(data_pbmp, fabric2line);
		BCM_PBMP_OR(data_pbmp, panel);
		
		BCM_PBMP_COUNT(conn_pbmp, conn_port_count);
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);

			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_add(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }
					}
				}
			}
	
			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, 
										peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_add(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }
					}
				}
			}			
		}
		/* PHASE 6 -End */
		
		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 1);
		}
	}	
	return NPD_SUCCESS;
}

long tsm9002_system_sdkdiff_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )

{
	int unit = 0;
    int port = 0;
	int i = 0;
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	int modid = 0;
	int modport = 0;
	bcm_trunk_t tid = 0;
	bcm_pbmp_t conn_pbmp;

	bcm_pbmp_t data_pbmp;	
	int data_port = 0;
	int conn_port_count = 0;
    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t panel;


    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);

		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(del_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(del_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(del_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(del_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
		PBMP_ITER(conn_pbmp, port)
		{
            modid = module_id;
			modport = port;
            if(port >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid = module_id + 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }		
            rv = bcm_trunk_find(unit, modid, modport, &tid);         
            if(BCM_E_NONE == rv)
            {
                rv = bcm_trunk_destroy(unit, tid);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Can not delete trunk when init system conn, err msg %s\n",
                        bcm_errmsg(rv));
                }
            }			
		}
		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/
		PBMP_ITER(conn_pbmp, port)
		{
			rv = bcm_port_l3_enable_set(unit, port, 0);			
			
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterIngress, 0);
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterLookup, 0);				
			rv = bcm_port_learn_set(unit, port, 0x5); /* FWD,ARL */
			rv = bcm_port_control_set(unit, port, bcmPortControlL2Move, 0x5); /* FWD,ARL */	
			
			rv = bcm_port_encap_set(unit, port, BCM_PORT_ENCAP_HIGIG);
			rv = bcm_port_ifg_set(unit, port, 10000, BCM_PORT_DUPLEX_FULL, 8*8);
		}

		/* PHASE 3 - End*/

		/* PHASE 4 - Set  Egress Table */

		/* PHASE 4 - End */

		/* PHASE 5 - Setting the Forwarding Table : Mod+port */
        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);

		BCM_PBMP_CLEAR(data_pbmp);
		BCM_PBMP_ASSIGN(data_pbmp, fabric2line);
		BCM_PBMP_OR(data_pbmp, panel);
		
		BCM_PBMP_COUNT(conn_pbmp, conn_port_count);
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(del_board_type, peer_plane);

			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_delete(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }							
					}
				}
			}
	
			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, 
										peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_delete(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }							
					}
				}
			}			
		}
		/* PHASE 5 -End */	

		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 0);
		}
    }		
	return 0;		
}

long tsm9002_system_indpnt_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int rv = 0;
	int unit = 0;
	int module_id = 0;
    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t panel;
	int f2f_port = 0;
	int cross_dport = 0;

	for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
	{
		bcm_pbmp_t conn_pbmp;
        int conn_plane_slot = 0;
		int conn_plane_port = 0;
        int conn_unit = 0;
        int conn_unit_port = 0;
		int i = 0;
		bcm_port_t port = 0;
		int modid = 0;
		int modport = 0;
		bcm_trunk_t tid = 0;

        bcm_trunk_add_info_t trunk_data;
        int tid_base;
		
		
		rv = bcm_stk_my_modid_get(unit, &module_id);

		/* PHASE 1 - get the conn port pbmp and delete the exist trunk */
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
			if( -1 == conn_plane_slot )
				continue;
			
        	conn_plane_port = SLOT_PORT_PEER_PORT(insert_slotid, i);	
			if(conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;
			
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if(unit != conn_unit)
				continue;
			
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);

            modid = module_id;
			modport = i;
            if(i >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid = module_id + 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }		
            rv = bcm_trunk_find(unit, modid, modport, &tid);         
            if(BCM_E_NONE == rv)
            {
                rv = bcm_trunk_destroy(unit, tid);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Can not delete trunk when init system conn, err msg %s\n",
                        bcm_errmsg(rv));
                }
            }			
		}

		/* PHASE 3 - set port to xe  */
			
		PBMP_ITER(conn_pbmp, port)
		{
			rv = bcm_port_encap_set(unit, port, BCM_PORT_ENCAP_IEEE);
			rv = bcm_port_ifg_set(unit, port, 10000, BCM_PORT_DUPLEX_FULL, 12*8);
			//rv = bcm_port_learn_set(unit, port, BCM_PORT_LEARN_FWD);			
			/* avoid to occur the station move event */
			//rv = bcm_port_control_set(unit, port, bcmPortControlL2Move, BCM_PORT_LEARN_FWD);
			//rv = bcm_port_control_set(unit, port, bcmPortControlForwardStaticL2MovePkt, 1);
			rv = bcm_port_l3_enable_set(unit, port, 1);
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterIngress, 1);
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterLookup, 1);		
			//nam_port_tos_block_set(unit, port);

			rv = bcm_port_link_failed_clear(unit, port);				    										
			rv = bcm_port_enable_set(unit, port, 1);						
			BCM_PBMP_PORT_ADD(PBMP_ST_ALL(unit), port);
		}



		/* PHASE 4 - create the new trunk  */
				
		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), unit, &tid_base);

		if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid_base, &trunk_data))
		{
			rv = bcm_trunk_create_id(unit, tid_base);

			if (rv < 0)
			{
				npd_syslog_err("Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
								   unit, tid_base, bcm_errmsg(rv));
			}

			bcm_trunk_add_info_t_init(&trunk_data);
			trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
			trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
			trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
			trunk_data.psc = BCM_TRUNK_PSC_SRCMAC;
		}		
		PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_t trunk_port = 0;
			bcm_module_t trunk_modid = 0;
			int find_port = FALSE;
			int j = 0;
			
			trunk_port = port;
			trunk_modid = module_id;
	        if (port >= TSERIES_PORT_PER_ASICMODULE)
	        {
	            trunk_modid += 1;
				trunk_port = port - TSERIES_PORT_PER_ASICMODULE;
	        }
            for (j = 0; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == trunk_port &&
					trunk_data.tm[j] == trunk_modid)
                {
                    find_port = TRUE;
                    break;
                }
            }			
			if (!find_port)
			{
		        trunk_data.tp[trunk_data.num_ports] = trunk_port;
		        trunk_data.tm[trunk_data.num_ports] = trunk_modid;
				trunk_data.num_ports++;
                rv = bcm_trunk_set(unit, tid_base, &trunk_data);				
			}			
		}
		
		/*  PHASE 5 - egress forward table*/
        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			BCM_PBMP_ITER(fabric2fabric, f2f_port)
			{
				cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, port);
				if (cross_dport == f2f_port)
				{
					bcm_port_flood_block_set(unit, port, f2f_port, 0);
				}
				else
				{
					bcm_port_flood_block_set(unit, port, f2f_port, BCM_PORT_FLOOD_BLOCK_ALL);
				}
			}
		}
		
		/*  PHASE 6 - stk forward table */		
	}
	return NPD_SUCCESS;
}

long tsm9002_system_indpnt_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
    int unit;
    int rv;
    int module_id;
	
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
		bcm_pbmp_t conn_pbmp;
        int conn_plane_slot = 0;
		int conn_plane_port = 0;
        int conn_unit = 0;
        int conn_unit_port = 0;
		int i = 0;
		bcm_port_t port = 0;

        bcm_trunk_add_info_t trunk_data;
        int tid_base;
		
        rv = bcm_stk_my_modid_get(unit, &module_id);
		
		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(del_slotid), unit, &tid_base);
		
        /*port belongs a trunk*/
        if(!SYS_CHASSIS_SLOTNO_ISFABRIC(del_slotid))
        {
            if (BCM_E_NONE == bcm_trunk_get(unit, tid_base, &trunk_data))
            {
				rv = bcm_trunk_destroy(unit, tid_base);
				if(rv < 0 )
					npd_syslog_err("Crossbar deinit: fail to destroy trunk of unit %d tid %d\n err %s",
                                       unit, tid_base, bcm_errmsg(rv));
            }                
        }

		/* PHASE 1 - get the conn port pbmp  */
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(del_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(del_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(del_slotid, i);
			if( -1 == conn_plane_slot )
				continue;
			
        	conn_plane_port = SLOT_PORT_PEER_PORT(del_slotid, i);	
			if(conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;
			
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if(unit != conn_unit)
				continue;
			
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);	
		}

		/* PHASE 3 - set port to hg  */			
		PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_control_set(unit, conn_unit_port, bcmPortControlL2Learn, 0x5); /* ARL, FWD */
			bcm_port_control_set(unit, conn_unit_port, bcmPortControlL2Move, 0x5);
			bcm_port_control_set(unit, conn_unit_port, bcmPortControlForwardStaticL2MovePkt, 1);
		
			rv = bcm_port_encap_set(unit, conn_unit_port, BCM_PORT_ENCAP_HIGIG);
		}

		/* PHASE 4- egress forward table */		
    }

	return NPD_SUCCESS;
}

long tsm9002_system_extserv_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int unit = 0;
    int port = 0;
	
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	int modid = 0;
	int modport = 0;
	bcm_trunk_t tid = 0;
	bcm_vlan_t vlan_id = 0;
	bcm_pbmp_t conn_pbmp;
	bcm_pbmp_t vlan_pbmp;
	
    bcm_trunk_add_info_t trunk_data;
    int tid_base;
	int i = 0;

    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t panel;
	bcm_pbmp_t allow_ports;
	bcm_pbmp_t block_ports;
    bcm_pbmp_t fw_ports; 


	int f2f_port = 0;
	int blk_port = 0;
	int cross_dport = 0;
	int fw_port = 0;

	bcm_pbmp_t data_pbmp;	
	int data_port = 0;
	int conn_port_count = 0;
	

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);

		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}	
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
		PBMP_ITER(conn_pbmp, port)
		{
            modid = module_id;
			modport = port;
            if(port >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid = module_id + 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }		
            rv = bcm_trunk_find(unit, modid, modport, &tid);         
            if(BCM_E_NONE == rv)
            {
                rv = bcm_trunk_destroy(unit, tid);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Can not delete trunk when init system conn, err msg %s\n",
                        bcm_errmsg(rv));
                }
            }			
		}
		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/
		PBMP_ITER(conn_pbmp, port)
		{
			rv = bcm_port_encap_set(unit, port, BCM_PORT_ENCAP_IEEE);
			rv = bcm_port_ifg_set(unit, port, 10000, BCM_PORT_DUPLEX_FULL, 12*8);
			rv = bcm_port_learn_set(unit, port, BCM_PORT_LEARN_FWD);
			rv = bcm_port_control_set(unit, port, bcmPortControlL2Move, BCM_PORT_LEARN_FWD);
			rv = bcm_port_control_set(unit, port, bcmPortControlForwardStaticL2MovePkt, 1);
			rv = bcm_port_l3_enable_set(unit, port, 1);			
		}
		/* Remove the port From the all vlan */
		BCM_PBMP_CLEAR(vlan_pbmp);
		BCM_PBMP_ASSIGN(vlan_pbmp, conn_pbmp);
		for(vlan_id = 1; vlan_id <=4095; vlan_id++)
		{							
        	bcm_vlan_port_remove(unit, vlan_id, vlan_pbmp);
		}
		/* PHASE 3 - End*/
		
		/* PHASE 4 - Rebuild Trunk of the Connected Port*/

		/* 4.1 create the trunk */
		bcm_trunk_add_info_t_init(&trunk_data);
		trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
		trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
		trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
		trunk_data.psc = BCM_TRUNK_PSC_SRCMAC;

		nam_get_trunk_by_peer_slot(SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), unit, &tid_base);
		if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid_base, &trunk_data))
		{
			rv = bcm_trunk_create_id(unit, tid_base);
			if (rv < 0)
			{
				npd_syslog_err("Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
								   unit, tid_base, bcm_errmsg(rv));
			}
		}		

		/* 4.2 add the connected port to the trunk  */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_t trunk_port = 0;
			bcm_module_t trunk_modid = 0;
			int find_port = FALSE;
			int j = 0;
			
			trunk_port = port;
			trunk_modid = module_id;
	        if (port >= TSERIES_PORT_PER_ASICMODULE)
	        {
	            trunk_modid += 1;
				trunk_port = port - TSERIES_PORT_PER_ASICMODULE;
	        }
            for (j = 0; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == trunk_port &&
					trunk_data.tm[j] == trunk_modid)
                {
                    find_port = TRUE;
                    break;
                }
            }			
			if (!find_port)
			{
		        trunk_data.tp[trunk_data.num_ports] = trunk_port;
		        trunk_data.tm[trunk_data.num_ports] = trunk_modid;
				trunk_data.num_ports++;
				rv = bcm_port_enable_set(unit, port, 0);
                rv = bcm_trunk_set(unit, tid_base, &trunk_data);				
			}
		}
		/* PHASE 4 - End */
		
		/* PHASE 5 - Set the Egress Forward Table */
        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);

		/* PHASE 5.1 - Set Allow Egress Ports of Connected port */
		BCM_PBMP_ASSIGN(allow_ports, PBMP_ALL(unit));
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
	         npd_syslog_dbg("%s %d port egress set: port 0x%x(%s), modid: %d, egress ports 0x%x\r\n", 
					__func__, __LINE__, port, SOC_PORT_NAME(unit, port), -1, allow_ports.pbits[0]);
             rv = bcm_port_modid_egress_set(unit, port, -1, allow_ports);
		}
		/* PHASE 5.2 - For Flooed Traffic block the port */
		/* 这里主要考虑防火墙自身发出的广播报文 */
		/* 由于主控板之间没有打TRUNK，需要奇数端口对奇数端口 */
		/*  偶数端口对偶数端口 */
		BCM_PBMP_CLEAR(block_ports);
		BCM_PBMP_ASSIGN(block_ports, fabric2line);
		BCM_PBMP_OR(block_ports, fabric2fabric);	
		BCM_PBMP_OR(block_ports, panel);

		BCM_PBMP_ITER(conn_pbmp, port)
		{
			nam_port_tos_block_set(unit, port);	
			BCM_PBMP_ITER(block_ports, blk_port)
			{
				/* need block the unknown unicast packets */
				bcm_port_flood_block_set(unit, port, blk_port, 0x2);
			}			
		}
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			BCM_PBMP_ITER(fabric2fabric, f2f_port)
			{
				cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, port);
				if (cross_dport != f2f_port)
				{
					bcm_port_flood_block_set(unit, port, f2f_port, BCM_PORT_FLOOD_BLOCK_ALL);
				}
			}
		}

		BCM_PBMP_ASSIGN(fw_ports, tsm9002_fabric2extserv_pbmp[unit]);
		BCM_PBMP_ITER(conn_pbmp, port)
		{			
			BCM_PBMP_ITER(fw_ports, fw_port)
			{
				/* fw ports block non-uc packet to other fw for  fw_group*/
				bcm_port_flood_block_set(unit, port, fw_port, 0x7);
			}
		}
		
				
		/* PHASE 5 - End */
		
		/* PHASE 6 - Setting the Forwarding Table : Mod+port */
		BCM_PBMP_CLEAR(data_pbmp);
		BCM_PBMP_ASSIGN(data_pbmp, fabric2line);
		BCM_PBMP_OR(data_pbmp, panel);
		
		BCM_PBMP_COUNT(conn_pbmp, conn_port_count);		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);

			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_add(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }
					}
				}				
			}
	
			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, 
										peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_add(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }
					}
				}
			}			
		}
		/* PHASE 6 -End */
		
		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 1);
		}
		BCM_PBMP_OR(tsm9002_fabric2extserv_pbmp[unit], conn_pbmp);		

	}

	/* STEP 6 - Other board Configure  */
	if(SYS_MODULE_EXTERNAL_SERVICE(insert_board_type))
	{		
		port_isolation_op_args port_isolation_args;

		port_isolation_args.op_type = PORT_ISOLATION_ADD;
		port_isolation_args.slot = insert_slotid+1;
		rv = nbm_port_isolation_control(&port_isolation_args);
	}
	return 0;
}

long tsm9002_system_extserv_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	int unit = 0;
    int port = 0;
	int i = 0;
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	int modid = 0;
	int modport = 0;
	bcm_trunk_t tid = 0;
	bcm_pbmp_t conn_pbmp;
	bcm_pbmp_t vlan_pbmp;
	bcm_pbmp_t vlan_utpbmp;
	bcm_vlan_t vlan_id = 0;

    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t panel;


	int blk_port = 0;
	bcm_pbmp_t data_pbmp;	
	int data_port = 0;
	int conn_port_count = 0;


    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);

		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(del_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(del_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(del_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(del_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
		PBMP_ITER(conn_pbmp, port)
		{
            modid = module_id;
			modport = port;
            if(port >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid = module_id + 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }		
            rv = bcm_trunk_find(unit, modid, modport, &tid);         
            if(BCM_E_NONE == rv)
            {
                rv = bcm_trunk_destroy(unit, tid);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Can not delete trunk when init system conn, err msg %s\n",
                        bcm_errmsg(rv));
                }
            }			
		}
		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/
		PBMP_ITER(conn_pbmp, port)
		{
			rv = bcm_port_l3_enable_set(unit, port, 0);			
			
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterIngress, 0);
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterLookup, 0);				
			bcm_port_learn_set(unit, port, 0x5); /* FWD,ARL */
			bcm_port_control_set(unit, port, bcmPortControlL2Move, 0x5); /* FWD,ARL */	
			
			rv = bcm_port_encap_set(unit, port, BCM_PORT_ENCAP_HIGIG);
			rv = bcm_port_ifg_set(unit, port, 10000, BCM_PORT_DUPLEX_FULL, 8*8);
		}
		/* Remove the port From the all vlan */
		BCM_PBMP_CLEAR(vlan_pbmp);
		BCM_PBMP_CLEAR(vlan_utpbmp);
		BCM_PBMP_ASSIGN(vlan_pbmp, conn_pbmp);
		for(vlan_id = 2; vlan_id <=4095; vlan_id++)
		{							
        	bcm_vlan_port_add(unit, vlan_id, vlan_pbmp, vlan_utpbmp);
		}
		
		BCM_PBMP_ASSIGN(vlan_utpbmp, conn_pbmp);
		bcm_vlan_port_add(unit, 1, vlan_pbmp, vlan_utpbmp);
		/* PHASE 3 - End*/

		/* PHASE 4 - Set  Egress Table */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			/* clear block  non-unicast packet of the data traffic  but from the fw cpu. */
			nam_port_tos_block_restore(unit, port);	
			BCM_PBMP_ITER(PBMP_ALL(unit), blk_port)
			{
				/* clear the block setting */
				bcm_port_flood_block_set(unit, port, blk_port, 0);
			}
		}
		/* PHASE 4 - End */
		
		/* PHASE 5 - Setting the Forwarding Table : Mod+port */

        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);
		
		BCM_PBMP_CLEAR(data_pbmp);
		BCM_PBMP_ASSIGN(data_pbmp, fabric2line);
		BCM_PBMP_OR(data_pbmp, panel);
		
		BCM_PBMP_COUNT(conn_pbmp, conn_port_count);		
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(del_board_type, peer_plane);
			
			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_delete(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }							
					}
				}				
			}		
			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, 
									peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
					/* 对目的端口根据源端口取余 */
					if (data_port%conn_port_count == port%conn_port_count)
					{
			            /*hg ports between fabric and line use bcm_stk_modport_add*/
			            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
							__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
			            rv = bcm_stk_port_modport_delete(unit, data_port, peer_module, port);
			            if(BCM_E_NONE != rv)
			            {
			                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
			                    peer_module, unit, port, bcm_errmsg(rv));
			            }							
					}
				}				
			}			
		}
		/* PHASE 5 -End */		

		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 0);
		}
		BCM_PBMP_REMOVE(tsm9002_fabric2extserv_pbmp[unit], conn_pbmp);				
    }		
	return 0;
}

long tsm9002_system_fabric_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int unit = 0;
    int port = 0;
	
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	bcm_pbmp_t conn_pbmp;
	
	int i = 0;

    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t panel;
	bcm_pbmp_t allow_ports;
	bcm_pbmp_t block_ports;

	bcm_pbmp_t data_pbmp;

	int f2l_port = 0;
	int cross_dport = 0;
	int panel_port = 0;
	int data_port = 0;
	

    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);

		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}	
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
		/* MASTER board have none trunk */
		
		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/

			/* MASTER BOARD hg don't set the attribute */
		/* PHASE 3 - End*/
		
		/* PHASE 4 - Rebuild Trunk of the Connected Port*/

			/* MASTER BOARD hg don't build the trunk */
		/* PHASE 4 - End */
		
		/* PHASE 5 - Set the Egress Forward Table */
        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);

		/* PHASE 5.1 - Set Allow Egress Ports of Connected port */
		/* Master board hg only send out to the pannel */
		BCM_PBMP_ASSIGN(allow_ports, panel);
		/*need modified for centec board*/
		BCM_PBMP_ITER(conn_pbmp, port)
		{
	         npd_syslog_dbg("%s %d port egress set: port 0x%x(%s), modid: %d, egress ports 0x%x\r\n", 
					__func__, __LINE__, port, SOC_PORT_NAME(unit, port), -1, allow_ports.pbits[0]);
             rv = bcm_port_modid_egress_set(unit, port, -1, allow_ports);
		}

		
		/* PHASE 5.2 - set The flooded bitmap of the panel ports */
		BCM_PBMP_CLEAR(block_ports);
		BCM_PBMP_ASSIGN(block_ports, fabric2line);
		BCM_PBMP_OR(block_ports, panel);
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{						
			BCM_PBMP_ITER(panel, panel_port)
			{
				/* if the panel port dest port not equal the cross dport, block it */
				cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, panel_port);
				if (cross_dport == port)
				{
					bcm_port_flood_block_set(unit, panel_port, port, 0);
				}
				else
				{
					bcm_port_flood_block_set(unit, panel_port, port, BCM_PORT_FLOOD_BLOCK_ALL);
				}
			}			
		}
		/* PHASE 5 - End */

	
		
		/* PHASE 6 - Setting the Forwarding Table : Mod+port */
		BCM_PBMP_CLEAR(data_pbmp);
		BCM_PBMP_ASSIGN(data_pbmp, fabric2line);
		BCM_PBMP_OR(data_pbmp, panel);
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);

			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{				
				/* because the MASTER BOARD hg is not a trunk, so neeed the modport control */
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
                    cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, data_port);
                    if(cross_dport == port)
                    {
                        npd_syslog_dbg("%s %d stk port modport set: ingress port:%d(%s) peer mod %d, egress port: %d(%s)\r\n", 
					        __func__, __LINE__, data_port, SOC_PORT_NAME(unit, data_port), peer_module, cross_dport, SOC_PORT_NAME(unit, cross_dport));
                        rv = bcm_stk_port_modport_set(unit, data_port, peer_module, port);
                        if(BCM_E_NONE != rv)
                        {
                            npd_syslog_err("Crossbar init: dst modid %d dst port %d unit %d forward port %d, err %s\n",
                                peer_module, data_port, unit, port, bcm_errmsg(rv));
                        }
                    }					
				}
			}
	
			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, 
										peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
                    cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, data_port);
                    if(cross_dport == port)
                    {
                        npd_syslog_dbg("%s %d stk port modport set: ingress port:%d(%s) peer mod %d, egress port: %d(%s)\r\n", 
					        __func__, __LINE__, data_port, SOC_PORT_NAME(unit, data_port), peer_module, cross_dport, SOC_PORT_NAME(unit, cross_dport));
                        rv = bcm_stk_port_modport_set(unit, data_port, peer_module, port);
                        if(BCM_E_NONE != rv)
                        {
                            npd_syslog_err("Crossbar init: dst modid %d dst port %d unit %d forward port %d, err %s\n",
                                peer_module, f2l_port, unit, port, bcm_errmsg(rv));
                        }
                    }					
				}
			}			
		}
		/* PHASE 6 -End */
		
		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 1);
		}
	}
	return 0;	
}

long tsm9002_system_fabric_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
	int unit = 0;
    int port = 0;
	int i = 0;
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	bcm_pbmp_t conn_pbmp;
    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t fabric2extserv;
	
    bcm_pbmp_t panel;
	bcm_pbmp_t data_pbmp;

	int f2l_port = 0;
	int cross_dport = 0;
	int data_port = 0;


    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);

		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(del_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(del_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(del_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(del_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
			/* ... */

		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/
			/* .... */

		/* PHASE 3 - End*/

		/* PHASE 4 - Set  Egress Table */
        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		BCM_PBMP_CLEAR(fabric2extserv);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);
		BCM_PBMP_ASSIGN(fabric2extserv, tsm9002_fabric2extserv_pbmp[unit]);
						
		/* PHASE 4 - End */
		
		/* PHASE 5 - Setting the Forwarding Table : Mod+port */
		BCM_PBMP_CLEAR(data_pbmp);
		BCM_PBMP_ASSIGN(data_pbmp, fabric2line);
		BCM_PBMP_OR(data_pbmp, panel);
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(del_board_type, peer_plane);

			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{				
				/* because the MASTER BOARD hg is not a trunk, so neeed the modport control */
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
                    cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, data_port);
                    if(cross_dport == port)
                    {
                        npd_syslog_dbg("%s %d stk port modport set: ingress port:%d(%s) peer mod %d, egress port: %d(%s)\r\n", 
					        __func__, __LINE__, data_port, SOC_PORT_NAME(unit, data_port), peer_module, cross_dport, SOC_PORT_NAME(unit, cross_dport));
                        rv = bcm_stk_port_modport_delete(unit, f2l_port, peer_module, port);
                        if(BCM_E_NONE != rv)
                        {
                            npd_syslog_err("Crossbar init: dst modid %d dst port %d unit %d forward port %d, err %s\n",
                                peer_module, data_port, unit, port, bcm_errmsg(rv));
                        }
                    }					
				}
			}
	
			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, 
										peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
				BCM_PBMP_ITER(data_pbmp, data_port)
				{
                    cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, data_port);
                    if(cross_dport == port)
                    {
                        npd_syslog_dbg("%s %d stk port modport set: ingress port:%d(%s) peer mod %d, egress port: %d(%s)\r\n", 
					        __func__, __LINE__, data_port, SOC_PORT_NAME(unit, data_port), peer_module, cross_dport, SOC_PORT_NAME(unit, cross_dport));
                        rv = bcm_stk_port_modport_delete(unit, data_port, peer_module, port);
                        if(BCM_E_NONE != rv)
                        {
                            npd_syslog_err("Crossbar init: dst modid %d dst port %d unit %d forward port %d, err %s\n",
                                peer_module, f2l_port, unit, port, bcm_errmsg(rv));
                        }
                    }					
				}
			}			
		}
		/* PHASE 5 -End */		

		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 0);
		}
		
    }		
	return 0;
}

long tsm9002_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
	int unit = 0;
    int port = 0;
	int i = 0;
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	int modid = 0;
	int modport = 0;
	bcm_trunk_t tid = 0;
	bcm_pbmp_t conn_pbmp;
	
    bcm_trunk_add_info_t trunk_data;
    int tid_base;

    bcm_pbmp_t internal_pbmp;
    bcm_pbmp_t fabric2fabric; 
    bcm_pbmp_t fabric2line;
    bcm_pbmp_t panel;
	bcm_pbmp_t allow_ports;

	int f2f_port = 0;
	int f2l_port = 0;
	int cross_dport = 0;

	if(SYS_MODULE_SDK_DIFFERENT(insert_board_type, SYS_LOCAL_MODULE_TYPE) &&
		SYS_MODULE_NONE_SERVICE(insert_board_type))
	{
		return tsm9002_system_sdkdiff_conn_init(product_type, 
			insert_board_type, insert_slotid);
	}
	if(SYS_MODULE_SLOT_EXTERNAL_SERVICE(insert_slotid))
	{
		return tsm9002_system_extserv_conn_init(product_type,
			insert_board_type, insert_slotid);
	}
	if(SYS_MODULE_SLOT_INDEPENDENT_SERVICE(insert_slotid))
	{
		return tsm9002_system_indpnt_conn_init(product_type, 
			insert_board_type, insert_slotid);
	}
	if(SYS_CHASSIS_SLOTNO_ISFABRIC(insert_slotid))
	{
		return tsm9002_system_fabric_conn_init(product_type,
			insert_board_type, insert_slotid);
	}	
	
    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);
		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(insert_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(insert_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(insert_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(insert_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}	
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
		PBMP_ITER(conn_pbmp, port)
		{
            modid = module_id;
			modport = port;
            if(port >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid = module_id + 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }		
            rv = bcm_trunk_find(unit, modid, modport, &tid);         
            if(BCM_E_NONE == rv)
            {
                rv = bcm_trunk_destroy(unit, tid);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Can not delete trunk when init system conn, err msg %s\n",
                        bcm_errmsg(rv));
                }
            }			
		}
		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/
		/* default do nothing */
		
		/* PHASE 3 - End*/
		
		/* PHASE 4 - Rebuild Trunk of the Connected Port*/

		/* 4.1 create the trunk */
	

		/* 4.2 add the connected port to the trunk  */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_t trunk_port = 0;
			bcm_module_t trunk_modid = 0;
			int find_port = FALSE;
			int j = 0;

			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);

			rv = tsm9002_get_stk_trunk_by_peer_slotunit(
				unit, SYS_CHASSIS_SLOT_INDEX2NO(insert_slotid), peer_unit, &tid_base);
			if (rv != 0)
			{
				continue;
			}
			if (BCM_E_NOT_FOUND == bcm_trunk_get(unit, tid_base, &trunk_data))
			{
				bcm_trunk_add_info_t_init(&trunk_data);
				trunk_data.dlf_index = BCM_TRUNK_UNSPEC_INDEX;
				trunk_data.mc_index = BCM_TRUNK_UNSPEC_INDEX;
				trunk_data.ipmc_index = BCM_TRUNK_UNSPEC_INDEX;
				trunk_data.psc = BCM_TRUNK_PSC_SRCMAC;
				
				rv = bcm_trunk_create_id(unit, tid_base);
				if (rv < 0)
				{
					npd_syslog_err("Crossbar init: fail to create trunk id of unit %d tid %d, err %s",
									   unit, tid_base, bcm_errmsg(rv));
				}
			}	
			
			trunk_port = port;
			trunk_modid = module_id;
	        if (port >= TSERIES_PORT_PER_ASICMODULE)
	        {
	            trunk_modid += 1;
				trunk_port = port - TSERIES_PORT_PER_ASICMODULE;
	        }
            for (j = 0; j < trunk_data.num_ports; j++)
            {
                if (trunk_data.tp[j] == trunk_port &&
					trunk_data.tm[j] == trunk_modid)
                {
                    find_port = TRUE;
                    break;
                }
				/* 这里不知道为什么，STK_TRUNK，
				在设置以后，经常会将tm变为0，所以需要在这里设置以下 */
				trunk_data.tm[j] = module_id;
            }			
			if (!find_port)
			{
		        trunk_data.tp[trunk_data.num_ports] = trunk_port;
		        trunk_data.tm[trunk_data.num_ports] = trunk_modid;
				trunk_data.num_ports++;
				rv = bcm_port_enable_set(unit, port, 0);
                rv = bcm_trunk_set(unit, tid_base, &trunk_data);				
			}
		}
		/* PHASE 4 - End */
		
		/* PHASE 5 - Set the Egress Forward Table */
        BCM_PBMP_CLEAR(internal_pbmp);
        BCM_PBMP_CLEAR(fabric2fabric);
        BCM_PBMP_CLEAR(fabric2line);
        BCM_PBMP_CLEAR(panel);
		
		tsm9002_local_pbmp_get(unit, &internal_pbmp, 
					&fabric2fabric, &fabric2line, &panel);

		/* PHASE 5.1 - Set Allow Egress Ports of Connected port */
		BCM_PBMP_ASSIGN(allow_ports, fabric2line);
		BCM_PBMP_OR(allow_ports, panel);
		BCM_PBMP_OR(allow_ports, fabric2fabric);
		
				
		BCM_PBMP_ITER(conn_pbmp, port)
		{
	         npd_syslog_dbg("%s %d port egress set: port 0x%x(%s), modid: %d, egress ports 0x%x\r\n", 
					__func__, __LINE__, port, SOC_PORT_NAME(unit, port), -1, allow_ports.pbits[0]);
             rv = bcm_port_modid_egress_set(unit, port, -1, allow_ports);
		}
		
		/* PHASE 5.2 - For Flooed Traffic block the port */
		/* conn port to other f2l port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			BCM_PBMP_ITER(fabric2line, f2l_port)
			{
				bcm_port_flood_block_set(unit, port, f2l_port, 0);
			}
		}

		/* conn port to the fabric2fbric */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			BCM_PBMP_ITER(fabric2fabric, f2f_port)
			{
				cross_dport = PPAL_PHY_2_CROSS(SYS_LOCAL_MODULE_TYPE, unit, port);
				if (cross_dport == f2f_port)
				{
					bcm_port_flood_block_set(unit, port, f2f_port, 0);
				}
				else
				{
					bcm_port_flood_block_set(unit, port, f2f_port, BCM_PORT_FLOOD_BLOCK_ALL);
				}
			}
		}
		
		
		/* PHASE 5 - End */
		
		/* PHASE 6 - Setting the Forwarding Table : Mod+port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(insert_board_type, peer_plane);

			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{
	            /*hg ports between fabric and line use bcm_stk_modport_add*/
	            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
					__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
	            rv = bcm_stk_modport_add(unit, peer_module, port);
	            if(BCM_E_NONE != rv)
	            {
	                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
	                    peer_module, unit, port, bcm_errmsg(rv));
	            }	
			}
	
			peer_module = UNIT_2_MODULE(insert_board_type, insert_slotid, 
										peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
	            /*hg ports between fabric and line use bcm_stk_modport_add*/
	            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
					__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
	            rv = bcm_stk_modport_add(unit, peer_module, port);
	            if(BCM_E_NONE != rv)
	            {
	                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
	                    peer_module, unit, port, bcm_errmsg(rv));
	            }	
			}	
		}
		/* PHASE 6 -End */

		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 1);
		}
		

	}
	
	return 0;
}


long tsm9002_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{

	int unit = 0;
    int port = 0;
	int i = 0;
	int rv = 0;
    int module_id = 0;
    int conn_plane_slot = 0;
	int conn_plane_port = 0;
    int conn_unit = 0;
    int conn_unit_port = 0;
	int modid = 0;
	int modport = 0;
	bcm_trunk_t tid = 0;
	bcm_pbmp_t conn_pbmp;

	if(SYS_MODULE_SDK_DIFFERENT(del_board_type, SYS_LOCAL_MODULE_TYPE)&&
		SYS_MODULE_NONE_SERVICE(del_board_type))
	{
		return tsm9002_system_sdkdiff_conn_deinit(product_type, 
			del_board_type, del_slotid);
	}
	if(SYS_MODULE_SLOT_EXTERNAL_SERVICE(del_slotid))
	{
		return tsm9002_system_extserv_conn_deinit(product_type, 
			del_board_type, del_slotid);
	}
	if(SYS_MODULE_SLOT_INDEPENDENT_SERVICE(del_slotid))
	{
		return tsm9002_system_indpnt_conn_deinit(product_type, 
			del_board_type, del_slotid);
	}
	if(SYS_CHASSIS_SLOTNO_ISFABRIC(del_slotid))
	{
		return tsm9002_system_fabric_conn_deinit(product_type,
			del_board_type, del_slotid);
	}	



    for(unit = 0; unit < nam_asic_get_instance_num(); unit++)
    {
        rv = bcm_stk_my_modid_get(unit, &module_id);

		
		/* PHASE 1 - Find the  Connected port*/
		BCM_PBMP_CLEAR(conn_pbmp);	
		for(i = 0; i < PPAL_PLANE_PORT_COUNT(del_board_type); i++)
		{
			if( -1 == PPAL_PLANE_2_UNIT(del_board_type, i))
				continue;
			
			conn_plane_slot = SLOT_PORT_PEER_SLOT(del_slotid, i);
			if( -1 == conn_plane_slot)
				continue;
			if (conn_plane_slot != SYS_LOCAL_MODULE_SLOT_INDEX)
				continue;

			conn_plane_port = SLOT_PORT_PEER_PORT(del_slotid, i);
			if(-1 == conn_plane_port)
				continue;
						
			conn_unit = PPAL_PLANE_2_UNIT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit)
				continue;
			if (unit != conn_unit)
				continue;
			conn_unit_port= PPAL_PLANE_2_PORT(SYS_LOCAL_MODULE_TYPE, conn_plane_port);
			if (-1 == conn_unit_port)
				continue;
						
			BCM_PBMP_PORT_ADD(conn_pbmp, conn_unit_port);		
		}	
		/* PHASE 1 - End */

		/* PHASE 2 - Deleted the Exsit Trunk*/
		PBMP_ITER(conn_pbmp, port)
		{
            modid = module_id;
			modport = port;
            if(port >= TSERIES_PORT_PER_ASICMODULE)
            {
                modid = module_id + 1;
                modport -= TSERIES_PORT_PER_ASICMODULE;
            }		
            rv = bcm_trunk_find(unit, modid, modport, &tid);         
            if(BCM_E_NONE == rv)
            {
                rv = bcm_trunk_destroy(unit, tid);
                if(BCM_E_NONE != rv)
                {
                    npd_syslog_err("Can not delete trunk when init system conn, err msg %s\n",
                        bcm_errmsg(rv));
                }
            }			
		}
		/* PHASE 2 - End*/

		/* PHASE 3 - Set the Attribute of the Connected Port*/
		PBMP_ITER(conn_pbmp, port)
		{
			rv = bcm_port_l3_enable_set(unit, port, 0);			
			
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterIngress, 0);
			rv = bcm_port_control_set(unit, port, bcmPortControlFilterLookup, 0);				
			bcm_port_learn_set(unit, port, 0x5); /* FWD,ARL */
			bcm_port_control_set(unit, port, bcmPortControlL2Move, 0x5); /* FWD,ARL */	
			
			rv = bcm_port_encap_set(unit, port, BCM_PORT_ENCAP_HIGIG);
			rv = bcm_port_ifg_set(unit, port, 10000, BCM_PORT_DUPLEX_FULL, 8*8);
		}
		/* PHASE 3 - End*/

		/* PHASE 4 - Set  Egress Table */
		
		/* PHASE 4 - End */
		
		/* PHASE 5 - Setting the Forwarding Table : Mod+port */
		
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			int local_plane = 0;
			int peer_plane = 0;
			int peer_unit = 0;
			int peer_module = 0;

			/* because these port have inspect, so just trans it */
			local_plane = PPAL_PHY_2_PLANE(SYS_LOCAL_MODULE_TYPE, unit, port);
			peer_plane = SLOT_PORT_PEER_PORT(SYS_LOCAL_MODULE_SLOT_INDEX, local_plane);
			peer_unit = PPAL_PLANE_2_UNIT(del_board_type, peer_plane);
			
			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, peer_unit, 0);
			if (-1 != peer_module)
			{
	            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
					__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
	            rv = bcm_stk_modport_delete(unit, peer_module, port);
	            if(BCM_E_NONE != rv)
	            {
	                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
	                    peer_module, unit, port, bcm_errmsg(rv));
	            }	
			}		
			peer_module = UNIT_2_MODULE(del_board_type, del_slotid, 
									peer_unit, TSERIES_PORT_PER_ASICMODULE);
			if (-1 != peer_module)
			{
	            npd_syslog_dbg("%s %d stk mod port set: peer mod %d, local port: %d(%s)\r\n", 
					__func__, __LINE__, peer_module, port, SOC_PORT_NAME(unit, port));
	            rv = bcm_stk_modport_delete(unit, peer_module, port);
	            if(BCM_E_NONE != rv)
	            {
	                npd_syslog_err("Crossbar init: dst modid %d  unit %d forward port %d, err %s\n",
	                    peer_module, unit, port, bcm_errmsg(rv));
	            }	
			}			
		}
		/* PHASE 5 -End */		

		/* Enable all connected port */
		BCM_PBMP_ITER(conn_pbmp, port)
		{
			bcm_port_enable_set(unit, port, 0);
		}
		
    }		
	return 0;

	
}

fiber_module_fix_param_t tsm9002_sfp_param =
{
    .fiber_module_inserted = &tsm9002_sfp_online,
    .fiber_module_insert_detect = &tsm9002_sfp_detect_start,
    .fiber_module_info_get = &tsm9002_sfp_info_get,
};

ams_fix_param_t tsm9002_asic_switch =
{
    .type = ASIC_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = &nam_asic_init,
    .ams_info_get = &nam_asic_info_get,
    .ams_led_proc = &tsm9002_asic_led_proc,
};
ams_fix_param_t tsm9002_ctrl_switch = 
{
    .type = ASIC_CTRL_SWITCH_TYPE,
    .num = 1,
    .ams_pre_init = NULL,
    .ams_driver_init = NULL,
    .ams_info_get = NULL,
};

board_spec_fix_param_t tsm9002_spec_param =
{
    .board_type = PPAL_BOARD_TYPE_TSM9002,
    .fiber_module_fix_param = &tsm9002_sfp_param,
    .ams_param = {
                    [0] = &tsm9002_asic_switch,
                    [2] = &tsm9002_ctrl_switch
                 },
    .slotno_get = &tsm9002_slotno_get,
    .reset = &tsm9002_local_reset,
    .get_reset_type = &tsm9002_reset_type,
    .sys_led_lighting = &tsm9002_led_lighting,
    .pne_monitor_start = &tsm9002_pne_mon_start,
    .board_man_param_get = &t9000_board_mnparam_get,
    .local_conn_init = &tsm9002_local_conn_init,
    .system_conn_init = &tsm9002_system_conn_init,
    .system_conn_deinit = &tsm9002_system_conn_deinit,
    .asic_config_init = &tsm9002_sal_config_init_defaults
};


#ifdef __cplusplus
extern "C"
}
#endif

