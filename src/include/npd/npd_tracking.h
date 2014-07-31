#ifndef __NPD_TRACKING_H_
#define __NPD_TRACKING_H_

#define NPD_TRACKING_GROUP_SIZE 64
#define NPD_TRACKING_MAX_OBJECT 32
#define NPD_TRACKING_RAW_LEN    128
#define NPD_TRACKING_MAX_COMMAND_LINE_LEN   45
#define NPD_TRACKING_GROUP_BUFFER_LEN    (NPD_TRACKING_RAW_LEN + NPD_TRACKING_MAX_COMMAND_LINE_LEN *  NPD_TRACKING_MAX_OBJECT)
#define NPD_TRACKING_GROUP_SHOW_RUNNING_BUFFER_LEN    \
    ((int)(((NPD_TRACKING_GROUP_BUFFER_LEN * NPD_TRACKING_GROUP_SIZE) + 128 - 1) / 128 ) * 128)

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

struct track_object
{
    unsigned int netif_index_layer;
    unsigned int netif_index;
    unsigned int status;    /* 0:down; 1:up */
};

struct track_group_action_s
{
    int is_happened;
    unsigned int netif_index;
};

struct track_group
{
    int tracking_group_id;
    int tracking_mode;  /* default tracking_mode_all */
    int tracking_count;
    int tracking_up_count;
    unsigned int tracking_event;
    struct track_object tracking_object[NPD_TRACKING_MAX_OBJECT];

    struct track_group_action_s downlink;
#define dl_happened downlink.is_happened
#define dl_index downlink.netif_index
};

void npd_tracking_init(void); 

#endif

