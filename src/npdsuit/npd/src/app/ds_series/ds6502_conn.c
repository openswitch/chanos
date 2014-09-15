#ifdef __cplusplus
extern "C"
{
#endif

struct slot_connection_s ds6502_dataplane_slot0[] =
{
    {
        .fabric_slot = 0,
        .fabric_slot_port = 0,
        .line_slot = 1,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 1,
        .line_slot = 2,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 2,
        .line_slot = 1,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 3,
        .line_slot = 2,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }
};

struct slot_connection_s ds6502_dataplane_slot1[] = 
{
    {
        .fabric_slot = 1,
        .fabric_slot_port = 0,
        .line_slot = 2,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 1,
        .line_slot = 0,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 2,
        .line_slot = 2,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 3,
        .line_slot = 0,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }
};

struct slot_connection_s *ds6502_dataplane[] =
{
    ds6502_dataplane_slot0,
    ds6502_dataplane_slot1,
};

struct slot_connection_s ds6502_ctrlplane_slot0[] = 
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 0,
        }
};

struct slot_connection_s *ds6502_ctrlplane[] =
{
    ds6502_ctrlplane_slot0
};

struct slot_connection_s ds6502_supervplane_slot0[] =
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 1,
        }
};

struct slot_connection_s *ds6502_supervplane[] =
{
    ds6502_supervplane_slot0
};



struct product_conn_type_s ds6502_product_conn = 
{
    .product_type = PRODUCT_DS6502,
    .chassis_topo = CENTRAL_FABRIC,
    .line_card_plane_port = 12,  
    .sys_backboard = ds6502_dataplane,
    .ctrl_backboard = ds6502_ctrlplane,
    .superv_backboard = ds6502_supervplane,
};

#ifdef __cplusplus
}
#endif




