ifndef CC
CC = $(COMPILER_PREFIX)gcc
endif
ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall
endif

INCLUDES += -I./ -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/lib -I$(MANAPI_DIR)
CFLAGS += $(INCLUDES)
ALL = file_split
all: $(ALL)
OBJS = file_split.o
BUILD_TARGET_OBJS += $(foreach module,$(OBJS), $(PROJECT_BUILD_DIR)/file_split/$(module))
 
file_split: $(BUILD_TARGET_OBJS)
	$(CC) $(LDFLAGS) $^ -o $(PROJECT_BUILD_DIR)/file_split/$@
install: all
	mv $(PROJECT_BUILD_DIR)/file_split/file_split $(BUILD_ROOTFS_DIR)/usr/bin/
	chmod 777 $(BUILD_ROOTFS_DIR)/usr/bin/file_split
	
clean:
	rm -f $(PROJECT_BUILD_DIR)/file_split/*.o
	rm -f $(PROJECT_BUILD_DIR)/file_split/file_split
$(PROJECT_BUILD_DIR)/file_split/%.o: %.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<


