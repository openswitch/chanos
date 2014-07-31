#ifndef __COMMON_NPD_FDB_H__
#define __COMMON_NPD_FDB_H__

typedef struct {
	 unsigned short 	 vlanid;
	 unsigned char		 ether_mac[6];
	 unsigned int		 inter_type;
	 unsigned int		 value;
	 unsigned int        type_flag;
 }NPD_FDB;

struct fdb_entry_item_s {
	unsigned short      vlanid;
	unsigned char       mac[MAC_ADDR_LEN];
	unsigned char       blockMode;
	unsigned char       isStatic;	
	unsigned char       isBlock;
	unsigned char       isAuthen;
	unsigned char       isMirror;
	unsigned char       mirrorProfile;
	unsigned int        ifIndex;
    unsigned int        time;
	unsigned int 		flagApp;
};


struct npd_fdb_cfg_s
{
	unsigned int agingtime;
    unsigned int del_netif_index;
    unsigned int del_vlanid;
    unsigned int del_all;
};

struct fdb_number_limit_s{
    unsigned short vlanId;
	unsigned char slot_no;
	unsigned char local_port_no;
	unsigned int number;
};


enum{
	STATIC_FLAG = 0,
	BLACK_FLAG,
	MIRROR_FLAG,
	AUTH_FLAG
};

#define FDB_CONFIG_FAIL		1
#define FDB_CONFIG_SUCCESS	0

#define NPD_FDB_BLACKLIST_SMAC_MODE  0x1
#define NPD_FDB_BLACKLIST_DMAC_MODE  0x2
#define NPD_FDB_BLACKLIST_ALL_MODE   0x3
#define NPD_FDB_BLACKLIST_MODE_GET(flag) (1<<(flag))

 enum{
	NPD_FDB_ERR_NONE =0,
	NPD_FDB_ERR_DBUS,
	NPD_FDB_ERR_GENERAL,
	NPD_FDB_ERR_BADPARA,
	NPD_FDB_ERR_OCCUR_HW,
	NPD_FDB_ERR_ITEM_ISMIRROR,
	NPD_FDB_ERR_NODE_EXIST,
	NPD_FDB_ERR_NODE_NOT_EXIST,
	NPD_FDB_ERR_PORT_NOTIN_VLAN, /* 3->4*/
	NPD_FDB_ERR_VLAN_NONEXIST, /*1.5->5*/
	NPD_FDB_ERR_SYSTEM_MAC,
	NPD_FDB_ERR_VLAN_NO_PORT,
	NPD_FDB_ERR_HW_NOT_SUPPORT,
	NPD_FDB_ERR_MAX
};

 enum{
 FDB_STATIC_NOMIRROR = 0,
 FDB_STATIC_ISMIRROR
 };

#define FDB_APP_FLAG_VRRP 	0x1


#endif

