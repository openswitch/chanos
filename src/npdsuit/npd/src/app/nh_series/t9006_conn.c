#ifdef __cplusplus
extern "C"
{
#endif
#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>

#include "util/npd_list.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nbm/npd_bmapi.h"
#include "npd/nam/npd_amapi.h"
#include "nbm/nbm_api.h"
#include "npd_log.h"

#include "product_feature.h"
#include "product_conn.h"

#include "npd_product.h"
#include "npd_c_slot.h"
#include "board/ts_product_feature.h"
struct slot_connection_s t9006_dataplane_slot0[] =
{
    {
        .fabric_slot = 0,
        .fabric_slot_port = 0,
        .line_slot = 2,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 1,
        .line_slot = 3,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 2,
        .line_slot = 2,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 3,
        .line_slot = 3,
        .line_slot_port = 3,
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
        .line_slot = 2,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 7,
        .line_slot = 3,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 8,
        .line_slot = 2,
        .line_slot_port = 7,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 9,
        .line_slot = 3,
        .line_slot_port = 7,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9006_dataplane_slot1[] = 
{
    {
        .fabric_slot = 1,
        .fabric_slot_port = 0,
        .line_slot = 2,
        .line_slot_port = 10,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 1,
        .line_slot = 3,
        .line_slot_port = 10,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 2,
        .line_slot = 2,
        .line_slot_port = 11,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 3,
        .line_slot = 3,
        .line_slot_port = 11,
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
        .line_slot = 2,
        .line_slot_port = 14,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 7,
        .line_slot = 3,
        .line_slot_port = 14,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 8,
        .line_slot = 2,
        .line_slot_port = 15,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 9,
        .line_slot = 3,
        .line_slot_port = 15,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9006_dataplane_slot2[] = 
{
    {
        .fabric_slot = 2,
        .fabric_slot_port = 0,
        .line_slot = 3,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 1,
        .line_slot = 3,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 2,
        .line_slot = 0,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 3,
        .line_slot = 0,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 4,
        .line_slot = 5,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 5,
        .line_slot = 5,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 6,
        .line_slot = 0,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 7,
        .line_slot = 0,
        .line_slot_port = 8,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 8,
        .line_slot = 5,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 9,
        .line_slot = 5,
        .line_slot_port = 8,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 10,
        .line_slot = 1,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 11,
        .line_slot = 1,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 12,
        .line_slot = 4,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 13,
        .line_slot = 4,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 14,
        .line_slot = 1,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 15,
        .line_slot = 1,
        .line_slot_port = 8,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 16,
        .line_slot = 4,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 17,
        .line_slot = 4,
        .line_slot_port = 8,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9006_dataplane_slot3[] = 
{
    {
        .fabric_slot = 3,
        .fabric_slot_port = 0,
        .line_slot = 2,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 1,
        .line_slot = 2,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 2,
        .line_slot = 0,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 3,
        .line_slot = 0,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 4,
        .line_slot = 5,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 5,
        .line_slot = 5,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 6,
        .line_slot = 0,
        .line_slot_port = 7,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 7,
        .line_slot = 0,
        .line_slot_port = 9,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 8,
        .line_slot = 5,
        .line_slot_port = 7,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 9,
        .line_slot = 5,
        .line_slot_port = 9,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 10,
        .line_slot = 1,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 11,
        .line_slot = 1,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 12,
        .line_slot = 4,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 13,
        .line_slot = 4,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 14,
        .line_slot = 1,
        .line_slot_port = 7,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 15,
        .line_slot = 1,
        .line_slot_port = 9,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 16,
        .line_slot = 4,
        .line_slot_port = 7,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 17,
        .line_slot = 4,
        .line_slot_port = 9,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9006_dataplane_slot4[] = 
{
    {
        .fabric_slot = 4,
        .fabric_slot_port = 0,
        .line_slot = 2,
        .line_slot_port = 12,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 1,
        .line_slot = 3,
        .line_slot_port = 12,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 2,
        .line_slot = 2,
        .line_slot_port = 13,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 3,
        .line_slot = 3,
        .line_slot_port = 13,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 6,
        .line_slot = 2,
        .line_slot_port = 16,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 7,
        .line_slot = 3,
        .line_slot_port = 16,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 8,
        .line_slot = 2,
        .line_slot_port = 17,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 9,
        .line_slot = 3,
        .line_slot_port = 17,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9006_dataplane_slot5[] = 
{
    {
        .fabric_slot = 5,
        .fabric_slot_port = 0,
        .line_slot = 2,
        .line_slot_port = 4,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 1,
        .line_slot = 3,
        .line_slot_port = 4,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 2,
        .line_slot = 2,
        .line_slot_port = 5,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 3,
        .line_slot = 3,
        .line_slot_port = 5,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 6,
        .line_slot = 2,
        .line_slot_port = 8,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 7,
        .line_slot = 3,
        .line_slot_port = 8,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 8,
        .line_slot = 2,
        .line_slot_port = 9,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 9,
        .line_slot = 3,
        .line_slot_port = 9,
        .bcast_bus = -1,
    },    
    {
        .fabric_slot = 5,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }    
};

struct slot_connection_s *t9006_dataplane[] =
{
    t9006_dataplane_slot0,
    t9006_dataplane_slot1,
    t9006_dataplane_slot2,
    t9006_dataplane_slot3,
    t9006_dataplane_slot4,
    t9006_dataplane_slot5
};

struct slot_connection_s t9006_ctrlplane_slot0[] = 
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 0,
        }
};

struct slot_connection_s *t9006_ctrlplane[] =
{
    t9006_ctrlplane_slot0
};

struct slot_connection_s t9006_supervplane_slot0[] =
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 1,
        }
};

struct slot_connection_s *t9006_supervplane[] =
{
    t9006_supervplane_slot0
};


struct product_conn_type_s t9006_product_conn = 
{
    .product_type = PRODUCT_T9006,
    .chassis_topo = CENTRAL_FABRIC,
    .line_card_plane_port = 12,    
    .sys_backboard = t9006_dataplane,
    .ctrl_backboard = t9006_ctrlplane,
    .superv_backboard = t9006_supervplane,
};


struct board_conn_type_s none_board_type_conn =
{
   .board_type = PPAL_BOARD_TYPE_NH_NONE,
};

#if 0
long tgm9024_local_conn_init(int product_type)
{
    return tseries_linecard_local_conn_init();
}

long tgm9024_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return tseries_linecard_system_conn_init(product_type, 
        insert_board_type, insert_slotid);
}
long tgm9024_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
    return tseries_linecard_system_conn_deinit(product_type, 
        del_board_type, del_slotid);
}

long txm9004_local_conn_init(int product_type)
{
    return tseries_linecard_local_conn_init();
}

long txm9004_system_conn_init(
    int product_type, 
    int insert_board_type, 
    int insert_slotid    
    )
{
    return tseries_linecard_system_conn_init(product_type, 
        insert_board_type, insert_slotid);
}

long txm9004_system_conn_deinit(
    int product_type, 
    int del_board_type, 
    int del_slotid    
    )
{
    return tseries_linecard_system_conn_deinit(product_type, 
        del_board_type, del_slotid);
}
#endif
#ifdef __cplusplus
}
#endif


