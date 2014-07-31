#ifndef __MAN_DHCLIENT_H__
#define __MAN_DHCLIENT_H__

#define DHCLIENT_DAY_HOURS   24
#define DHCLIENT_HOUR_MINUTES   60
#define DHCLIENT_MINUTE_SECONDS   60

#define DHCLINET_MUTEX_SERVER   1
#define DHCLINET_MUTEX_RELAY    2

enum dhclient_dhcp_e
{
    dhclient_no_dhcp = 0,
    dhclient_dhcp,
};

enum dhclient_action_e
{
    dhclient_release = 0,
    dhclient_renew
};

#define MAN_DHCLIENT_IFNAME_LENGTH_MAX   20
#define MAN_DHCLIENT_STRING_LENGTH_MAX   64
#define MAN_DHCLIENT_DB_OPTION_LENGTH_MAX   768

struct man_dhclient_interface_s
{
    char    ifname[MAN_DHCLIENT_IFNAME_LENGTH_MAX];
    char    ipaddr[MAN_DHCLIENT_IFNAME_LENGTH_MAX];
    char    client_id[MAN_DHCLIENT_STRING_LENGTH_MAX];
    char    class_id[MAN_DHCLIENT_STRING_LENGTH_MAX];
    unsigned int interface_enable;
    unsigned int request_time;
    unsigned int default_route_flag;
    unsigned int option_request_bit[256 / sizeof(unsigned int)];
    unsigned int expiry;
};

#endif
