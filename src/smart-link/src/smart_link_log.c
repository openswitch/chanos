#ifdef HAVE_SMART_LINK

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>

#include <pthread.h>

pthread_mutex_t smart_link_log_mutex = PTHREAD_MUTEX_INITIALIZER;

enum
{
    smart_link_log_level_all = 0xff,
    smart_link_log_level_debug = 0x01,
    smart_link_log_level_packet = 0x02,
    smart_link_log_level_event = 0x04,
    smart_link_log_level_error = 0x08,
    smart_link_log_level_max = 0xff
};

unsigned int smart_link_log_type = 0;
unsigned int smart_link_log_is_open = 0;

unsigned int smart_link_log_type_set(unsigned int type)
{
    smart_link_log_type |= type;

    if (!smart_link_log_is_open)
    {
        openlog("smartlink", 0, LOG_DAEMON); 
        smart_link_log_is_open = 1;
    }
        
    return 0;
}

unsigned int smart_link_log_type_unset(unsigned int type)
{
    smart_link_log_type &= ~type;

    if (smart_link_log_is_open && (!(smart_link_log_type & 0xff)))
    {
        closelog();
        smart_link_log_type = 0;
        smart_link_log_is_open = 0;
    }
    return 0;
}

#define smart_link_log_func(type)                        \
void smart_link_log_##type(char* format, ...)            \
{                                                        \
    va_list ap;                                          \
                                                         \
    if (!(smart_link_log_type & smart_link_log_level_##type))   \
    {                                                    \
        return ;                                         \
    }                                                    \
                                                         \
    if (!format)                                         \
    {                                                    \
        return ;                                         \
    }                                                    \
                                                         \
    va_start(ap, format);                                \
    pthread_mutex_lock(&smart_link_log_mutex);           \
    vsyslog(LOG_NOTICE, format, ap);                     \
    pthread_mutex_unlock(&smart_link_log_mutex);         \
    va_end(ap);                                          \
                                                         \
    return ;                                             \
}

smart_link_log_func(event)
smart_link_log_func(debug)
smart_link_log_func(packet)
smart_link_log_func(error)

int smart_link_sync_log_event(int pri, char* format, ...)
{
    va_list ap;

    if (!format)
    {
        return -1;
    }
    va_start(ap, format); 
    smart_link_log_event(format, ap);
    va_end(ap); 
    
    return 0;
}

#endif

