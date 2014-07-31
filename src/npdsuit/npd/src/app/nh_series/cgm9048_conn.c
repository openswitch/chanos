#ifdef __cplusplus
extern "C"
{
#endif

struct plane_conn_s cgm9048_plane_conn[] =
{
    {
        .slot_port = 0,
        .chip_port = 
        {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 48,
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
                .unit_port = 48,
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
                .unit_port = 49,
                .distance = 0
            },
        },
         .trunk = 1,
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
                .unit_port = 49,
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
                .unit = 0,
                .unit_port = 51,
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
                .unit = 0,
                .unit_port = 51,
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
                .unit_port = 50,
                .distance = 0
            },
        },
         .trunk = 1,
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
                .unit_port = 50,
                .distance = 0
            },
        },
         .trunk = 1,
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

struct panel_conn_s cgm9048_panel_conn[] =
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
        .driver_type = MODULE_DRIVER_NAM_CTC, 
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },  


    {
        .panel_port = 25,
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
        .driver_type = MODULE_DRIVER_NAM_CTC, 
    },
    {
        .panel_port = 26,
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
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 27,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 28,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 29,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 30,
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
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 31,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 30,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 32,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 31,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 33,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 32,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 34,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 33,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 35,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 34,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },    
    {
        .panel_port = 36,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 35,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 37,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 36,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    }, 
    {
        .panel_port = 38,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 37,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    }, 
    {
        .panel_port = 39,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 38,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    }, 
    {
        .panel_port = 40,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 39,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },     
    {
        .panel_port = 41,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 40,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },     
    {
        .panel_port = 42,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 41,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    }, 
    {
        .panel_port = 43,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 42,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },  
    {
        .panel_port = 44,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 43,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },   
    {
        .panel_port = 45,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 44,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },     
    {
        .panel_port = 46,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 45,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },  
    {
        .panel_port = 47,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 46,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    },
    {
        .panel_port = 48,
        .chip_port = {
            [ASIC_SWITCH_TYPE] = {
                .unit = 0,
                .unit_port = 47,
                .distance = 0
            },
        },
        .subslot_port = -1,
        .plane_port = {-1, -1},
        .bcast_bus = -1,
        .user_type = ETH_GTX,
        .driver_type = MODULE_DRIVER_NAM_CTC,
    }                
};

struct asic_conn_s cgm9048_0_asic_conn[] =
{
    {
        .unit = 0,
        .unit_port = 0,
        .bcast_bus = -1,
        .panel_port = 1,
        .plane_port = -1,
        .phy_addr = 0,
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
        .panel_port = 2,
        .plane_port = -1,
        .phy_addr = 1,
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
        .panel_port = 3,
        .plane_port = -1,
        .phy_addr = 2,
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
        .panel_port = 4,
        .plane_port = -1,
        .phy_addr = 3,
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
        .panel_port = 5,
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
        .unit_port = 5,
        .bcast_bus = -1,
        .panel_port = 6,
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
        .unit_port = 6,
        .bcast_bus = -1,
        .panel_port = 7,
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
        .unit_port = 7,
        .bcast_bus = -1,
        .panel_port = 8,
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
        .unit_port = 8,
        .bcast_bus = -1,
        .panel_port = 9,
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
        .unit_port = 9,
        .bcast_bus = -1,
        .panel_port = 10,
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
        .unit_port = 10,
        .bcast_bus = -1,
        .panel_port = 11,
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
        .unit_port = 11,
        .bcast_bus = -1,
        .panel_port = 12,
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
        .unit_port = 12,
        .bcast_bus = -1,
        .panel_port = 13,
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
        .unit_port = 13,
        .bcast_bus = -1,
        .panel_port = 14,
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
        .unit_port = 14,
        .bcast_bus = -1,
        .panel_port = 15,
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
        .unit_port = 15,
        .bcast_bus = -1,
        .panel_port = 16,
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
        .unit_port = 16,
        .bcast_bus = -1,
        .panel_port = 17,
        .plane_port = -1,
        .phy_addr = 16,
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
        .phy_addr = 17,
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
        .phy_addr = 18,
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
        .phy_addr = 19,
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
        .phy_addr = 20,
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
        .phy_addr = 21,
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
        .phy_addr = 22,
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
        .phy_addr = 23,
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
        .panel_port = 25,
        .plane_port = -1,
        .phy_addr = 24,
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
        .panel_port = 26,
        .plane_port = -1,
        .phy_addr = 25,
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
        .panel_port = 27,
        .plane_port = -1,
        .phy_addr = 26,
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
        .panel_port = 28,
        .plane_port = -1,
        .phy_addr = 27,
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
        .panel_port = 29,
        .plane_port = -1,
        .phy_addr = 28,
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
        .panel_port = 30,
        .plane_port = -1,
        .phy_addr = 29,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 30,
        .bcast_bus = -1,
        .panel_port = 31,
        .plane_port = -1,
        .phy_addr = 30,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 31,
        .bcast_bus = -1,
        .panel_port = 32,
        .plane_port = -1,
        .phy_addr = 31,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 32,
        .bcast_bus = -1,
        .panel_port = 33,
        .plane_port = -1,
        .phy_addr = 32,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 33,
        .bcast_bus = -1,
        .panel_port = 34,
        .plane_port = -1,
        .phy_addr = 33,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 34,
        .bcast_bus = -1,
        .panel_port = 35,
        .plane_port = -1,
        .phy_addr = 34,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 35,
        .bcast_bus = -1,
        .panel_port = 36,
        .plane_port = -1,
        .phy_addr = 35,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 36,
        .bcast_bus = -1,
        .panel_port = 37,
        .plane_port = -1,
        .phy_addr = 36,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 37,
        .bcast_bus = -1,
        .panel_port = 38,
        .plane_port = -1,
        .phy_addr = 37,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 38,
        .bcast_bus = -1,
        .panel_port = 39,
        .plane_port = -1,
        .phy_addr = 38,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 39,
        .bcast_bus = -1,
        .panel_port = 40,
        .plane_port = -1,
        .phy_addr = 39,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 40,
        .bcast_bus = -1,
        .panel_port = 41,
        .plane_port = -1,
        .phy_addr = 40,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 41,
        .bcast_bus = -1,
        .panel_port = 42,
        .plane_port = -1,
        .phy_addr = 41,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 42,
        .bcast_bus = -1,
        .panel_port = 43,
        .plane_port = -1,
        .phy_addr = 42,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 43,
        .bcast_bus = -1,
        .panel_port = 44,
        .plane_port = -1,
        .phy_addr = 43,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 44,
        .bcast_bus = -1,
        .panel_port = 45,
        .plane_port = -1,
        .phy_addr = 44,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 45,
        .bcast_bus = -1,
        .panel_port = 46,
        .plane_port = -1,
        .phy_addr = 45,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 46,
        .bcast_bus = -1,
        .panel_port = 47,
        .plane_port = -1,
        .phy_addr = 46,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 47,
        .bcast_bus = -1,
        .panel_port = 48,
        .plane_port = -1,
        .phy_addr = 47,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 48,
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
        .unit_port = 49,
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
        .unit_port = 50,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 3,
        .phy_addr = -1,
        .trunk = -1,
        .subslot_port = -1,
        .peer_unit = -1,
        .peer_port = -1,
        .cross_port = -1
    },
    {
        .unit = 0,
        .unit_port = 51,
        .bcast_bus = -1,
        .panel_port = -1,
        .plane_port = 2,
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


struct asic_conn_s cgm9048_cpu_conn[] =
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

struct unit_gmodule_s cgm9048_unit_gmodule[] =
{
    {
        .module_base = {0, 1},
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE
    }
};

struct gmodule_unit_s cgm9048_gmodule_unit[] =
{
    { 
        .unit = 0,
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE,
        .unit_port_base = 0
    },
    { 
        .unit = 0,
        .max_port_per_module = TSERIES_PORT_PER_ASICMODULE,
        .unit_port_base = 1
    }
};

board_conn_type_t cgm9048_board_conn = 
{
    .board_type = PPAL_BOARD_TYPE_CGM9048,
    .chassis_topo = CENTRAL_FABRIC,
    .plane_portnum = 12,
    .panel_portnum = {
         [ASIC_SWITCH_TYPE] = {
            [0] = 48
         }
    },
    .board_conn_from_plane = cgm9048_plane_conn,
    .board_conn_from_panel = cgm9048_panel_conn,
    .board_conn_from_chip = {
         [ASIC_SWITCH_TYPE] = {
               [0] = cgm9048_0_asic_conn
         }
    },
    .board_gmodule = cgm9048_unit_gmodule,
    .board_unit = cgm9048_gmodule_unit
};



#ifdef __cplusplus
}
#endif

