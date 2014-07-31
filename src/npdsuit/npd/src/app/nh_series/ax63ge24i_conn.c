#ifdef __cplusplus
extern "C"
{
#endif

struct plane_conn_s ax63ge24i_plane_conn[] =
{
    {
        .slot_port = 0,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 26,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 1,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 27,
                .distance = 0
            },
        },
         .trunk = 1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 2,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 24,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 3,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 25,
                .distance = 0
            },
        },
         .trunk = 1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 4,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = -1
            },
        },
         .trunk = -1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 5,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = -1
            },
        },
         .trunk = -1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 6,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = -1
            },
        },
         .trunk = -1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 7,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = -1
            },
        },
         .trunk = -1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },

};

struct panel_conn_s ax63ge24i_panel_conn[] =
{
    {
        .panel_port = 1,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 0,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*LINUX_ETH,*/
    },
    {
        .panel_port = 2,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 1,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 3,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 2,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 4,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 3,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 5,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 4,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 6,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 5,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 7,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 6,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 8,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 7,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 9,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 8,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 10,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 9,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 11,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 10,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 12,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 11,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 13,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 12,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 14,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 13,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 15,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 14,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 16,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 15,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 17,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 16,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 18,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 17,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 19,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 18,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 20,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 19,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 21,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 20,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 22,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 21,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 23,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 22,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    },
    {
        .panel_port = 24,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 23,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CPSS, /*CPSS_SDK_DRIVER,*/
    }
};

struct asic_conn_s ax63ge24i_0_asic_conn[] =
{
    {
        .unit = 0,
        .unit_port = 0,
        .bcast_bus = -1,
        .panel_port = 1,
        .plane_port = -1,
        .phy_addr = 4,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 1,
        .bcast_bus = -1,
        .panel_port = 2,
        .plane_port = -1,
        .phy_addr = 5,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 2,
        .bcast_bus = -1,
        .panel_port = 3,
        .plane_port = -1,
        .phy_addr = 6,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 3,
        .bcast_bus = -1,
        .panel_port = 4,
        .plane_port = -1,
        .phy_addr = 7,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 4,
        .bcast_bus = -1,
        .panel_port = 5,
        .plane_port = -1,
        .phy_addr = 8,
        .trunk = 1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 5,
        .bcast_bus = -1,
        .panel_port = 6,
        .plane_port = -1,
        .phy_addr = 9,
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
        .panel_port = 7,
        .plane_port = -1,
        .phy_addr = 10,
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
        .panel_port = 8,
        .plane_port = -1,
        .phy_addr = 11,
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
        .panel_port = 9,
        .plane_port = -1,
        .phy_addr = 12,
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
        .panel_port = 10,
        .plane_port = -1,
        .phy_addr = 13,
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
        .panel_port = 11,
        .plane_port = -1,
        .phy_addr = 14,
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
        .panel_port = 12,
        .plane_port = -1,
        .phy_addr = 15,
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
        .panel_port = 13,
        .plane_port = -1,
        .phy_addr = 4,
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
        .panel_port = 14,
        .plane_port = -1,
        .phy_addr = 5,
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
        .panel_port = 15,
        .plane_port = -1,
        .phy_addr = 6,
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
        .panel_port = 16,
        .plane_port = -1,
        .phy_addr = 7,
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
        .panel_port = 17,
        .plane_port = -1,
        .phy_addr = 8,
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
        .panel_port = 18,
        .plane_port = -1,
        .phy_addr = 9,
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
        .panel_port = 19,
        .plane_port = -1,
        .phy_addr = 10,
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
        .panel_port = 20,
        .plane_port = -1,
        .phy_addr = 11,
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
        .panel_port = 21,
        .plane_port = -1,
        .phy_addr = 12,
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
        .panel_port = 22,
        .plane_port = -1,
        .phy_addr = 13,
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
        .panel_port = 23,
        .plane_port = -1,
        .phy_addr = 14,
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
        .panel_port = 24,
        .plane_port = -1,
        .phy_addr = 15,
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
        .plane_port = 2,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = 0,
        .peer_port = 25,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 25,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 3,
        .phy_addr = -1,
        .trunk = 1,
        .subslot_port = -1,
        .peer_unit = 0,
        .peer_port = 24,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 26,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 0,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,/*对端是24口单板与48口单板不同*/
        .peer_port = 27,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 27,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 1,
        .phy_addr = -1,
        .trunk = 1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = 26,
        .cross_port = -1
    }
};

struct asic_conn_s ax63ge24i_cpu_conn[] =
{
    {
        .unit = 0,
        .unit_port = 2,
        .bcast_bus = -1,
        .panel_port = 1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    }
};

struct unit_gmodule_s ax63ge24i_unit_gmodule[] =
{
    {
        .module_base = {0, -1},
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE
    }
};

struct gmodule_unit_s ax63ge24i_gmodule_unit[] =
{
    { 
        .unit = 0,
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE,
        .unit_port_base = 0
    },

	{
		.unit = -1,
		.max_port_per_module = TSERIES_PORT_PER_ASICMODULE,
		.unit_port_base = -1
	}

};

board_conn_type_t ax63ge24i_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_AX63GE24I,
    .chassis_topo = FULL_MESH,
    .plane_portnum = 8,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 24,
         }
    },
    .board_conn_from_plane = ax63ge24i_plane_conn,
    .board_conn_from_panel = ax63ge24i_panel_conn,
    .board_conn_from_chip = {
         [ASIC_SWITCH_TYPE] = {
               [0] = ax63ge24i_0_asic_conn
         }
    },
    .board_gmodule = ax63ge24i_unit_gmodule,
    .board_unit = ax63ge24i_gmodule_unit
};




#ifdef __cplusplus
}
#endif

