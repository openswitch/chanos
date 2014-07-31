#ifndef _UDLD_MAIN_H_
#define _UDLD_MAIN_H_

#define UDLD_SERVICE_MAGIC				1380
#define UDLD_NPD_MSG_MAGIC				1381
#define UDLD_SWITCH_OVER_INSTANCE		40
#define UDLD_MAX_PORT_NUM				2048

#define UDLD_STATUS_UNI_DIR				0x0001
#define UDLD_STATUS_BI_DIR				0x0003
#define UDLD_STATUS_WRONG_CON			0x0004
#define UDLD_STATUS_LOOP				0x0008
#define UDLD_STATUS_LINK_DISABLE		0x0100
#define UDLD_STATUS_LINK_DOWN			0x0200

#define UDLD_MODE_NORMAL				0x1
#define UDLD_MODE_ENHANCED			0x2
#define UDLD_MODE_LOOPDETECT		0x3


/*suijianzhi -- 20130422 -- recover mode define*/
#define UDLD_RECOVER_MODE_AUTO              0x1 
#define UDLD_RECOVER_MODE_MANUAL        0x2


typedef struct _udld_global_ctrl_
{
	unsigned char global_enable;
	unsigned char mode;
	unsigned char echo_timer;
	unsigned char enhanced_timer;
	unsigned char enhanced_retry;
	unsigned char echo_timeout;
	unsigned char recover_timer;
	unsigned char recover_mode; /*recover mode select : 1--auto; 2--manual*/
}udld_global_ctrl;

typedef struct _udld_netif_ctrl_
{
	unsigned int netif_index;
	unsigned short enable;
	unsigned short status;
	unsigned int remote_netif_index;
	unsigned char local_mac[6];
	unsigned char remote_mac[6];
	unsigned char echo_timer;
	unsigned char enhanced_timer;
	unsigned char enhanced_retry;
	unsigned char echo_timeout;
	unsigned char recover_timer;
	unsigned char recover_mode; /*recover mode select : 1--auto; 2--manual*/
}udld_netif_ctrl;


int udld_netif_update_enable(unsigned int netif_index, unsigned short netif_enable);
int udld_global_enable(unsigned char udld_enable);
int udld_global_update_echo_timer(unsigned char echo_timer);
int udld_global_update_echo_timeout(unsigned char echo_timeout);
int udld_global_update_enhanced_echo_timer(unsigned char enhanced_echo_timer);
int udld_global_update_enhanced_echo_retry(unsigned char enhanced_echo_retry);
int udld_global_update_recover_timer(unsigned char recover_timer);
int udld_global_update_mode(unsigned char mode);
int udld_global_update_recover_mode(unsigned char recover_mode);

#endif

