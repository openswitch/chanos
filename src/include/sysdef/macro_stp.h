#ifndef __MACRO_STP_H__
#define __MACRO_STP_H__

/*stp cfg default value*/

#define DEF_BR_PRIO 32768
#define MIN_BR_PRIO 0
#define MAX_BR_PRIO 61440

#define DEF_BR_HELLOT   2
#define MIN_BR_HELLOT   1
#define MAX_BR_HELLOT   10

#define DEF_BR_MAXAGE   20
#define MIN_BR_MAXAGE   6
#define MAX_BR_MAXAGE   40

#define DEF_BR_FWDELAY  15
#define MIN_BR_FWDELAY  4
#define MAX_BR_FWDELAY  30

#define DEF_BR_REVISION  0
#define MIN_BR_REVISION  0
#define MAX_BR_REVISION  65535

#define DEF_REMAINING_HOPS 20
#define MIN_REMAINING_HOPS 6
#define MAX_REMAINING_HOPS 40

#define MIN_MST_ID 0
#define MAX_MST_ID 64

#define STP_FORCE_VERS  2 /* NORMAL_RSTP */
#define MST_FORCE_VERS 3 /*NORMAL_MSTP*/

/* port configuration */
#define DEF_PORT_PRIO   128
#define MIN_PORT_PRIO   0
#define MAX_PORT_PRIO   240 /* in steps of 16 */

#define ADMIN_PORT_PATH_COST_AUTO   200000000 /*zhengcaisheng change 20->200000000*/
#define DEF_ADMIN_NON_STP   NPD_FALSE
#define DEF_ADMIN_EDGE      NPD_TRUE 
#define DEF_LINK_DELAY      3 /* see edge.c */
#define P2P_AUTO  2
#define DEF_P2P        P2P_AUTO

typedef enum
{
   NAM_STP_PORT_STATE_DISABLE_E = 0,
   NAM_STP_PORT_STATE_DISCARD_E,
   NAM_STP_PORT_STATE_LEARN_E,
   NAM_STP_PORT_STATE_FORWARD_E
}NAM_RSTP_PORT_STATE_E;

#endif
