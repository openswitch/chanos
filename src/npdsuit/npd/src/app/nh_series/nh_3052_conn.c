#ifdef __cplusplus
extern "C"
{
#endif

#if 0
struct slot_connection_s nh_6606_dataplane_slot0[] =
{
    {
        .fabric_slot = 0,
        .fabric_slot_port = 0,
        .line_slot = 0,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 1,
        .line_slot = 0,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 2,
        .line_slot = 0,
        .line_slot_port = 2,
        .bcast_bus = -1,
    }
};

struct slot_connection_s nh_6606_dataplane_slot1[] = 
{
};

struct slot_connection_s nh_6606_dataplane_slot2[] = 
{
};

struct slot_connection_s nh_6606_dataplane_slot3[] = 
{
};

struct slot_connection_s nh_6606_dataplane_slot4[] = 
{
};

struct slot_connection_s nh_6606_dataplane_slot5[] = 
{
};

struct slot_connection_s *nh_6606_dataplane[] =
{
    nh_6606_dataplane_slot0,
    nh_6606_dataplane_slot1,
    nh_6606_dataplane_slot2,
    nh_6606_dataplane_slot3,
    nh_6606_dataplane_slot4,
    nh_6606_dataplane_slot5
};
struct slot_connection_s nh_6606_ctrlplane_slot0[] = 
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 0,
        }
};

struct slot_connection_s *nh_6606_ctrlplane[] =
{
    nh_6606_ctrlplane_slot0
};

struct slot_connection_s nh_6606_supervplane_slot0[] =
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 1,
        }
};

struct slot_connection_s *nh_6606_supervplane[] =
{
    nh_6606_supervplane_slot0
};

#endif

struct product_conn_type_s nh_3052_product_conn = 
{
    .product_type = PRODUCT_NH_3052,
    .chassis_topo = FULL_MESH,
    .sys_backboard = NULL,
    .ctrl_backboard = NULL,
    .superv_backboard = NULL,
};

#if 0
struct plane_conn_s nh_1msb_plane_conn[] =
{
    {
        .slot_port = 0,
        .chip_port = 
        {
            [0] = {
                .unit = 0,
                .unit_port = 0,
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
            [0] = {
                .unit = 0,
                .unit_port = 1,
                .distance = 0
            },
        },
         .trunk = 0,
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
                .unit_port = 2,
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
                .unit_port = 3,
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
                .unit_port = 4,
                .distance = 0
            },
        },
         .trunk = 1,
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
                .unit_port = 5,
                .distance = 0
            },
        },
         .trunk = 1,
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
                .unit_port = 6,
                .distance = 0
            },
        },
         .trunk = 1,
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
                .unit_port = 7,
                .distance = 0
            },
        },
         .trunk = 1,
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
        .slot_port = 9,
        .chip_port = 
        {
            [0] ={
                .unit = 0,
                .unit_port = 9,
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
                .unit_port = 10,
                .distance = 0
            },
        },
         .trunk = 2,
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
                .unit_port = 11,
                .distance = 0
            },
        },
         .trunk = 2,
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
                .unit_port = 12,
                .distance = 0
            },
        },
         .trunk = 3,
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
                .unit_port = 13,
                .distance = 0
            },
        },
         .trunk = 3,
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
                .unit_port = 14,
                .distance = 0
            },
        },
         .trunk = 3,
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
                .unit_port = 15,
                .distance = 0
            },
        },
         .trunk = 3,
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
                .unit_port = 16,
                .distance = 0
            },
        },
         .trunk = 4,
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
                .unit_port = 17,
                .distance = 0
            },
        },
         .trunk = 4,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    }
};
#endif

struct panel_conn_s nh_31msb_panel_conn[] =
{
    {
        .panel_port = 1,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*LINUX_ETH,*/
    },
    {
        .panel_port = 2,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 3,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 4,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 5,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 6,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 7,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 8,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 9,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 10,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 11,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 12,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 13,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 14,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 15,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 16,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 17,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 18,
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
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 19,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 24,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 20,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 25,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 21,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 26,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 22,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 27,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 23,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 28,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 24,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 29,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 25,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 6,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 26,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 7,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 27,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 8,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 28,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 9,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 29,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 10,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 30,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 11,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 31,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 12,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 32,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 13,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 33,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 14,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 34,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 15,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 35,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 16,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 36,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 17,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 37,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 18,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 38,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 19,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 39,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 20,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 40,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 21,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 41,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 22,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 42,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 23,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 43,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 24,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 44,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 25,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 45,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 26,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 46,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 27,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 47,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 28,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 48,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 29,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_FE_TX,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 49,
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
        .user_type = ETH_GE_COMBO,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 50,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 1,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GE_COMBO,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 51,
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
        .user_type = ETH_GE_SFP,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    },
    {
        .panel_port = 52,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 2,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GE_SFP,
        .driver_type = MODULE_DRIVER_NAM_BCM, /*BCM_SDK_DRIVER,*/
    }
};

struct asic_conn_s nh_31msb_asic_conn0[] =
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
        .panel_port = 49,
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
        .panel_port = 51,
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
        .unit = 0,
        .unit_port = 4,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = 1,
        .peer_port = 4,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 5,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = 1,
        .peer_port = 5,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 6,
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
        .unit_port = 7,
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
        .unit_port = 8,
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
        .unit_port = 9,
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
        .unit_port = 10,
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
        .unit_port = 11,
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
        .unit_port = 12,
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
        .unit_port = 13,
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
        .unit_port = 14,
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
        .unit_port = 15,
        .bcast_bus = -1,
        .panel_port = 10,
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
        .panel_port = 11,
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
        .panel_port = 12,
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
        .panel_port = 13,
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
        .panel_port = 14,
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
        .panel_port = 15,
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
        .panel_port = 16,
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
        .panel_port = 17,
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
        .panel_port = 18,
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
        .panel_port = 19,
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
        .unit_port = 25,
        .bcast_bus = -1,
        .panel_port = 20,
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
        .panel_port = 21,
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
        .panel_port = 22,
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
        .panel_port = 23,
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
        .unit_port = 29,
        .bcast_bus = -1,
        .panel_port = 24,
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

struct asic_conn_s nh_31msb_asic_conn1[] =
{
    {
        .unit = 1,
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
        .unit = 1,
        .unit_port = 1,
        .bcast_bus = -1,
        .panel_port = 50,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 2,
        .bcast_bus = -1,
        .panel_port = 52,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
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
        .unit = 1,
        .unit_port = 4,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = 0,
        .peer_port = 4,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 5,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = 0,
        .peer_port = 5,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 6,
        .bcast_bus = -1,
        .panel_port = 25,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 7,
        .bcast_bus = -1,
        .panel_port = 26,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },

    {
        .unit = 1,
        .unit_port = 8,
        .bcast_bus = -1,
        .panel_port = 27,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 9,
        .bcast_bus = -1,
        .panel_port = 28,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 10,
        .bcast_bus = -1,
        .panel_port = 29,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 11,
        .bcast_bus = -1,
        .panel_port = 30,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 12,
        .bcast_bus = -1,
        .panel_port = 31,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 13,
        .bcast_bus = -1,
        .panel_port = 32,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 14,
        .bcast_bus = -1,
        .panel_port = 33,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 15,
        .bcast_bus = -1,
        .panel_port = 34,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 16,
        .bcast_bus = -1,
        .panel_port = 35,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 17,
        .bcast_bus = -1,
        .panel_port = 36,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 18,
        .bcast_bus = -1,
        .panel_port = 37,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 19,
        .bcast_bus = -1,
        .panel_port = 38,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 20,
        .bcast_bus = -1,
        .panel_port = 39,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 21,
        .bcast_bus = -1,
        .panel_port = 40,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 22,
        .bcast_bus = -1,
        .panel_port = 41,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 23,
        .bcast_bus = -1,
        .panel_port = 42,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 24,
        .bcast_bus = -1,
        .panel_port = 43,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 25,
        .bcast_bus = -1,
        .panel_port = 44,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 26,
        .bcast_bus = -1,
        .panel_port = 45,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 27,
        .bcast_bus = -1,
        .panel_port = 46,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 28,
        .bcast_bus = -1,
        .panel_port = 47,
        .plane_port = -1,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 29,
        .bcast_bus = -1,
        .panel_port = 48,
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

struct asic_conn_s nh_31msb_cpu_conn[] =
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

struct unit_gmodule_s nh_31msb_unit_gmodule[] =
{
    {
        .module_base = {0, -1},
        .max_port_per_module = 32
    },
    {
        .module_base = {1, -1},
        .max_port_per_module = 32
    }
};

struct gmodule_unit_s nh_31msb_gmodule_unit[] =
{
    { 
        .unit = 0,
        .max_port_per_module = 32,
        .unit_port_base = 0
    },

    {
        .unit = 1,
        .max_port_per_module = 32,
        .unit_port_base = 0
    }
};

board_conn_type_t nh_31msb_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_NH_3052,
    .chassis_topo = CENTRAL_FABRIC,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 26,
            [1] = 26
         }
    },
    .board_conn_from_plane = NULL,
    .board_conn_from_panel = nh_31msb_panel_conn,
    .board_conn_from_chip = {
         [ASIC_SWITCH_TYPE] = {
               [0] = nh_31msb_asic_conn0,
               [1] = nh_31msb_asic_conn1
         }
    },
    .board_gmodule = nh_31msb_unit_gmodule,
    .board_unit = nh_31msb_gmodule_unit
};
  

#ifdef __cplusplus
}
#endif


