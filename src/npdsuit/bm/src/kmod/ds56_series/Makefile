
#ifneq ($(KDIR),)
ifneq ($(KERNELRELEASE),)
obj-m	:= bm.o 

bm-y := bmk_main.o bmk_read_eeprom.o bmk_write_eeprom.o bmk_operation_boot_env.o

bm-y += bmk_ds5600_series_info.o bmk_product_init.o 

EXTRA_CFLAGS += -I${ACCAPI_DIR}
EXTRA_CFLAGS += -I${ACCAPI_DIR}/board
EXTRA_CFLAGS += -I${PWD}
EXTRA_CFLAGS += -I${KERNEL_ROOT}
EXTRA_CFLAGS += ${PRODUCT_CFLAGS}
EXTRA_CFLAGS += ${BOARD_CFLAGS}

ASDK_TOP= ${NPDSUIT_MOD}/$(SDK_TYPE)_$(SDK_VERSION)
ASDK_OSAL_PATH = $(ASDK_TOP)/osal
DRV_SWITCH_PATH = $(ASDK_TOP)/driver/switch
SYS_APC_INC_PATH = $(ASDK_TOP)/system/apc/linux/um/include
ifndef KDIR
KDIR = $(KERNEL_ROOT)
endif
ifndef PRJ
PRJ = $(ASDK_TOP)
endif


MODULE_INC += -I$(ASDK_OSAL_PATH)/include \
                -I$(ASDK_OSAL_PATH)/include/linux/kernel \
		-I$(DRV_SWITCH_PATH)/include \
		-I$(SYS_SWITCH_PATH)/common/include \
                -I$(SYS_APC_INC_PATH)
EXTRA_CFLAGS += $(MODULE_INC)  \
	-I$(KDIR)/arch/mips/include \
	-I$(KDIR)/arch/mips/include/asm/mach-generic/ \
	-I$(PRJ)/system/cpusub/music/linux/2.6.31/kernel/arch/inc/

else


MODULE    := bm
KVERS     := $(shell uname -r)
KDIR      := ${KERNEL_ROOT}
INCLUDE   := -isystem $(KDIR)/include  
MODCFLAGS := -DMODULE -D__KERNEL__  -Wall ${INCLUDE} 
PWD       := $(shell pwd)

all: 
	$(MAKE) V=1 -C ${KDIR} M=`pwd` modules
	mv *.ko $(KMOD_EXPORT_DIR)

clean:
	rm -f *.o
	rm -f *.mod.o
	rm -f *.mod.c
	rm -f *.*~
	rm -rf .tmp_versions
	rm -rf .bm*
	rm -rf Module.symvers

endif
