ifndef CC
CC = $(COMPILER_PREFIX)gcc
endif
ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall
endif

QUAGGA_DIR=${BUILD_DIR}/src/quagga/debsrc/quagga-0.99.17

INCLUDES = -I$(QUAGGA_DIR) -I$(QUAGGA_DIR)/${BUILDDIR} -I$(QUAGGA_DIR)/lib -I$(QUAGGA_DIR)/debsrc/quagga-0.99.17 -I$(QUAGGA_DIR)/debsrc/quagga-0.99.17/lib 


INCLUDES += -I./ -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/lib -I$(ACCAPI_DIR)/dbus/npd -I$(MANAPI_DIR)
INCLUDES += -I $(USER_ADDITION_LIB_ROOT_PATH)/include/dbus-1.0 
CFLAGS += $(INCLUDES)
CFLAGS += -DHAVE_CONFIG_H
LD_PATH = -L$(USER_ADDITION_LIB_ROOT_PATH)/lib
LDFLAGS = -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(LIB_EXPORT_DIR)
LDFLAGS += $(LD_PATH) 

ifeq ($(PRODUCT_SERIES), us3000)
LDFLAGS += -lman -ldbus-1 -lnpdlib -ltipc_api -lzcommon -lpthread -lxml2
else
LDFLAGS += -lman -ldbus-1 -lnpdlib -ltipc_api -lzcommon -lpthread -lxml2 -lz -liconv
endif
ALL = unpack
all: $(ALL)
OBJS = unpack.o

BUILD_TARGET_OBJS += $(foreach module,$(OBJS), $(PROJECT_BUILD_DIR)/unpack/$(module))
 
unpack: $(BUILD_TARGET_OBJS)
	$(CC) $(LDFLAGS) $(BUILD_TARGET_OBJS) -o $(PROJECT_BUILD_DIR)/unpack/$@
	mv $(PROJECT_BUILD_DIR)/unpack/$@ $(BUILD_ROOTFS_DIR)/usr/bin
	chmod 777 $(BUILD_ROOTFS_DIR)/usr/bin/$@
	
$(PROJECT_BUILD_DIR)/unpack/%.o: %.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<
clean:
	@rm -f $(PROJECT_BUILD_DIR)/unpack/*~  $(PROJECT_BUILD_DIR)/unpack/*.o $(PROJECT_BUILD_DIR)/unpack/unpack $(PROJECT_BUILD_DIR)/unpack/*.d
