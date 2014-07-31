#ifndef __NPD_LOG_H__
#define __NPD_LOG_H__

enum
{
    NPD_LOG_FLAG_EVENT = 0x1,
    NPD_LOG_FLAG_PACKET_RCV = 0x2,
    NPD_LOG_FLAG_PACKET_SND = 0x4,
    NPD_LOG_FLAG_DB_INS = 0x8,
    NPD_LOG_FLAG_DB_DEL = 0x10,
    NPD_LOG_FLAG_DB_UPD  = 0x20,
    NPD_LOG_FLAG_ERR    = 0x40,
    NPD_LOG_FLAG_DBG    = 0x80,
    NPD_LOG_FLAG_WARNING    = 0x100
};

#define NPD_LOG_FLAG_PACKET (NPD_LOG_FLAG_PACKET_RCV|NPD_LOG_FLAG_PACKET_SND)
#define NPD_LOG_FLAG_DB (NPD_LOG_FLAG_DB_INS|NPD_LOG_FLAG_DB_DEL|NPD_LOG_FLAG_DB_UPD)
#define NPD_LOG_FLAG_ALL     0xffffffff

typedef struct npd_log_module_s
{
    char *module_user_name;
    char *inter_name;
    unsigned int interval_flag; /*logging info for developer, usually for debugging*/
    unsigned int flag;          /*logging info for user*/
}npd_log_module_t;

#define DECL_NPDLOG_MODULE_ERR_FUNC(name) \
    int npd_syslog_##name##_err(char *fmt,...) \
    {\
        if(npd_log_module_##name.interval_flag & NPD_LOG_FLAG_ERR)\
        {\
            va_list args; \
            va_start(args, fmt); \
            npd_syslog_lock();\
            vzlog(NULL, LOG_DEBUG, fmt, args); \
            npd_syslog_unlock();\
            va_end(args); \
        }\
        return 0;\
    }

#define DECL_NPDLOG_MODULE_DBG_FUNC(name) \
    int npd_syslog_##name##_dbg(char *fmt,...) \
    {\
        if(npd_log_module_##name.interval_flag & NPD_LOG_FLAG_DBG)\
        {\
            va_list args; \
            va_start(args, fmt); \
            npd_syslog_lock();\
            vzlog(NULL, LOG_DEBUG, fmt, args); \
            npd_syslog_unlock();\
            va_end(args); \
        }\
        return 0;\
    }
#define DECL_NPDLOG_MODULE_INTDBG_FUNC(name)\
    int npd_syslog_##name##_interval_dbg(char *fmt,...) \
    {\
        if(npd_log_module_##name.interval_flag & NPD_LOG_FLAG_DBG)\
        {\
            va_list args; \
            va_start(args, fmt); \
            npd_syslog_lock(); \
            vzlog(NULL, LOG_DEBUG, fmt, args); \
            npd_syslog_unlock(); \
            va_end(args); \
        }\
        return 0;\
    }
    
#define DECL_NPDLOG_MODULE_PKTRCV_FUNC(name) \
    int npd_syslog_##name##_pkt_rev(char *fmt,...) \
    {\
        if(npd_log_module_##name.interval_flag & NPD_LOG_FLAG_PACKET_RCV)\
        {\
            va_list args; \
            va_start(args, fmt); \
            npd_syslog_lock();\
            vzlog(NULL, LOG_DEBUG, fmt, args); \
            npd_syslog_unlock(); \
            va_end(args); \
        }\
        return 0;\
    }

#define DECL_NPDLOG_MODULE_PKTSND_FUNC(name) \
    int npd_syslog_##name##_pkt_send(char *fmt,...) \
    {\
        if(npd_log_module_##name.interval_flag & NPD_LOG_FLAG_PACKET_SND)\
        {\
            va_list args; \
            va_start(args, fmt); \
            npd_syslog_lock(); \
            vzlog(NULL, LOG_DEBUG, fmt, args); \
            npd_syslog_unlock(); \
            va_end(args); \
        }\
        return 0;\
    }

#define DECL_NPDLOG_MODULE_EVENT_FUNC(name) \
    int npd_syslog_##name##_event(char *fmt,...) \
    {\
        if(npd_log_module_##name.interval_flag & NPD_LOG_FLAG_EVENT)\
        {\
            va_list args; \
            va_start(args, fmt); \
            npd_syslog_lock(); \
            vzlog(NULL, LOG_DEBUG, fmt, args); \
            npd_syslog_unlock(); \
            va_end(args); \
        }\
        return 0;\
    }

#define DECL_NPDLOG_MODULE_WARNING_FUNC(name) \
    int npd_syslog_##name##_warning(char *fmt,...) \
    {\
        if(npd_log_module_##name.interval_flag & NPD_LOG_FLAG_WARNING)\
        {\
            va_list args; \
            va_start(args, fmt); \
            npd_syslog_lock(); \
            vzlog(NULL, LOG_DEBUG, fmt, args); \
            npd_syslog_unlock(); \
            va_end(args); \
        }\
        return 0;\
    }



#define DECLARE_LOG_MODULE_SET(name, int_name) \
    npd_log_module_t npd_log_module_##int_name = {#name, #int_name}; \
    DECL_NPDLOG_MODULE_ERR_FUNC(int_name) \
    DECL_NPDLOG_MODULE_DBG_FUNC(int_name) \
    DECL_NPDLOG_MODULE_PKTRCV_FUNC(int_name) \
    DECL_NPDLOG_MODULE_PKTSND_FUNC(int_name) \
    DECL_NPDLOG_MODULE_EVENT_FUNC(int_name) \
    DECL_NPDLOG_MODULE_WARNING_FUNC(int_name) \
    DECL_NPDLOG_MODULE_INTDBG_FUNC(int_name)

#define DECLARE_LOG_MODULE_SET_HEADER(name) \
    extern npd_log_module_t npd_log_module_##name; \
    int npd_syslog_##name##_err(char *fmt, ...); \
    int npd_syslog_##name##_dbg(char *fmt, ...); \
    int npd_syslog_##name##_interval_dbg(char *fmt, ...); \
    int npd_syslog_##name##_pkt_rev(char *fmt, ...); \
    int npd_syslog_##name##_pkt_send(char *fmt, ...); \
    int npd_syslog_##name##_event(char *fmt, ...); \
    int npd_syslog_##name##_warning(char *fmt, ...); 
    

#define NPD_LOG_MODULE(name) npd_log_module_##name 
#define NPD_LOG_MODULE_FLAG_SET(name, flag)     (npd_log_module_##name.interval_flag & flag)
#define NPD_LOG_MODULE_FORMAL_FLAG_SET(name, flag) (npd_log_module_##name.flag & flag)

DECLARE_LOG_MODULE_SET_HEADER(all)
DECLARE_LOG_MODULE_SET_HEADER(asic)
DECLARE_LOG_MODULE_SET_HEADER(trunk)
DECLARE_LOG_MODULE_SET_HEADER(fdb)
DECLARE_LOG_MODULE_SET_HEADER(eth_port)
DECLARE_LOG_MODULE_SET_HEADER(switchport)
DECLARE_LOG_MODULE_SET_HEADER(vlan)
DECLARE_LOG_MODULE_SET_HEADER(mirror)
DECLARE_LOG_MODULE_SET_HEADER(intf)
DECLARE_LOG_MODULE_SET_HEADER(pvlan)
DECLARE_LOG_MODULE_SET_HEADER(igmp)
DECLARE_LOG_MODULE_SET_HEADER(route)
DECLARE_LOG_MODULE_SET_HEADER(arpsnooping)
DECLARE_LOG_MODULE_SET_HEADER(acl)
DECLARE_LOG_MODULE_SET_HEADER(qos)
DECLARE_LOG_MODULE_SET_HEADER(dbus)
DECLARE_LOG_MODULE_SET_HEADER(cslot)
DECLARE_LOG_MODULE_SET_HEADER(e_slot)
DECLARE_LOG_MODULE_SET_HEADER(product)
DECLARE_LOG_MODULE_SET_HEADER(main)
DECLARE_LOG_MODULE_SET_HEADER(rstp)
DECLARE_LOG_MODULE_SET_HEADER(dldp)
DECLARE_LOG_MODULE_SET_HEADER(dhcp_snp)
DECLARE_LOG_MODULE_SET_HEADER(asd)


/* Extern debug flag. */
extern char *npd_log_module_name[];
extern npd_log_module_t *conf_npd_log_module[];

#define npd_syslog_dbg npd_syslog_all_dbg
#define npd_syslog_err npd_syslog_all_err
#define npd_syslog_event npd_syslog_all_event
#define npd_syslog_pkt_rev npd_syslog_all_pkt_rev
#define npd_syslog_pkt_send npd_syslog_all_pkt_send
#define npd_syslog_warning  npd_syslog_all_warning
#define npd_syslog_dbg_internal npd_syslog_all_interval_dbg


/** Logging levels for NPD daemon
 */

#define NPD_OK              0
#define NPD_ERR             (NPD_OK + 1)


enum {
	SYSLOG_DBG_DEF     = 0x0,	/* default value*/
	SYSLOG_DBG_DBG     = 0x1,	/*normal */
	SYSLOG_DBG_WAR     = 0x2,	/*warning*/
	SYSLOG_DBG_ERR     = 0x4,	/* error*/
	SYSLOG_DBG_EVT     = 0x8,	/* event*/
	SYSLOG_DBG_PKT_REV = 0x10,  /*packet receive*/
	SYSLOG_DBG_PKT_SED = 0x20,  /*packet send*/
	SYSLOG_DBG_PKT_ALL = 0x30,  /*packet send and receive*/
	SYSLOG_DBG_INTERNAL = 0x40, /*internal debug*/
	SYSLOG_DBG_ALL = 0xFF	/* all*/
};

int npd_log_set_debug_value(unsigned int val_mask);

#define npd_syslog_formal_event(name, expr) \
    do \
    { \
        if(npd_log_module_##name.flag & NPD_LOG_FLAG_EVENT)\
        {\
            npd_syslog_lock(); \
            zlog_notice expr; \
            npd_syslog_unlock(); \
        }\
    }while(0)

#define npd_syslog_formal_err(name, expr) \
    do \
    { \
        if(npd_log_module_##name.flag & NPD_LOG_FLAG_ERR)\
        {\
            npd_syslog_lock(); \
            zlog_err expr; \
            npd_syslog_unlock(); \
        }\
    }while(0)

#define npd_syslog_formal_pkt_rcv(name, expr) \
    do \
    { \
        if(npd_log_module_##name.flag & NPD_LOG_FLAG_PACKET_RCV)\
        {\
            npd_syslog_lock(); \
            zlog_debug expr; \
            npd_syslog_unlock(); \
        }\
    }while(0)
            
#define npd_syslog_formal_pkt_snd(name, expr) \
    do \
    { \
        if(npd_log_module_##name.flag & NPD_LOG_FLAG_PACKET_SND)\
        {\
            npd_syslog_lock(); \
            zlog_debug expr; \
            npd_syslog_unlock(); \
        }\
    }while(0)

#define npd_syslog_formal_warning(name, expr) \
    do \
    { \
        if(npd_log_module_##name.flag & NPD_LOG_FLAG_WARNING)\
        {\
            npd_syslog_lock(); \
            zlog_warning expr; \
            npd_syslog_unlock(); \
        }\
    }while(0)

#define npd_syslog_formal_dbg(name, expr) \
    do \
    { \
        if(npd_log_module_##name.flag & NPD_LOG_FLAG_DBG)\
        {\
            npd_syslog_lock(); \
            zlog_debug expr; \
            npd_syslog_unlock(); \
       }\
    }while(0)
            
void npd_syslog_official_event(char *format,...);

#define DECLARE_LOG_MODULE_SET_AX(name) \
   extern int syslog_ax_##name##_err(char *format,...) __attribute__((alias("npd_syslog_"#name"_err"))); \
   extern int syslog_ax_##name##_dbg(char *format,...) __attribute__((alias("npd_syslog_"#name"_dbg"))); \
   extern int syslog_ax_##name##_pkt_rev(char *format,...) __attribute__((alias("npd_syslog_"#name"_pkt_rev"))); \
   extern int syslog_ax_##name##_pkt_send(char *format,...) __attribute__((alias("npd_syslog_"#name"_pkt_send"))); \
   extern int syslog_ax_##name##_warning(char *format,...) __attribute__((alias("npd_syslog_"#name"_warning"))); \
   extern int syslog_ax_##name##_event(char *format,...) __attribute__((alias("npd_syslog_"#name"_event")));

#define DECLARE_LOG_MODULE_SET_AX_HEADER(name) \
   extern int syslog_ax_##name##_err(char *format,...); \
   extern int syslog_ax_##name##_dbg(char *format,...); \
   extern int syslog_ax_##name##_pkt_rev(char *format,...); \
   extern int syslog_ax_##name##_pkt_send(char *format,...); \
   extern int syslog_ax_##name##_warning(char *format,...); \
   extern int syslog_ax_##name##_event(char *format,...);

    

DECLARE_LOG_MODULE_SET_AX_HEADER(trunk)
DECLARE_LOG_MODULE_SET_AX_HEADER(fdb)
DECLARE_LOG_MODULE_SET_AX_HEADER(eth_port)
DECLARE_LOG_MODULE_SET_AX_HEADER(intf)
DECLARE_LOG_MODULE_SET_AX_HEADER(pvlan)
DECLARE_LOG_MODULE_SET_AX_HEADER(vlan)
DECLARE_LOG_MODULE_SET_AX_HEADER(igmp)
DECLARE_LOG_MODULE_SET_AX_HEADER(route)
DECLARE_LOG_MODULE_SET_AX_HEADER(arpsnooping)
DECLARE_LOG_MODULE_SET_AX_HEADER(acl)
DECLARE_LOG_MODULE_SET_AX_HEADER(qos)
DECLARE_LOG_MODULE_SET_AX_HEADER(e_slot)
DECLARE_LOG_MODULE_SET_AX_HEADER(mirror)
DECLARE_LOG_MODULE_SET_AX_HEADER(dbus)
DECLARE_LOG_MODULE_SET_AX_HEADER(product)
DECLARE_LOG_MODULE_SET_AX_HEADER(rstp)
DECLARE_LOG_MODULE_SET_AX_HEADER(main)
DECLARE_LOG_MODULE_SET_AX_HEADER(dldp)
DECLARE_LOG_MODULE_SET_AX_HEADER(dhcp_snp)
DECLARE_LOG_MODULE_SET_AX_HEADER(asd)

/* syslog for chassis slot*/
#define syslog_ax_c_slot_err          npd_syslog_cslot_err
#define syslog_ax_c_slot_dbg          npd_syslog_cslot_dbg
#define syslog_ax_c_slot_event        npd_syslog_cslot_event
#define syslog_ax_c_slot_warning      npd_syslog_cslot_warning
#define syslog_ax_c_slot_pkt_rev      npd_syslog_cslot_pkt_rev
#define syslog_ax_c_slot_pkt_send     npd_syslog_cslot_pkt_send

/* syslog for system forwarding engine*/
#define syslog_ax_engine_err          npd_syslog_asic_err
#define syslog_ax_engine_dbg          npd_syslog_asic_dbg 
#define syslog_ax_engine_event        npd_syslog_asic_event 
#define syslog_ax_engine_pkt_rev      npd_syslog_asic_pkt_rev 
#define syslog_ax_engine_pkt_send     npd_syslog_asic_pkt_send
#define syslog_ax_engine_warning      npd_syslog_asic_warning 

#define NAM_OK              0
#define NAM_ERR             (NAM_OK + 1)
#define NAM_SDK_UNSUPPORT  (NAM_OK +2)

#define DECLARE_LOG_MODULE_SET_NAM(name) \
    int nam_syslog_##name##_err(char *format,...) __attribute__((alias("npd_syslog_"#name"_err"))); \
    int nam_syslog_##name##_dbg(char *format,...) __attribute__((alias("npd_syslog_"#name"_dbg"))); \
    int nam_syslog_##name##_pkt_rev(char *format,...) __attribute__((alias("npd_syslog_"#name"_pkt_rev"))); \
    int nam_syslog_##name##_pkt_send(char *format,...) __attribute__((alias("npd_syslog_"#name"_pkt_send"))); \
    int nam_syslog_##name##_warning(char *format,...) __attribute__((alias("npd_syslog_"#name"_warning"))); \
    int nam_syslog_##name##_event(char *format,...) __attribute__((alias("npd_syslog_"#name"_event")));\
    int syslog_ax_nam_##name##_err(char *format,...) __attribute__((alias("npd_syslog_"#name"_err"))); \
    int syslog_ax_nam_##name##_dbg(char *format,...) __attribute__((alias("npd_syslog_"#name"_dbg"))); \
    int syslog_ax_nam_##name##_pkt_rev(char *format,...) __attribute__((alias("npd_syslog_"#name"_pkt_rev"))); \
    int syslog_ax_nam_##name##_pkt_send(char *format,...) __attribute__((alias("npd_syslog_"#name"_pkt_send"))); \
    int syslog_ax_nam_##name##_warning(char *format,...) __attribute__((alias("npd_syslog_"#name"_warning"))); \
    int syslog_ax_nam_##name##_event(char *format,...) __attribute__((alias("npd_syslog_"#name"_event")));

#define DECLARE_LOG_MODULE_SET_NAM_HEADER(name) \
    int nam_syslog_##name##_err(char *format,...); \
    int nam_syslog_##name##_dbg(char *format,...) ; \
    int nam_syslog_##name##_pkt_rev(char *format,...); \
    int nam_syslog_##name##_pkt_send(char *format,...) ; \
    int nam_syslog_##name##_warning(char *format,...); \
    int nam_syslog_##name##_event(char *format,...);\
    int syslog_ax_nam_##name##_err(char *format,...); \
    int syslog_ax_nam_##name##_dbg(char *format,...); \
    int syslog_ax_nam_##name##_pkt_rev(char *format,...); \
    int syslog_ax_nam_##name##_pkt_send(char *format,...); \
    int syslog_ax_nam_##name##_warning(char *format,...); \
    int syslog_ax_nam_##name##_event(char *format,...);
     
#define nam_syslog_dbg npd_syslog_asic_dbg 
#define nam_syslog_err npd_syslog_asic_err 
#define nam_syslog_event npd_syslog_asic_event
#define nam_syslog_pkt_rx npd_syslog_asic_pkt_rev
#define nam_syslog_pkt_tx npd_syslog_asic_pkt_send
#define nam_syslog_warning  npd_syslog_asic_warning

#define syslog_ax_nam_eth_dbg npd_syslog_eth_port_dbg 
#define syslog_ax_nam_arp_err npd_syslog_arpsnooping_err
#define syslog_ax_nam_arp_dbg npd_syslog_arpsnooping_dbg

DECLARE_LOG_MODULE_SET_NAM_HEADER(trunk)
DECLARE_LOG_MODULE_SET_NAM_HEADER(fdb)
DECLARE_LOG_MODULE_SET_NAM_HEADER(eth_port)
DECLARE_LOG_MODULE_SET_NAM_HEADER(acl)
DECLARE_LOG_MODULE_SET_NAM_HEADER(asic)
DECLARE_LOG_MODULE_SET_NAM_HEADER(vlan)
DECLARE_LOG_MODULE_SET_NAM_HEADER(mirror)
DECLARE_LOG_MODULE_SET_NAM_HEADER(route)
DECLARE_LOG_MODULE_SET_NAM_HEADER(intf)



enum {
	NPD_LOGPRI_TRACE = (1 << 0),   /**< function call sequences */
	NPD_LOGPRI_DEBUG = (1 << 1),   /**< debug statements in code */
	NPD_LOGPRI_INFO = (1 << 2),    /**< informational level */
	NPD_LOGPRI_WARNING = (1 << 3), /**< warnings */
	NPD_LOGPRI_ERROR = (1 << 4)    /**< error */
};


int npd_asic_debug_set
(
	unsigned int level
);

int npd_asic_debug_clr
(
	unsigned int level
);

int npd_log_init();


#endif				/* LOGGER_H */
