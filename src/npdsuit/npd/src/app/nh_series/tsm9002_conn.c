#ifdef __cplusplus
extern "C"
{
#endif

struct plane_conn_s tsm9002_plane_conn[] =
{
    {
        .slot_port = 0,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 3,
                .distance = 0
            },
        },
         .trunk = -1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 1,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 4,
                .distance = 0
            },
        },
         .trunk = -1,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 2,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 5,
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
            [0] = {
                .unit = 0,
                .unit_port = 6,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 4,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 7,
                .distance = 0
            },
        },
         .trunk = 2,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 5,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 8,
                .distance = 0
            },
        },
         .trunk = 2,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 6,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 9,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 7,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 10,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 8,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 15,
                .distance = 0
            },
        },
         .trunk = 2,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 9,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 16,
                .distance = 0
            },
        },
         .trunk = 2,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 10,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 17,
                .distance = 0
            },
        },
         .trunk = 4,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 11,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 18,
                .distance = 0
            },
        },
         .trunk = 4,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 12,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 19,
                .distance = 0
            },
        },
         .trunk = 6,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 13,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 20,
                .distance = 0
            },
        },
         .trunk = 6,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 14,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 21,
                .distance = 0
            },
        },
         .trunk = 4,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 15,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 22,
                .distance = 0
            },
        },
         .trunk = 4,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 16,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 23,
                .distance = 0
            },
        },
         .trunk = 6,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 17,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 24,
                .distance = 0
            },
        },
         .trunk = 6,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    }
};

struct panel_conn_s tsm9002_panel_conn[] =
{
    {
        .panel_port = 1,
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
        .user_type = ETH_XGE_SFPPLUS,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 2,
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
        .user_type = ETH_XGE_SFPPLUS,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    }
 
};

struct asic_conn_s tsm9002_asic_conn[] =
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
        .cross_port = 3
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
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 3,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 0,
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
        .panel_port = -1,
        .plane_port = 1,
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
        .panel_port = -1,
        .plane_port = 2,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },
    {
        .unit = 0,
        .unit_port = 6,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 3,
        .phy_addr = -1,
        .trunk = 1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 7,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 4,
        .phy_addr = -1,
        .trunk = 2,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },

    {
        .unit = 0,
        .unit_port = 8,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 5,
        .phy_addr = -1,
        .trunk = 3,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 9,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 6,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },
    {
        .unit = 0,
        .unit_port = 10,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 7,
        .phy_addr = -1,
        .trunk = 1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
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
        .plane_port = 8,
        .phy_addr = -1,
        .trunk = 2,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },
    {
        .unit = 0,
        .unit_port = 16,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 9,
        .phy_addr = -1,
        .trunk = 3,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 17,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 10,
        .phy_addr = -1,
        .trunk = 4,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },
    {
        .unit = 0,
        .unit_port = 18,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 11,
        .phy_addr = -1,
        .trunk = 5,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 19,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 12,
        .phy_addr = -1,
        .trunk = 6,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },
    {
        .unit = 0,
        .unit_port = 20,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 13,
        .phy_addr = -1,
        .trunk = 7,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 21,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 14,
        .phy_addr = -1,
        .trunk = 4,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },
    {
        .unit = 0,
        .unit_port = 22,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 15,
        .phy_addr = -1,
        .trunk = 5,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 23,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 16,
        .phy_addr = -1,
        .trunk = 6,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 3
    },
    {
        .unit = 0,
        .unit_port = 24,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 17,
        .phy_addr = -1,
        .trunk = 7,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = 4
    },
    {
        .unit = 0,
        .unit_port = 25,
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
        .unit_port = 26,
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
        .unit_port = 27,
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
        .unit_port = 28,
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
        .unit = -1
    }
};

struct unit_gmodule_s tsm9002_unit_gmodule[] =
{
    {
        .module_base = {0, -1},
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE
    }
};

struct gmodule_unit_s tsm9002_gmodule_unit[] =
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

board_conn_type_t tsm9002_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_TSM9002,
    .chassis_topo = CENTRAL_FABRIC,
    .plane_portnum = 18,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 2,
         }
    },
    .board_conn_from_plane = tsm9002_plane_conn,
    .board_conn_from_panel = tsm9002_panel_conn,
    .board_conn_from_chip = {
         [ASIC_SWITCH_TYPE] = {
               [0] = tsm9002_asic_conn
         }
    },
    .board_gmodule = tsm9002_unit_gmodule,
    .board_unit = tsm9002_gmodule_unit
};



#ifdef __cplusplus
extern "C"
}
#endif

