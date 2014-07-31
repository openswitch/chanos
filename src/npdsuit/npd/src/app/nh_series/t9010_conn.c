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
struct slot_connection_s t9010_dataplane_slot0[] =
{
    {
        .fabric_slot = 0,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 3,
        .line_slot = 5,
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
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 0,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
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

struct slot_connection_s t9010_dataplane_slot1[] = 
{
    {
        .fabric_slot = 1,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 6,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 7,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 3,
        .line_slot = 5,
        .line_slot_port = 7,
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
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 1,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
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

struct slot_connection_s t9010_dataplane_slot2[] = 
{
    {
        .fabric_slot = 2,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 10,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 10,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 11,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 3,
        .line_slot = 5,
        .line_slot_port = 11,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 2,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9010_dataplane_slot3[] = 
{
    {
        .fabric_slot = 3,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 14,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 14,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 15,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 3,
        .line_slot = 5,
        .line_slot_port = 15,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 3,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }     
};

struct slot_connection_s t9010_dataplane_slot4[] = 
{
    {
        .fabric_slot = 4,
        .fabric_slot_port = 0,
        .line_slot = 5,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 2,
        .line_slot = 0,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 3,
        .line_slot = 0,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 4,
        .line_slot = 9,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 5,
        .line_slot = 9,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 6,
        .line_slot = 1,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 7,
        .line_slot = 1,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 8,
        .line_slot = 8,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 9,
        .line_slot = 8,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 10,
        .line_slot = 2,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 11,
        .line_slot = 2,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 12,
        .line_slot = 7,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 13,
        .line_slot = 7,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 14,
        .line_slot = 3,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 15,
        .line_slot = 3,
        .line_slot_port = 2,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 16,
        .line_slot = 6,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 4,
        .fabric_slot_port = 17,
        .line_slot = 6,
        .line_slot_port = 2,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9010_dataplane_slot5[] = 
{
    {
        .fabric_slot = 5,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 0,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 1,
        .line_slot = 4,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 2,
        .line_slot = 0,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 3,
        .line_slot = 0,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 4,
        .line_slot = 9,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 5,
        .line_slot = 9,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 6,
        .line_slot = 1,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 7,
        .line_slot = 1,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 8,
        .line_slot = 8,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 9,
        .line_slot = 8,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 10,
        .line_slot = 2,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 11,
        .line_slot = 2,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 12,
        .line_slot = 7,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 13,
        .line_slot = 7,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 14,
        .line_slot = 3,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 15,
        .line_slot = 3,
        .line_slot_port = 3,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 16,
        .line_slot = 6,
        .line_slot_port = 1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 5,
        .fabric_slot_port = 17,
        .line_slot = 6,
        .line_slot_port = 3,
        .bcast_bus = -1,
    }
};

struct slot_connection_s t9010_dataplane_slot6[] = 
{
    {
        .fabric_slot = 6,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 16,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 16,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 17,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 3,
        .line_slot = 5,
        .line_slot_port = 17,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },    
    {
        .fabric_slot = 6,
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 6,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    } 
};

struct slot_connection_s t9010_dataplane_slot7[] = 
{
    {
        .fabric_slot = 7,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 12,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 12,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 13,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 3,
        .line_slot = 5,
        .line_slot_port = 13,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 7,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }     
};

struct slot_connection_s t9010_dataplane_slot8[] = 
{
    {
        .fabric_slot = 8,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 8,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 8,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 9,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 3,
        .line_slot = 5,
        .line_slot_port = 9,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 8,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }     
};

struct slot_connection_s t9010_dataplane_slot9[] = 
{
    {
        .fabric_slot = 9,
        .fabric_slot_port = 0,
        .line_slot = 4,
        .line_slot_port = 4,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 1,
        .line_slot = 5,
        .line_slot_port = 4,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 2,
        .line_slot = 4,
        .line_slot_port = 5,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 3,
        .line_slot = 5,
        .line_slot_port = 5,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 4,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 5,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 6,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 7,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },    
    {
        .fabric_slot = 9,
        .fabric_slot_port = 8,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 9,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 10,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    },
    {
        .fabric_slot = 9,
        .fabric_slot_port = 11,
        .line_slot = -1,
        .line_slot_port = -1,
        .bcast_bus = -1,
    }  
};

struct slot_connection_s *t9010_dataplane[] =
{
    t9010_dataplane_slot0,
    t9010_dataplane_slot1,
    t9010_dataplane_slot2,
    t9010_dataplane_slot3,
    t9010_dataplane_slot4,
    t9010_dataplane_slot5,
    t9010_dataplane_slot6,
    t9010_dataplane_slot7,
    t9010_dataplane_slot8,
    t9010_dataplane_slot9
};

struct slot_connection_s t9010_ctrlplane_slot0[] = 
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 0,
        }
};

struct slot_connection_s *t9010_ctrlplane[] =
{
    t9010_ctrlplane_slot0
};

struct slot_connection_s t9010_supervplane_slot0[] =
{
        {
            .fabric_slot = 0,
            .fabric_slot_port = 1,
            .line_slot = 0,
            .line_slot_port = 1,
            .bcast_bus = 1,
        }
};

struct slot_connection_s *t9010_supervplane[] =
{
    t9010_supervplane_slot0
};


struct product_conn_type_s t9010_product_conn = 
{
    .product_type = PRODUCT_T9010,
    .chassis_topo = CENTRAL_FABRIC,
    .line_card_plane_port = 12,    
    .sys_backboard = t9010_dataplane,
    .ctrl_backboard = t9010_ctrlplane,
    .superv_backboard = t9010_supervplane,
};

#ifdef __cplusplus
}
#endif



