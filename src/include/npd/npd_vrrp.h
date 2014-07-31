#ifndef __NPD_VRRP_H_
#define __NPD_VRRP_H_


#define NPD_VRRP_COMMON_PATH           "/tmp/npd_vrrp_common"

#define NPD_VRRP_MAX_SIZE  128

enum npd_vrrp_state
{
    NPD_VRRP_STATE_INIT = 0,
    NPD_VRRP_STATE_BACKUP,
    NPD_VRRP_STATE_MASTER,
};

enum
{
    NPD_VRRP_VIRTUAL_MAC = 0,
    NPD_VRRP_MAC
};

enum
{
    NPD_VRRP_IFNAME = 0,
    NPD_VRRP_MAC_MODE,
    NPD_VRRP_HWADDR,
    NPD_VRRP_VIRTUAL_IPADDR,
    NPD_VRRP_MAX
};

struct npd_vrrp_common
{
    int type;
    int length;
};

struct clv_attr
{
    int clv_type;
    int clv_length;
};

struct npd_vrrp_msg_buf
{
    struct npd_vrrp_common nvhdr;
    char clv_buf[512 - sizeof(struct npd_vrrp_common)];
};

#define NVMSG_ALIGNTO 4
#define NVMSG_ALIGN(len) (((len) + NVMSG_ALIGNTO - 1) & ~(NVMSG_ALIGNTO - 1))
#define NVMSG_LENGTH(len) ((len) + NVMSG_ALIGN(sizeof(struct npd_vrrp_common)))
#define NVMSG_DATA(nlh) ((void*) (((char*) nlh) + NVMSG_LENGTH(0)))

#define CLV_ALIGNTO 4
#define CLV_ALIGN(len) (((len) + CLV_ALIGNTO - 1) & ~(CLV_ALIGNTO - 1))

#define CLV_OK(clv, len) \
((len) > 0 && (clv)->clv_length >= sizeof(struct clv_attr) && (clv)->clv_length <= (len))

#define CLV_NEXT(clv, attrlen) \
((attrlen) -= CLV_ALIGN((clv)->clv_length), \
(struct clv_attr *) (((char *)(clv)) + CLV_ALIGN((clv)->clv_length)))

#define CLV_LENGTH(len)	(CLV_ALIGN(sizeof(struct clv_attr)) + (len))
//#define CLV_SPACE(len)	CLV_ALIGN(CLV_LENGTH(len))
#define CLV_DATA(clv)   ((void*)(((char*)(clv)) + CLV_LENGTH(0)))
//#define CLV_PAYLOAD(clv) ((int)((clv)->rta_len) - CLV_LENGTH(0))




#endif
