#ifdef __cplusplus
extern "C"
{
#endif

struct plane_conn_s tgm9048_plane_conn[] =
{
    {
        .slot_port = 0,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
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
                .unit = 1,
                .unit_port = 27,
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
        .slot_port = 3,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 27,
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
            [ASIC_SWITCH_TYPE] = {
                .unit = -1,
                .unit_port = -1,
                .distance = -1
            },
        },
         .trunk = 0,
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
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 6,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 28,
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
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 29,
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
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 28,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 9,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 29,
                .distance = 0
            },
        },
         .trunk = 0,
         .subslot_port = -1,
         .panel_port = -1,
         .bcast_bus = -1,
    },
    {
        .slot_port = 10,
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
        .slot_port = 11,
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
    }
                
};

struct panel_conn_s tgm9048_panel_conn[] =
{
    {
        .panel_port = 1,
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
        .driver_type = MODULE_DRIVER_NAM_BCM, 
    },
    {
        .panel_port = 2,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 3,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 4,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 5,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 6,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 7,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 8,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 9,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 10,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 11,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },    
    {
        .panel_port = 12,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 13,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 14,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 15,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },     
    {
        .panel_port = 16,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 17,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 18,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 19,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },  
    {
        .panel_port = 20,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },   
    {
        .panel_port = 21,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },     
    {
        .panel_port = 22,
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
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },  
    {
        .panel_port = 23,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 24,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },  


    {
        .panel_port = 25,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM, 
    },
    {
        .panel_port = 26,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 3,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 27,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 4,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 28,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 1,
                .unit_port = 5,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 29,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 30,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 31,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 32,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 33,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 34,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 35,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },    
    {
        .panel_port = 36,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 37,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 38,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 39,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 40,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },     
    {
        .panel_port = 41,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },     
    {
        .panel_port = 42,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }, 
    {
        .panel_port = 43,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },  
    {
        .panel_port = 44,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },   
    {
        .panel_port = 45,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },     
    {
        .panel_port = 46,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },  
    {
        .panel_port = 47,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    },
    {
        .panel_port = 48,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_BCM,
    }                
};

struct asic_conn_s tgm9048_0_asic_conn[] =
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
        .unit_port = 3,
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
        .unit_port = 4,
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
        .unit_port = 5,
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
        .unit_port = 6,
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
        .unit_port = 7,
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
        .unit_port = 8,
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
        .unit_port = 9,
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
        .unit_port = 10,
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
        .unit_port = 11,
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
        .unit_port = 12,
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
        .unit_port = 13,
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
        .unit_port = 14,
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
        .unit_port = 15,
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
        .unit_port = 16,
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
        .unit_port = 17,
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
        .unit_port = 18,
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
        .unit_port = 19,
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
        .unit_port = 20,
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
        .unit_port = 21,
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
        .unit_port = 22,
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
        .unit_port = 23,
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
        .unit_port = 24,
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
        .unit_port = 25,
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
        .unit = 0,
        .unit_port = 26,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 2,
        .phy_addr = -1,
        .trunk = 0,
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
        .plane_port = 3,
        .phy_addr = -1,
        .trunk = 0,
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
        .plane_port = 8,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 29,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 9,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = -1
    }
        
};


struct asic_conn_s tgm9048_1_asic_conn[] =
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
        .unit_port = 2,
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
        .unit_port = 3,
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
        .unit_port = 4,
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
        .unit_port = 5,
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
        .unit_port = 6,
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
        .unit_port = 7,
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
        .unit_port = 8,
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
        .unit_port = 9,
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
        .unit_port = 10,
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
        .unit_port = 11,
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
        .unit_port = 12,
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
        .unit_port = 13,
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
        .unit_port = 14,
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
        .unit_port = 15,
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
        .unit_port = 16,
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
        .unit_port = 17,
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
        .unit_port = 18,
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
        .unit_port = 19,
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
        .unit_port = 20,
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
        .unit_port = 21,
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
        .unit_port = 22,
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
        .unit_port = 23,
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
        .unit_port = 24,
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
        .unit_port = 25,
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
        .unit = 1,
        .unit_port = 26,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 0,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 27,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 1,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 28,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 6,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 1,
        .unit_port = 29,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 7,
        .phy_addr = -1,
        .trunk = 0,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = -1
    }
};

struct asic_conn_s tgm9048_cpu_conn[] =
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

struct unit_gmodule_s tgm9048_unit_gmodule[] =
{
    {
        .module_base = {0, -1},
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE
    },
    {
        .module_base = {1, -1},
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE
    }
};

struct gmodule_unit_s tgm9048_gmodule_unit[] =
{
    { 
        .unit = 0,
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE,
        .unit_port_base = 0
    },

    {
        .unit = 1,
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE,
        .unit_port_base = 0
    }
};

board_conn_type_t tgm9048_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_TGM9048,
    .chassis_topo = CENTRAL_FABRIC,
	.plane_portnum = 12,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 24,
            [1] = 24
         }
    },
    .board_conn_from_plane = tgm9048_plane_conn,
    .board_conn_from_panel = tgm9048_panel_conn,
    .board_conn_from_chip = {
         [ASIC_SWITCH_TYPE] = {
               [0] = tgm9048_0_asic_conn,
               [1] = tgm9048_1_asic_conn
         }
    },
    .board_gmodule = tgm9048_unit_gmodule,
    .board_unit = tgm9048_gmodule_unit
};



#ifdef __cplusplus
extern "C"
}
#endif

