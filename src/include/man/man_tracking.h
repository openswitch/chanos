#ifndef __MAN_TRACKING_GROUP_
#define __MAN_TRACKING_GROUP_

#define NPD_TRACKING_GROUP_SIZE 64
#define NPD_TRACKING_MAX_OBJECT 32

enum
{
    tracking_mode_all,
    tracking_mode_any
};

enum
{
    netif_index_l2,
    netif_index_l3,
    netif_index_slot
};

struct track_object_query
{
    unsigned int netif_index;
    unsigned int netif_index_layer;
    unsigned int status;    /* 0:down; 1:up */
};

struct man_track_group_action_s
{
    int is_happened;
    unsigned int netif_index;
};

struct track_group_query
{
    int tracking_group_id;
    int tracking_mode;  /* default tracking_mode_all */
    int tracking_count;
    int tracking_up_count;
    unsigned int tracking_event;
    struct track_object_query tracking_object[NPD_TRACKING_MAX_OBJECT];

    struct man_track_group_action_s downlink;
#define dl_happened downlink.is_happened
#define dl_index downlink.netif_index
};

#endif

