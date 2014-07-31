# Network Platform Daemon 
#

APP_TARGET_NAME = npd

CC  = $(COMPILER_PREFIX)gcc


CFLAGS = -g -Wall -fPIC #-ansi

INCLUDES += -I$(SDK_TOPDIR)/mainPpDrv/h
INCLUDES += -I$(SDK_TOPDIR)/common/h
INCLUDES += -I$(SDK_TOPDIR)/cpssEnabler/mainOs/h
CFLAGS += -DINCLUDE_L3 -DDRV_LIB_CPSS -DDRV_LIB_CPSS_XCAT
INCLUDES += -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/npd -I$(ACCAPI_DIR)/lib -I$(ACCAPI_DIR)/nam

# add for dhcp snooping db
INCLUDES += -I$(USER_ADDITION_LIB_ROOT_PATH)/include

NPD_CHASM_SPEC_OBJS = as_product_init.o as_product_info.o

NPD_CHASM_SPEC_DEPFILES = as.dep

DC_LIST := $(patsubst %.o,%.dc,$(NPD_CHASM_SPEC_OBJS))

BUILD_CHASM_OBJS += $(foreach module,$(NPD_CHASM_SPEC_OBJS), $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(module))

.PHONY: all
all: $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(NPD_CHASM_SPEC_DEPFILES) $(BUILD_CHASM_OBJS)

dep: $(DC_LIST)
	@echo building file dependency done...
cleandep:
	rm -rf $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(NPD_CHASM_SPEC_DEPFILES)
	
$(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(NPD_CHASM_SPEC_DEPFILES):cleandep dep

include $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(NPD_CHASM_SPEC_DEPFILES)

$(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -o $@ $<

%.dc: %.c
	$(CC) $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -MM $< >>  $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(NPD_CHASM_SPEC_DEPFILES)
		
clean:
	rm -rf *~ $(NPD_CHASM_SPEC_DEPFILES) *.o

