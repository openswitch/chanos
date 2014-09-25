
TOPDIR 	= .
RM		= rm 

CC 		= $(COMPILER_PREFIX)gcc #-E
CFLAGS += $(INCLUDE_CHANOS_MODULES) -g -Wall -D__USE_GNU -D_GNU_SOURCE -D_REENTRANT -D__LINUX__ -I$(USER_ADDITION_LIB_ROOT_PATH)/include -I$(ACCAPI_DIR)
CPPFLAGS +=  $(INCLUDE_CHANOS_MODULES) -I$(USER_ADDITION_LIB_ROOT_PATH)/usr/include -ansi --std=c99
LDFLAGS +=  -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(USER_ADDITION_LIB_ROOT_PATH)/usr/lib -L$(LIB_EXPORT_DIR) -L${BUILD_ROOTFS_DIR}/opt/lib
INCLUDES = -I. 

CPPFLAGS += $(DBUS_INCLUDE)

COMPILE = $(CC) $(INCLUDES) $(CFLAGS)

EXECUTABLE=smartlink

DBUS_LIBS = dbus-1  -lpthread -lrt -lnpdlib -ltipc_api -lzcommon
.SUFFIXES: .S .c .o .s

all: $(EXECUTABLE)

SMART_LINK_OBJS = smart_link_log.o smart_link_dbus.o smart_link_packet.o smart_link_event.o  smart_link.o

BUILD_SMART_LINK_OBJS = $(foreach module,$(SMART_LINK_OBJS), $(PROJECT_BUILD_DIR)/$(EXECUTABLE)/$(module))

smartlink: $(BUILD_SMART_LINK_OBJS)
	$(CC) -g $(LIBS) $(LDFLAGS) -L. -l$(DBUS_LIBS) $^ -o $(PROJECT_BUILD_DIR)/$(EXECUTABLE)/$@

clean:
	$(RM) -f all *.o *.a *~ $(EXECUTABLE) *.bak make.depend $(PROJECT_BUILD_DIR)/$(EXECUTABLE)/*

depend:
	echo "# DO NOT DELETE THIS LINE -- make  depend  depends  on it." > make.depend
	makedepend -- $(DEFS) $(INCLUDES) $(CFLAGS) -- $(CFILES) -f make.depend

$(PROJECT_BUILD_DIR)/$(EXECUTABLE)/%.o: %.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) -o $@ $<
	
-include make.depend
