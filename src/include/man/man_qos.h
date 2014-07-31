#ifndef __MAN_QOS_H__
#define __MAN_QOS_H__

struct qos_port_show_s
{
    int egressMark;
    int ingressMark;
    unsigned int qosProfileIndex;
    unsigned int poMapId;
    int trust;
};

struct qos_policy_show_s
{
    int index;
    int domain;
    struct
    {
        int profile_id;
    }in_up_map[8];
    struct
    {
        int profile_id;
    }out_up_map[8];
    struct
    {
        int profile_id;
    }in_dscp_map[64];
    struct
    {
        int profile_id;
    }out_dscp_map[64];
};

typedef struct policer_stats_s
{
	unsigned int policer_index;
	unsigned long long conform_bytes;
	unsigned long long conform_pkts;
	unsigned long long exceed_bytes;
	unsigned long long exceed_pkts;
	unsigned long long violate_bytes;
	unsigned long long violate_pkts;
}policer_stats_t;

#endif