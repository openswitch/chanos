#ifndef __NPD_ETH_PORT_H__
#define __NPD_ETH_PORT_H__


#include "npd_database.h"
#include "netif_index.h"


#define NPD_ETHPORT_SHOWRUN_CFG_SIZE	(3*1024) /* for all 24GE ports configuration */

typedef struct _eth_port_attribute_s {
	unsigned char 	admin_state; /* administrative status */
	unsigned short 	mtu;		/* port MTU or MRU */
	unsigned char 	duplex;		/* duplex mode */
	unsigned char 	speed;		/* speed */
	unsigned char 	fc;			/* flow-control */
	unsigned char 	bp;			/* back-pressure */
	unsigned char 	autoNego;	/* Auto-Negotiation */
	unsigned char 	duplex_an;	/* duplex Auto-Negotiation */
	unsigned char 	speed_an;	/* speed Auto-Negotiation */
	unsigned char 	fc_an;		/* flow-control Auto-Negotiation */
	/*unsigned char   copper;	*/	/* preferred copper*/
	/*unsigned char   fiber;*/		/* preferred fiber*/
	unsigned char   mediaPrefer;/* media preferred*/
}NPD_ETHPORT_ATTRIBUTE_S;

typedef struct 
{
    struct list_head list;
	unsigned int netif_index;
	int event;
} npd_port_event_t;

struct npd_eth_cfg_s
{
    int rate_poll_enable;
	int buffer_mode;
	int max_buffer;
	int sc_glb_dlf_kbps;
	int sc_glb_mc_kbps;
	int sc_glb_bc_kbps;
	int global_load_balance_mode;
};

#endif
