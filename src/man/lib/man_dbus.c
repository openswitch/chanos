#include <dbus/dbus.h>
#include <stdio.h>
#include "man_dbus.h"


DBusConnection *config_dbus_connection = NULL;
DBusConnection *dcli_dbus_connection = NULL;
DBusConnection *web_dbus_connection = NULL;
int config_dbus_connection_init(void) {
	DBusError dbus_error;

	dbus_error_init (&dbus_error);
	if(NULL == config_dbus_connection) {
		config_dbus_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
		if (config_dbus_connection == NULL) {
			printf ("dbus_bus_get(): %s", dbus_error.message);
			return -1;
		}
		dbus_bus_request_name(config_dbus_connection,"aw.config",0,&dbus_error);
		dcli_dbus_connection = config_dbus_connection;
		web_dbus_connection = config_dbus_connection;
		if (dbus_error_is_set(&dbus_error)) {
			printf("request name failed: %s", dbus_error.message);
			return -1;
		}
		

	}
	return 0;
}


int config_dbus_connection_deinit(void) {
	
	if(config_dbus_connection)
		dbus_connection_unref(config_dbus_connection);
	
	return 0;
}
