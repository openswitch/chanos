ifndef CC
CC = $(COMPILER_PREFIX)gcc
endif
ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall 
endif

APP_TARGET_NAME = libnpdlib

CFLAGS += -fPIC -D_GNU_SOURCE -D__USE_UNIX98 -D__USE_GNU $(INCLUDE_AUTEWARE_MODULES)
CFLAGS += -I.
CFLAGS += -I$(USER_ADDITION_LIB_ROOT_PATH)/include -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/lib -I$(ACCAPI_DIR)/npd 
#CFLAGS += -DTIPC_API_TEST

OBJS =	npd_database.o npd_bitarray.o netif_index.o netif_array_port.o netif_app_event.o db_user_api.o db_server.o common_api.o db_app_sync.o chassis_man_app.o

LDFLAGS = -g -L$(LIB_EXPORT_DIR) -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -ltipc_api 

DB_OBJS = npd_database.o
DB_LDFLAGS = -g 

ALL = libbase

all: $(ALL)

BUILD_TARGET_OBJS += $(foreach module,$(OBJS), $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(module))

BUILD_DB_OBJS += $(foreach module,$(DB_OBJS), $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(module))

libbase: $(BUILD_TARGET_OBJS) libnpddb
	$(CC) -shared $(LDFLAGS) $(BUILD_TARGET_OBJS) -o libnpdlib.so
	cp libnpdlib.so $(USER_ADDITION_LIB_ROOT_PATH)/lib

libnpddb: $(BUILD_DB_OBJS)
	$(CC) -shared $(DB_LDFLAGS) $^ -o libnpddb.so
	cp libnpddb.so $(USER_ADDITION_LIB_ROOT_PATH)/lib
	
install: all
	for i in $(ALL); do cp $$i $(USER_ADDITION_LIB_ROOT_PATH)/bin/$$i; done

clean:
	rm -f core *~ *.o *.so
	rm -f *.d
	rm -f *~
	rm -f $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/*

$(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) -o $@ $<

%.eps: %.fig
	fig2dev -L eps $*.fig $*.eps

%.png: %.fig
	fig2dev -L png -m 3 $*.fig | pngtopnm | pnmscale 0.4 | pnmtopng \
		> $*.png


-include $(OBJS:%.o=%.d)

