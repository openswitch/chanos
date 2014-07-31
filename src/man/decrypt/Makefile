ifndef CC
CC = $(COMPILER_PREFIX)gcc
endif
ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall
endif

INCLUDES += -I./ -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/lib -I$(MANAPI_DIR)
CFLAGS += $(INCLUDES)
ALL = decrypt
all: $(ALL)
OBJS = decrypt.o
BUILD_TARGET_OBJS += $(foreach module,$(OBJS), $(PROJECT_BUILD_DIR)/decrypt/$(module))
 
decrypt: $(BUILD_TARGET_OBJS)
	$(CC) $(LDFLAGS) $^ -o $(PROJECT_BUILD_DIR)/decrypt/$@
install: all
	mv $(PROJECT_BUILD_DIR)/decrypt/decrypt $(BUILD_ROOTFS_DIR)/usr/bin/
	chmod 777 $(BUILD_ROOTFS_DIR)/usr/bin/decrypt
	
clean:
	rm -f $(PROJECT_BUILD_DIR)/decrypt/*.o
	rm -f $(PROJECT_BUILD_DIR)/decrypt/decrypt
$(PROJECT_BUILD_DIR)/decrypt/%.o: %.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<


