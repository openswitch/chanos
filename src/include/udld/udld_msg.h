#ifndef _UDLD_MSG_H_
#define _UDLD_MSG_H_

#define UDLD_MSG_NPD_2_UDLD			"/tmp/npd2udld.rx"
#define UDLD_MSG_UDLD_2_NPD			"/tmp/udld2npd.tx"

#define MSG_UDLD_2_NPD				1380
#define MSG_NETIF_ADMIN_OP			1

typedef struct _udld_ctrl_msg_
{
	struct nlmsghdr nlh;
	char buffer[0];
}udld_ctrl_msg;

typedef struct _udld_netif_err_disable_
{
	unsigned int netif_index;
	unsigned int opcode;
}udld_netif_disable_msg;

int udld_netif_err_disable(unsigned int netif_index);
int udld_netif_recover_enable(unsigned int netif_index);
int udld_msg_init();

#endif

