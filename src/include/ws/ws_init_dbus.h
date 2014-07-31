#include <stdio.h>
#include <dbus/dbus.h>

#define CCGI_SUCCESS  0
#define CCGI_FAIL	-1


extern DBusConnection *ccgi_dbus_connection;

extern int ccgi_dbus_init(void);
extern int snmpd_dbus_init(void);

