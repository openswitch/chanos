ifndef CC
CC = $(COMPILER_PREFIX)gcc
endif
ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall
endif

INCLUDES += -I./ -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/lib -I$(MANAPI_DIR)
CFLAGS += $(INCLUDES) $(INCLUDE_AUTEWARE_MODULES)
OBJS = command_db_sync.o user_configure.o
TEST_OBJS = test_echo.o

LDFLAGS = -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(LIB_EXPORT_DIR) -lpthread -ltipc_api -lnpdlib -lzcommon -lman -ldbus-1 -lxml2 -lz 
ifeq ($(PRODUCT_SERIES), topsec)
LDFLAGS = -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(LIB_EXPORT_DIR) -lpthread -ltipc_api -lnpdlib -lzcommon -ldbus-1  -lz 
endif
ifeq ($(PRODUCT_SERIES), us3000)
LDFLAGS = -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(LIB_EXPORT_DIR) -lpthread -ltipc_api -lnpdlib -lzcommon -ldbus-1 -lman
endif
ifeq ($(PRODUCT_SERIES), as3000)
LDFLAGS = -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(LIB_EXPORT_DIR) -lpthread -ltipc_api -lnpdlib -lzcommon -ldbus-1 -lman
endif
ifeq ($(PRODUCT_SERIES), ds5600)
LDFLAGS = -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(LIB_EXPORT_DIR) -lpthread -ltipc_api -lnpdlib -lzcommon -ldbus-1 -lman
endif

ALL = confsyncd testecho
all: $(ALL)

BUILD_TARGET_OBJS += $(foreach module,$(OBJS), $(PROJECT_BUILD_DIR)/confsyncd/$(module))
BUILD_TEST_OBJS += $(foreach module,$(TEST_OBJS), $(PROJECT_BUILD_DIR)/confsyncd/$(module))

confsyncd: $(BUILD_TARGET_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
testecho: $(BUILD_TEST_OBJS)	
	$(CC) $^ -o $@
install: all
	mv confsyncd $(BUILD_ROOTFS_DIR)/opt/bin
	mv testecho $(BUILD_ROOTFS_DIR)/usr/bin

clean:
	rm -f core *~ *.o *.so confsyncd 
	rm -f *.d
	for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) -C $$d clean; done
	rm -f *~
	rm -f $(PROJECT_BUILD_DIR)/confsyncd/*.o

$(PROJECT_BUILD_DIR)/confsyncd/%.o: %.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<

%.eps: %.fig
	fig2dev -L eps $*.fig $*.eps

%.png: %.fig
	fig2dev -L png -m 3 $*.fig | pngtopnm | pnmscale 0.4 | pnmtopng \
		> $*.png


-include $(OBJS:%.o=%.d)
-include $(TEST_OBJS:%.o=%.d)

