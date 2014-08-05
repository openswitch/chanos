#ifndef __PRODUCT_CONNECTION_H__
#define __PRODUCT_CONNECTION_H__

/*Usually, a telecom equipment consists of backplane/midplan and fabric card, line card,
  this file wants to give a general description of the connection among these components.
  first, the backplane have the bus connect different slot, the connection need a table of 
  { source slot number, source slot port number, destination slot number, destination slot port number}
  second, the fabric card and line card both need a table description how the line card fabric components/device 
  commponents connect backplane/midplane, it's a table of 
  { fabric component unit, port number, trunk number,slot port number}
  third, the line card need a table explain how the components connect to each other.
  { fabric component unit, port number, trunk number, device unit, device port}
*/ 
/*product connection configuration data structure types, 
 using these structure, the new product type can be easy supported.
  */
#define CONN_MAX_GMODULE_PERSLOT 2
#define CONN_MAX_MODULE_PERUNIT 2
#define CONN_MAX_SUBPORT_PER_ETHPORT 4
enum
{
    CENTRAL_FABRIC,
    FULL_MESH
};

enum
{
    ASIC_SWITCH_TYPE,
    ASIC_FPGA_TYPE,
    ASIC_CTRL_SWITCH_TYPE,
    ASIC_CPU_TYPE,
    ASIC_POE_TYPE,
    ASIC_TYPE_MAX
};

enum
{
    BCM_HIGIGPLUS,
    BCM_HIGIG2,
    COMMON_XG,
    MARVELL_XXX
};


typedef struct slot_connection_s
{
	int fabric_slot; /*src slot no, in fullmesh architecture, is src slot*/
	int fabric_slot_port;
	int line_slot;
	int line_slot_port;
    int bcast_bus;  /*broadcast bus ID*/
}slot_connection_t;

typedef slot_connection_t **backplane_conn_t;

typedef struct peer_chip_port_s
{
    int unit;
    int unit_port;
    int distance; /*the unit in which distance, should be PLANE_DISTANCE_x*/
}peer_chip_port_t;

typedef struct eth_sub_port_s
{
    int unit;
    int unit_port;
    int user_type;
}eth_sub_port_t;

typedef enum plane_porttype_s
{
    PLANE_PORTTYPE_STACK,
    PLANE_PORTTYPE_STACKETH,
    PLANE_PORTTYPE_USERETH,
}plane_porttype_t;

typedef enum sdktype_s
{
    SDK_BCM,
    SDK_MARVELL,
    SDK_TOPSEC,
	SDK_ATHEROS
}sdktype_t;

#define BOARD_INNER_CONN_PORT 0xFFFFFFFD

typedef struct plane_conn_s
{
    int slot_port;
    peer_chip_port_t chip_port[ASIC_TYPE_MAX];
	int trunk; /*-1 means no trunk*/
    int subslot_port;
    int panel_port;
    int bcast_bus;
    plane_porttype_t port_type;  /*the port type of stack*/
} plane_conn_t;

typedef plane_conn_t *plane_chip_conn_t;


typedef struct panel_conn_s
{
    int panel_port;
    peer_chip_port_t chip_port[ASIC_TYPE_MAX];
    int subslot_port;
    int plane_port[2];
    int bcast_bus;
    int user_type;  /*such as cooper, sfp, combo, */
    int driver_type; /*such as broadcom, marvell, cavium*/
    int can_be_stack; /*can configure as stack port*/
	eth_sub_port_t sub_ports[CONN_MAX_SUBPORT_PER_ETHPORT];
}panel_conn_t;

typedef panel_conn_t *panel_chip_conn_t;
typedef panel_conn_t *subboard_conn_t;

typedef struct asic_conn_s
{
    int unit;
    int unit_port;
    int bcast_bus;
    int panel_port;
    int plane_port;
    int phy_addr;
    int trunk;
    int subslot_port;
    int peer_unit;
    int peer_port;
    int cross_port;
    int sub_port;
}asic_conn_t;

typedef asic_conn_t  *board_chip_conn_t;

typedef struct product_conn_type_s
{
    unsigned long product_type;
    int chassis_topo;
    int line_card_plane_port;
	slot_connection_t **sys_backboard;
    slot_connection_t  **ctrl_backboard;
    slot_connection_t **superv_backboard;
} product_conn_type_t;

typedef struct unit_gmodule_s
{
    int module_base[2];
    int max_port_per_module;
}unit_gmodule_t;
typedef struct gmodule_unit_s
{
    int unit;
    int max_port_per_module;
    int unit_port_base;
}gmodule_unit_t;

typedef unit_gmodule_t *board_unit_gmodule_t;
typedef gmodule_unit_t *board_gmodule_unit_t;



typedef struct board_conn_type_s
{
    unsigned long board_type;
    int chassis_topo;
    int plane_portnum;
    int panel_portnum[ASIC_TYPE_MAX][4];
    plane_chip_conn_t board_conn_from_plane;
    panel_chip_conn_t board_conn_from_panel;
    board_chip_conn_t board_conn_from_chip[ASIC_TYPE_MAX][4];
    subboard_conn_t   board_conn_from_sub[4];
    board_unit_gmodule_t   board_gmodule;
    board_gmodule_unit_t   board_unit;
}board_conn_type_t;

extern product_conn_type_t **product_conn_type;
extern board_conn_type_t **board_conn_type;
extern board_conn_type_t **board_conn_fullmesh_type;
extern board_conn_type_t *local_board_conn_type;
extern product_conn_type_t *backplane_type;

#define SLOT_PORT_PEER_SLOT(slot, port)\
    (((backplane_type->sys_backboard)[slot])[port].line_slot)
#define SLOT_PORT_PEER_PORT(slot,port)\
    (((backplane_type->sys_backboard[slot]))[port].line_slot_port)
#define SYS_PRODUCT_LINE_CARD_PLANE_PORT()\
    (backplane_type->line_card_plane_port)

#define PPAL_PLANE_PORT_COUNT(mtype)	\
	(board_conn_type[mtype]->plane_portnum)
	
#define PPAL_PLANE_2_UNIT_TYPE(mtype, port, type) \
    ((board_conn_type[mtype]->board_conn_from_plane)[port].chip_port[type].unit)
#define PPAL_PLANE_2_PORT_TYPE(mtype, port, type) \
    ((board_conn_type[mtype]->board_conn_from_plane)[port].chip_port[type].unit_port)
    
#define PPAL_PHY_2_PLANE_TYPE(mtype, unit, port, type) \
    ((board_conn_type[mtype]->board_conn_from_chip[type][unit])[port].plane_port)
#define PPAL_PHY_2_TRUNK_TYPE(mtype, unit, port, type) \
    ((board_conn_type[mtype]->board_conn_from_chip[type][unit])[port].trunk)
#define PPAL_PHY_2_SUBSLOT_TYPE(mtype, unit, port, type) \
    ((board_conn_type[mtype]->board_conn_from_chip[type][unit])[port].subslot_port)
#define PPAL_PHY_2_PEERUNIT_TYPE(mtype, unit, port, type) \
    ((board_conn_type[mtype]->board_conn_from_chip[type][unit])[port].peer_unit)
#define PPAL_PHY_2_PEERPORT_TYPE(mtype, unit, port, type) \
    ((board_conn_type[mtype]->board_conn_from_chip[type][unit])[port].peer_port)
#define PPAL_PHY_2_CROSS_TYPE(mtype, unit, port, type) \
    ((board_conn_type[mtype]->board_conn_from_chip[type][unit])[port].cross_port)
#define PPAL_PHY_EXIST_TYPE(mtype, dev, port, type) \
    ((board_conn_type[mtype]->board_conn_from_chip[type][dev])[port].unit != -1)
#define PPAL_PLANE_2_UNIT(mtype, port) \
    PPAL_PLANE_2_UNIT_TYPE(mtype, port, ASIC_SWITCH_TYPE)
#define PPAL_PLANE_2_PORT(mtype, port) \
    PPAL_PLANE_2_PORT_TYPE(mtype, port, ASIC_SWITCH_TYPE)
#define PPAL_PLANE_PORTTYPE_TYPE(mtype, port) \
    ((board_conn_type[mtype]->board_conn_from_plane)[port].port_type)

#define PPAL_PLANE_2_SUBSLOT(mtype, port, type) \
    ((board_conn_type[mtype]->board_conn_from_plane)[port].subslot_port)
#define PPAL_PLANE_2_PANEL(mtype, port, type) \
    ((board_conn_type[mtype]->board_conn_from_plane)[port].panel_port)
#define PPAL_PHY_2_PLANE(mtype, unit, port) \
    PPAL_PHY_2_PLANE_TYPE(mtype, unit, port, ASIC_SWITCH_TYPE)
#define PPAL_PHY_2_TRUNK(mtype, unit, port) \
    PPAL_PHY_2_TRUNK_TYPE(mtype, unit, port, ASIC_SWITCH_TYPE)
#define PPAL_PHY_2_SUBSLOT(mtype, unit, port) \
    PPAL_PHY_2_SUBSLOT_TYPE(mtype, unit, port, ASIC_SWITCH_TYPE)
#define PPAL_PHY_2_PEERUNIT(mtype, unit, port) \
    PPAL_PHY_2_PEERUNIT_TYPE(mtype, unit, port, ASIC_SWITCH_TYPE)
#define PPAL_PHY_2_PEERPORT(mtype, unit, port) \
    PPAL_PHY_2_PEERPORT_TYPE(mtype, unit, port, ASIC_SWITCH_TYPE)
#define PPAL_PHY_2_CROSS(mtype, unit, port) \
    PPAL_PHY_2_CROSS_TYPE(mtype, unit, port, ASIC_SWITCH_TYPE)
#define PPAL_PHY_EXIST(mtype, unit, port) \
    PPAL_PHY_EXIST_TYPE(mtype, unit, port, ASIC_SWITCH_TYPE)
    
#define PPAL_PHY_2_NONE(mtype, unit, port)\
   (-1 == PPAL_PHY_2_PANEL(mtype, unit, port) \
     && -1 == PPAL_PHY_2_PLANE(mtype, unit, port) \
     && -1 == PPAL_PHY_2_SUBSLOT(mtype, unit, port) \
     && -1 == PPAL_PHY_2_PEERUNIT(mtype, unit, port) \
   )

/* get panel port no by (unit,port)  */
#define PHY_2_PANEL_TYPE(unit, phy_port, asic_type) \
	((local_board_conn_type->board_conn_from_chip[asic_type][unit])[phy_port].panel_port)
	
#define PHY_2_PANEL_SUBPORT_TYPE(unit, phy_port, asic_type) \
	((local_board_conn_type->board_conn_from_chip[asic_type][unit])[phy_port].sub_port)
	
#define PHY_2_PANEL(unit, phy_port) \
	PHY_2_PANEL_TYPE(unit, phy_port, ASIC_SWITCH_TYPE)
	
#define PHY_2_PANEL_SUBPORT(unit, phy_port) \
	PHY_2_PANEL_SUBPORT_TYPE(unit, phy_port, ASIC_SWITCH_TYPE)


#define PPAL_PANEL_PORT_TYPE(mtype, lport) \
    ((board_conn_type[mtype]->board_conn_from_panel)[lport-1].user_type)

#define PPAL_PANEL_PORT_DRVIER_TYPE(mtype, lport) \
    ((board_conn_type[mtype]->board_conn_from_panel)[lport-1].driver_type)


#define PANEL_2_PHY_PORT_TYPE(lport,asic_type) \
	((local_board_conn_type->board_conn_from_panel)[lport-1].chip_port[asic_type].unit_port)
#define PANEL_2_PHY_PORT(lport) \
    PANEL_2_PHY_PORT_TYPE(lport,ASIC_SWITCH_TYPE)
    
#define PANEL_SUBPORT_2_PHY_PORT(lport, sub_port) \
    (sub_port&&((local_board_conn_type->board_conn_from_panel)[lport-1].sub_ports[sub_port].user_type))? \
        ((local_board_conn_type->board_conn_from_panel)[lport-1].sub_ports[sub_port].unit_port):PANEL_2_PHY_PORT_TYPE(lport,ASIC_SWITCH_TYPE)


   /* get device port no by panel port no */
#define PANEL_2_PHY_UNIT_TYPE(lport,asic_type) \
	((local_board_conn_type->board_conn_from_panel)[lport-1].chip_port[asic_type].unit)
#define PANEL_2_PHY_UNIT(lport) \
    PANEL_2_PHY_UNIT_TYPE(lport,ASIC_SWITCH_TYPE)
    
#define PANEL_SUBPORT_2_PHY_UNIT(lport, sub_port) \
    (sub_port&&((local_board_conn_type->board_conn_from_panel)[lport-1].sub_ports[sub_port].user_type))? \
        ((local_board_conn_type->board_conn_from_panel)[lport-1].sub_ports[sub_port].unit):PANEL_2_PHY_UNIT_TYPE(lport,ASIC_SWITCH_TYPE)


#define PANEL_2_POE_PORT_TYPE(lport,asic_type) \
	((local_board_conn_type->board_conn_from_panel)[lport-1].chip_port[asic_type].unit_port)
#define PANEL_2_POE_PORT(lport) \
    PANEL_2_POE_PORT_TYPE(lport,ASIC_POE_TYPE)    

   /* get poe port no by panel port no */
#define PANEL_2_POE_UNIT_TYPE(lport,asic_type) \
	((local_board_conn_type->board_conn_from_panel)[lport-1].chip_port[asic_type].unit)
#define PANEL_2_POE_UNIT(lport) \
    PANEL_2_POE_UNIT_TYPE(lport,ASIC_POE_TYPE)


#define PORT_PHY_ADDR_TYPE(unit, port,asic_type) \
	((local_board_conn_type->board_conn_from_chip[asic_type][unit])[port].phy_addr)
	
#define NPD_PORT_PHY_ADDR(unit, port) \
	PORT_PHY_ADDR_TYPE(unit, port, ASIC_SWITCH_TYPE)

#define UNIT_PORT_2_SUB_SLOT_INDEX(unit, port) \
    (((local_board_conn_type->board_conn_from_chip[ASIC_SWITCH_TYPE][unit])[port].subslot_port \
    == -1) ? 0:((local_board_conn_type->board_conn_from_chip[ASIC_SWITCH_TYPE][unit])[port].subslot_port))
    
#define UNIT_2_MODULE_BASE(mtype,unit) \
    ((board_conn_type[mtype]->board_gmodule)[unit].module_base)
#define UNIT_MODULE_MAXPORT(mtype,unit) \
    ((board_conn_type[mtype]->board_gmodule)[unit].max_port_per_module)
#define UNIT_2_MODULE_BASENUM(mtype,unit, port) \
    (port/UNIT_MODULE_MAXPORT(mtype, unit))

#define UNIT_PORT_2_MODULE_PORT(mtype, unit, port)\
    (port%UNIT_MODULE_MAXPORT(mtype, unit))
    
#define UNIT_2_MODULE(mtype, slot_id, unit, port) \
    (\
        (UNIT_2_MODULE_BASE(mtype, unit)[UNIT_2_MODULE_BASENUM(mtype,unit,port)] == -1)?-1:(UNIT_2_MODULE_BASE(mtype, unit)[UNIT_2_MODULE_BASENUM(mtype,unit,port)] \
        + slot_id * CONN_MAX_GMODULE_PERSLOT) \
    )

#define MOD_ID_TO_CHASSIS(module) 0
#define MOD_ID_TO_SLOT_INDEX(module) (module/CONN_MAX_GMODULE_PERSLOT)
#define MOD_ID_TO_SUB_SLOT_INDEX(module) 0

#define MODULE_2_SLOT_INDEX(module) (module/CONN_MAX_GMODULE_PERSLOT)

#define MODULE_2_UNIT_CHECK(mtype, module) \
	(((board_conn_type[mtype]->board_unit)[module%CONN_MAX_GMODULE_PERSLOT].unit != -1))

#define MODULE_2_UNIT(mtype, module) \
    ((board_conn_type[mtype]->board_unit)[module%CONN_MAX_GMODULE_PERSLOT].unit)
#define MODULE_UNIT_MAXPORT(mtype, module) \
    ((board_conn_type[mtype]->board_unit)[module%CONN_MAX_GMODULE_PERSLOT].max_port_per_module)
#define MODULE_2_UNIT_PORT_BASE(mtype, module) \
    ((board_conn_type[mtype]->board_unit)[module%CONN_MAX_GMODULE_PERSLOT].unit_port_base)

#define MODULE_PORT_2_UNIT_PORT(mtype, module, port) \
    ( \
        MODULE_UNIT_MAXPORT(mtype, module)\
        *(module-(UNIT_2_MODULE(mtype, MODULE_2_SLOT_INDEX(module), \
                           MODULE_2_UNIT(mtype, module), 0)))\
        + port\
    )

#define MODULE_PORT_2_LOCAL_PORT(mtype, module, port) \
    (MODULE_2_UNIT_PORT_BASE(mtype, module))

/* get panel port no by (unit,port)  */
#define PPAL_PHY_2_PANEL_TYPE(mtype, unit, phy_port, type) \
	((board_conn_type[mtype]->board_conn_from_chip[type][unit])[phy_port].panel_port)

#define PPAL_PHY_2_PANEL_SUBPORT_TYPE(mtype, unit, phy_port, type) \
	((board_conn_type[mtype]->board_conn_from_chip[type][unit])[phy_port].sub_port)

#define PPAL_PHY_2_PANEL(mtype, unit, phy_port) \
	PPAL_PHY_2_PANEL_TYPE(mtype,unit, phy_port, ASIC_SWITCH_TYPE)
	
#define PPAL_PHY_2_PANEL_SUBPORT(mtype, unit, phy_port) \
	PPAL_PHY_2_PANEL_SUBPORT_TYPE(mtype,unit, phy_port, ASIC_SWITCH_TYPE)


#define PPAL_PANEL_2_PHY_PORT_TYPE(mtype, lport,type) \
	((board_conn_type[mtype]->board_conn_from_panel)[lport-1].chip_port[type].unit_port)
#define PPAL_PANEL_2_PHY_PORT(mtype, lport) \
    PPAL_PANEL_2_PHY_PORT_TYPE(mtype, lport,ASIC_SWITCH_TYPE)
    
#define PPAL_PANEL_SUBPORT_2_PHY_PORT(mtype, lport, sub_port) \
    (sub_port&&((board_conn_type[mtype]->board_conn_from_panel)[lport-1].sub_ports[sub_port].user_type))? \
        ((board_conn_type[mtype]->board_conn_from_panel)[lport-1].sub_ports[sub_port].unit_port):PPAL_PANEL_2_PHY_PORT_TYPE(mtype, lport,ASIC_SWITCH_TYPE)
    
   /* get device port no by panel port no */
#define PPAL_PANEL_2_PHY_UNIT_TYPE(mtype, lport,type) \
	((board_conn_type[mtype]->board_conn_from_panel)[lport-1].chip_port[type].unit)
	
#define PPAL_PANEL_2_PHY_UNIT(mtype, lport) \
    PPAL_PANEL_2_PHY_UNIT_TYPE(mtype,lport,ASIC_SWITCH_TYPE)
    
#define PPAL_PANEL_SUBPORT_2_PHY_UNIT(mtype, lport, sub_port) \
    (sub_port&&((board_conn_type[mtype]->board_conn_from_panel)[lport-1].sub_ports[sub_port].user_type))? \
        ((board_conn_type[mtype]->board_conn_from_panel)[lport-1].sub_ports[sub_port].unit):PPAL_PANEL_2_PHY_UNIT_TYPE(mtype,lport,ASIC_SWITCH_TYPE)


#define PPAL_PANEL_2_SUBSLOT(mtype, lport) \
    ((board_conn_type[mtype]->board_conn_from_panel)[lport-1].subslot_port)
#define PPAL_PANEL_2_PLANE(mtype, lport, id) \
    ((board_conn_type[mtype]->board_conn_from_panel)[lport-1].plane_port[id])
#define PPAL_PANEL_TYPE(mtype, lport) \
    ((board_conn_type[mtype]->board_conn_from_panel)[lport-1].type)
    
#define PPAL_PORT_PHY_ADDR_TYPE(mtype, unit, port,type) \
	((board_conn_type[mtype]->board_conn_from_chip[type][unit])[port].phy_addr)
	
#define PPAL_PORT_PHY_ADDR(mtype, unit, port) \
	PPAL_PORT_PHY_ADDR_TYPE(mtype, unit, port,ASIC_SWITCH_TYPE)

#define PPAL_UNIT_PANEL_PORT_NUM(mtype, unit) \
    (board_conn_type[mtype]->panel_portnum[ASIC_SWITCH_TYPE][unit])
    
#define PPAL_SUB_SLOT_PANEL_PORT_2_UNIT(sub_slot, port) \
        ((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].chip_port[ASIC_SWITCH_TYPE].unit)
#define PPAL_SUB_SLOT_PANEL_PORT_2_PORT(sub_slot, port) \
        ((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].chip_port[ASIC_SWITCH_TYPE].unit_port)
        
#define PPAL_SUB_SLOT_PANEL_PORT_AND_SUBPORT_2_UNIT(sub_slot, port, sub_port) \
        (sub_port&&((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].sub_ports[sub_port].user_type))?((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].sub_ports[sub_port].unit):((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].chip_port[ASIC_SWITCH_TYPE].unit)

#define PPAL_SUB_SLOT_PANEL_PORT_AND_SUBPORT_2_PORT(sub_slot, port, sub_port) \
        (sub_port&&((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].sub_ports[sub_port].user_type))?((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].sub_ports[sub_port].unit_port):((local_board_conn_type->board_conn_from_sub[sub_slot - 1])[port].chip_port[ASIC_SWITCH_TYPE].unit_port)

extern void ax_sal_config_init_defaults(void);


#endif  /* end of __PRODUCT_CONNECTION_H_*/



