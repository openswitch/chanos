
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 * Class map function.
 * Copyright (C) 2011 Autelan Co.,Ltd.
 *
 * Author: chengjun@autelan.com
 * Version: 1.0
 */

#ifndef _NPD_CLASSMAP_H
#define _NPD_CLASSMAP_H

#include "util/npd_list.h"
#include "lib/npd_bitop.h"

#define CLASSMAP_ACL_ID_INDEX_START         5
#define CLASSMAP_SERVICE_INDEX_START        3006
#define CLASSMAP_SERVICE_PORTAL_STA_INDEX_START     3601
#define CLASSMAP_SERVICE_PORTAL_INTF_INDEX_START    3941
#define CLASSMAP_SERVICE_SG_INDEX_START        4005
#define RULE_ENTRY_FIRST_ID         20
#define RULE_ENTRY_MAX              64
#define CLASSMAP_PHASE         5

/* Class map rule. This rule has both `match' rule and `set' rule. */
#define NGCMD_BUFFSIZE		100*1024
#define MAX_CLASSMAP_RULE_NUM				20000

/* Depth limit in RMAP recursion using RMAP_CALL. */
#define CMAP_RECURSION_LIMIT      10
#define CMAP_MAX_INDEX_NUM        2000
#define CMAP_MAX_RULE_NUM         6000

enum
{
    RULE_NO_USED,
    RULE_USED,
    RULE_DELETED
};

typedef struct entry_rule_s
{
    struct entry_rule_s *next;
    int flag;
    int entryId;
    int phase;
    int offset;
    int unit;
    void *rule; /*acl rule struct*/
}entry_rule;

typedef struct service_index_rule_s
{
    entry_rule *head;
    int service_index_flag;
}service_index_rule_t;

typedef struct service_index_s
{
    int max_rule_num;
    int rule_entry_size;
    int rule_entry_max;
    int (*nam_set_entry)(int entryId, void *rule, int flag, char lkphase, unsigned char unit);
    service_index_rule_t* service_rule;
} service_index_t;

struct time_s
{
    int year;   
    int month;  
    int day;    
    int hh;  
    int mm;  
    int what_day;
};

struct time_abs_peri_s
{
    int flag;   
    struct time_s start_time; 
    struct time_s end_time;   
};

struct time_range_info_s
{
    char name[16];  
    int index;      
    int acl_bind_count;
    struct time_abs_peri_s abs_time;  
    struct time_abs_peri_s periodic_time; 
};




#if 0
/* show running for servicepolicy. */
char* acl_service_show_running_config(char* showStr, int* safe_len);
#endif

#endif

