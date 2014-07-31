#ifndef __SMART_LINK_H__
#define __SMART_LINK_H__

/* SL events */
enum sl_event 
{
    SL_EVENT,
    SL_MSG,
    SL_DBUS_READ,
    SL_DBUS_WRITE,
    SL_EVENT_MAX
};

void smart_link_event(enum sl_event , int );

#endif
