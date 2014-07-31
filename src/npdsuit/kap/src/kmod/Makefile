#cross-compile environment settings
DIST_HOME       = ${KERNEL_ROOT}

ifeq ($(DRV_LIB_FLAG),CPSS_XCAT)
EXTRA_CFLAGS += -DDRV_LIB_CPSS -DDRV_LIB_CPSS_XCAT
SDK_INCLUDES += -I$(SDK_TOPDIR)/mainPpDrv/h
SDK_INCLUDES += -I$(SDK_TOPDIR)/common/h
SDK_INCLUDES += -I$(SDK_TOPDIR)/cpssEnabler/mainOs/h
EXTRA_CFLAGS += -I$(DIST_HOME)/include -I$(SUBDIRS)/include ${SDK_INCLUDES}
else
ifeq ($(DRV_LIB_FLAG),ATHEROS)
ASDK_TOP= ${NPDSUIT_MOD}/$(SDK_TYPE)_$(SDK_VERSION)
ASDK_OSAL_PATH = $(ASDK_TOP)/osal
DRV_SWITCH_PATH = $(ASDK_TOP)/driver/switch
SYS_APC_INC_PATH = $(ASDK_TOP)/system/apc/linux/um/include
EXTRA_CFLAGS += -DDRV_LIB_ATHEROS
SDK_INCLUDES += -I$(ASDK_TOP)/system/cpusub/music/linux/2.6.31/kernel/arch/inc
SDK_INCLUDES += -I$(ASDK_TOP)/osal/include
SDK_INCLUDES += -I$(ASDK_TOP)/osal/include/linux/kernel
SDK_INCLUDES += -I$(ASDK_OSAL_PATH)/include \
								-I$(ASDK_OSAL_PATH)/include/linux/kernel \
								-I$(DRV_SWITCH_PATH)/include \
								-I$(SYS_SWITCH_PATH)/common/include \
								-I$(SYS_APC_INC_PATH)
SDK_INCLUDES += -I$(ASDK_TOP)/driver/cpusub/music
EXTRA_CFLAGS += -I$(DIST_HOME)/include -I$(SUBDIRS)/include ${SDK_INCLUDES}

else
ifeq ($(SDK_PLATFORM),loongson)
EXTRA_CFLAGS += -I$(DIST_HOME)/include -I$(SUBDIRS)/include
else
CAVIUM_INCLUDES := -I ${SUBDIRS}/config -I ${OCTEON_ROOT}/gpl-executive

EXTRA_CFLAGS += -I$(DIST_HOME)/include -I$(SUBDIRS)/include ${CAVIUM_INCLUDES}
endif
endif
endif

ifndef SUBDIRS
SUBDIRS         =$(shell pwd)
endif
export SUBDIRS

CURRENT         = kap
USE_GNUEABI     = 1

TARGET = kapDrv

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m := ${TARGET}.o
${TARGET}-objs := kap.o

else

default:
	$(MAKE) V=1 -C $(DIST_HOME) M=$(shell pwd) modules
	@ cp $(TARGET).ko $(KMOD_EXPORT_DIR)/
	@ rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order
	@ echo "make kapDrv.ko completed."
endif

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
