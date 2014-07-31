#ifdef __cplusplus
extern "C"
{
#endif

struct plane_conn_s asx6602_plane_conn[] =
{
    {
        .slot_port = 0,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = 0
            },
        },
         .trunk = -1,
         .subslot_port = -1,
         .panel_port = 2,
         .bcast_bus = -1,
         .port_type = PLANE_PORTTYPE_USERETH,
    },
    {
        .slot_port = 1,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = 0
            },
        },
         .trunk = 1,
         .subslot_port = -1,
         .panel_port = 2,
         .bcast_bus = -1,
         .port_type = PLANE_PORTTYPE_USERETH,
    },
    {
        .slot_port = 2,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = 1,
         .bcast_bus = -1,
         .port_type = PLANE_PORTTYPE_USERETH,
    },
    {
        .slot_port = 3,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = 0
            },
        },
         .trunk = 1,
         .subslot_port = -1,
         .panel_port = 1,
         .bcast_bus = -1,
         .port_type = PLANE_PORTTYPE_USERETH,
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
         .port_type = PLANE_PORTTYPE_USERETH,
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
         .port_type = PLANE_PORTTYPE_USERETH,
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
         .port_type = PLANE_PORTTYPE_USERETH,
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
         .port_type = PLANE_PORTTYPE_USERETH,
    },

};

struct panel_conn_s asx6602_panel_conn[] =
{
    {
        .panel_port = 1,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {2, 3},
        .bcast_bus = -1,
        .user_type = ETH_XGE_SFPPLUS,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 2,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {0, 1},
        .bcast_bus = -1,
        .user_type = ETH_XGE_SFPPLUS,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    }
 
};


struct unit_gmodule_s asx6602_unit_gmodule[] =
{
    {
        .module_base = {0, -1},
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE
    },
};

struct gmodule_unit_s asx6602_gmodule_unit[] =
{
    { 
        .unit = 0,
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE,
        .unit_port_base = 0
    },
};


board_conn_type_t asx6602_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_ASX6602,
    .chassis_topo = FULL_MESH,
    .plane_portnum = 8,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 2,
         }
    },
    .board_conn_from_plane = asx6602_plane_conn,
    .board_conn_from_panel = asx6602_panel_conn,

	.board_gmodule = asx6602_unit_gmodule,
	.board_unit = asx6602_gmodule_unit,
};

#ifdef __cplusplus
}
#endif

