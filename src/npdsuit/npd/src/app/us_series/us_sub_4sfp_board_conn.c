#ifdef __cplusplus
extern "C"
{
#endif

struct panel_conn_s us_sub_4sfp_board_panel_conn[] =
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
        .plane_port = -1,
        .bcast_bus = -1,
        .user_type = ETH_GE_SFP,
        .driver_type = MODULE_DRIVER_NAM_ATHEROS, /*LINUX_ETH,*/
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
        .plane_port = -1,
        .bcast_bus = -1,
        .user_type = ETH_GE_SFP,
        .driver_type = MODULE_DRIVER_NAM_ATHEROS, /*LINUX_ETH,*/
    },
    {
        .panel_port = 3,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 3,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = -1,
        .bcast_bus = -1,
        .user_type = ETH_GE_SFP,
        .driver_type = MODULE_DRIVER_NAM_ATHEROS, /*LINUX_ETH,*/
    },
    {
        .panel_port = 4,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 4,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = -1,
        .bcast_bus = -1,
        .user_type = ETH_GE_SFP,
        .driver_type = MODULE_DRIVER_NAM_ATHEROS, /*LINUX_ETH,*/
    },
};

struct asic_conn_s us_sub_4sfp_board_asic_conn0[] =
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
        .unit_port = 2,
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
        .unit_port = 3,
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

struct asic_conn_s us_sub_4sfp_board_cpu_conn[] =
{
    {
        .unit = 0,
        .unit_port = 0,
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

struct unit_gmodule_s us_sub_4sfp_board_unit_gmodule[] =
{
    {
        .module_base = {0, -1},
        .max_port_per_module = 32
    },

};

struct gmodule_unit_s us_sub_4sfp_board_gmodule_unit[] =
{
    { 
        .unit = 0,
        .max_port_per_module = 32,
        .unit_port_base = 0
    },
};

board_conn_type_t us_sub_4sfp_board_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_US_SUB_4SFP,
    .chassis_topo = CENTRAL_FABRIC,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 4,
         }
    },
    .board_conn_from_plane = NULL,
    .board_conn_from_panel = us_sub_4sfp_board_panel_conn,
    .board_conn_from_chip = {
         [ASIC_SWITCH_TYPE] = {
               [0] = us_sub_4sfp_board_asic_conn0,
         }
    },
    .board_gmodule = us_sub_4sfp_board_unit_gmodule,
    .board_unit = us_sub_4sfp_board_gmodule_unit
};
  

#ifdef __cplusplus
}
#endif