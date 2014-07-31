#ifdef __cplusplus
extern "C"
{
#endif


struct panel_conn_s as2k_3200_9gc_pwr_panel_conn[] =
{
    {
        .panel_port = 1,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 1,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 0,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*LINUX_ETH,*/
    },
    {
        .panel_port = 2,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 2,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 1,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 3,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 3,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 2,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 4,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 4,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 3,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 5,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 5,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 4,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 6,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 6,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 5,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 7,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 7,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 6,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 8,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 8,
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = 7,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 9,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 17, 
                .distance = 0
            },
            [ASIC_POE_TYPE] = {
                .unit = 0,
                .unit_port = -1,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1,-1},
        .bcast_bus = -1,
        .user_type = ETH_GE_SFP,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    }
};


struct asic_conn_s as2k_3200_9gc_pwr_asic_conn0[] =
{
    {
        .unit = 0,
        .unit_port = 0,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 1,
        .bcast_bus = -1,
        .panel_port = 1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 2,
        .bcast_bus = -1,
        .panel_port = 2,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 3,
        .bcast_bus = -1,
        .panel_port = 3,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 4,
        .bcast_bus = -1,
        .panel_port = 4,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 5,
        .bcast_bus = -1,
        .panel_port = 5,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 6,
        .bcast_bus = -1,
        .panel_port = 6,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 7,
        .bcast_bus = -1,
        .panel_port = 7,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },

    {
        .unit = 0,
        .unit_port = 8,
        .bcast_bus = -1,
        .panel_port = 8,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 9,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 10,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 11,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 12,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 13,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 14,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 15,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 16,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 17,
        .bcast_bus = -1,
        .panel_port = 9,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 18,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 19,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 20,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 21,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 22,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 23,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 24,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = -1,
    }

};


struct unit_gmodule_s as2k_3200_9gc_pwr_unit_gmodule[] =
{
    {
        .module_base = {0, -1},
        .max_port_per_module = 32
    }
};

struct gmodule_unit_s as2k_3200_9gc_pwr_gmodule_unit[] =
{
    { 
        .unit = 0,
        .max_port_per_module = 32,
        .unit_port_base = 0
    }
};


board_conn_type_t as2k_3200_9gc_pwr_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_AS2K_3209,
    .chassis_topo = CENTRAL_FABRIC,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 9,
         },
		 [ASIC_POE_TYPE] = {
            [0] = 8,
         }
    },
    .plane_portnum = 0,
    .board_conn_from_plane = NULL,
    .board_conn_from_panel = as2k_3200_9gc_pwr_panel_conn,
    .board_conn_from_chip = {
         [ASIC_SWITCH_TYPE] = {
               [0] = as2k_3200_9gc_pwr_asic_conn0
         }
    },
    .board_gmodule = as2k_3200_9gc_pwr_unit_gmodule,
    .board_unit = as2k_3200_9gc_pwr_gmodule_unit
};



long as2k_3200_9gc_pwr_local_conn_init(int product_type)
{
	SOC_PBMP_CLEAR(PBMP_ST_ALL(0));
	NUM_ST_PORT(0) = 0;
	CMIC_PORT(0) = 0;
	SOC_PBMP_CLEAR(PBMP_CMIC(0));
    SOC_PBMP_PORT_ADD(PBMP_CMIC(0), 0);
    return BCM_E_NONE;  
}

long as2k_3200_9gc_pwr_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return 0;
}
    

#ifdef __cplusplus
}
#endif


