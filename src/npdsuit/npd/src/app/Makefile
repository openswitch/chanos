# Network Platform Daemon 
#

APP_TARGET_NAME = npd

CC  = $(COMPILER_PREFIX)gcc

CFLAGS = -Wall -D_GNU_SOURCE $(INCLUDE_AUTEWARE_MODULES)
LDFLAGS = -L$(LIB_EXPORT_DIR) -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -lzcommon -ltipc_api -lnpdlib 
CFLAGS += -g

LDLIBS = -lnam -lnbm -lpthread -lpdt_init
# add for dhcp snooping db

# ASIC driver type Broadcom
ifeq ($(DRV_LIB_FLAG),BCM)
CFLAGS += -DBCM_ESW_SUPPORT #-ansi
LDLIBS += -lmbcm
endif

# ASIC driver type Marvell
ifeq ($(DRV_LIB_FLAG),CPSS)
CFLAGS += -ansi
LDLIBS += -lmcpss
endif

ifeq ($(DRV_LIB_FLAG),CPSS_XCAT)
CFLAGS += -DCPU_ARM_XCAT
LDLIBS += -lmcpss -lrt
endif

ifeq ($(DRV_LIB_FLAG),CPSS_LION)
CFLAGS += -D__EXCEPTIONS -D__USE_GNU
LDLIBS += -lmcpss -lrt
endif

ifeq ($(DRV_LIB_FLAG),CPSS_LION2)
CFLAGS += -D__EXCEPTIONS -D__USE_GNU -DCPU_ARM_XCAT
LDLIBS += -lmcpss -lrt
endif
#ASIC driver type centec
ifeq ($(DRV_LIB_FLAG),CENTEC)
LDLIBS += -lsdkkal -lrt -lctclib -lsal -lm -ldrvs -lsdkcore -lsdkdrv -lctccli -lmctc -lvsc -lsdkcli
endif

#TOPSEC driver type
ifeq ($(DRV_LIB_FLAG),TOPSEC)
LDLIBS += -ltos_npd -ltos_chassis -lcrypt
endif

ifeq ($(DRV_LIB_FLAG),ATHEROS)
CFLAGS += -fPIC -DFAL_LED
LDLIBS += -lssdk 
endif

#ASX6600 
ifeq ($(DRV_LIB_FLAG),ASX)
CFLAGS += -DBCM_ESW_SUPPORT #-ansi
endif

ifneq ($(findstring DBUS_RELAY,$(INCLUDE_AUTEWARE_MODULES)),)
LDFLAGS += -ldbus_relay -ldbus-convenience
endif
#CFLAGS += -DDBUS_API_SUBJECT_TO_CHANGE

INCLUDES += -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/npd -I$(ACCAPI_DIR)/lib

# add for dhcp snooping db
INCLUDES += -I$(USER_ADDITION_LIB_ROOT_PATH)/include

NPD_COMMON_OBJS := npd_main.o npd_dbus.o npd_log.o npd_vlan.o npd_trunk.o npd_eth_port.o npd_engine.o npd_rstp_common.o npd_classmap.o \
			npd_acl.o npd_qos.o npd_vlan.o npd_trunk.o npd_intf.o npd_hash.o npd_fdb.o npd_arpsnooping.o npd_route.o \
			npd_igmp_snp_com.o npd_pvlan.o npd_mirror.o npd_diag.o npd_tunnel.o npd_dldp_common.o npd_vrrp.o npd_smart_link.o \
			npd_dhcp_snp_com.o npd_dhcp_snp_options.o npd_dhcp_snp_pkt.o npd_dhcp_snp_tbl.o  npd_ip_source_guard.o\
			product_conn.o npd_c_slot.o npd_product.o chassis_manage_proto.o \
			circle.o os_unix.o npd_asd.o npd_database.o npd_netif_event.o npd_switch_port.o npd_lacp.o npd_dhcp_relay.o \
                        npd_rx.o npd_tx.o npd_udld.o \
                        npd_pkt_list.o npd_tracking.o npd_poe.o npd_erpp.o npd_sflow.o
ifneq ($(findstring GVRP,$(INCLUDE_AUTEWARE_MODULES)),)
NPD_COMMON_OBJS += npd_gvrp.o npd_gvrp_gid.o npd_gvrp_gidtt.o npd_gvrp_timer.o npd_gvrp_gip.o 
endif
ifneq ($(findstring NPD_IPV6,$(INCLUDE_AUTEWARE_MODULES)),)
NPD_COMMON_OBJS += npd_ndisc_snooping.o npd_dhcpv6_relay.o
endif
ifneq ($(findstring CAPWAP_ENGINE,$(INCLUDE_AUTEWARE_MODULES)),)
NPD_COMMON_OBJS += npd_capwap.o
endif

NPD_CHASM_SPEC_DIR = "nh_series"
ifeq ($(PRODUCT_SERIES), as3000)
NPD_CHASM_SPEC_DIR = "as2k_series"
endif
ifeq ($(PRODUCT_SERIES), us3000)
NPD_CHASM_SPEC_DIR := "us_series" 
endif

ifeq ($(PRODUCT_SERIES), ds5600)
NPD_CHASM_SPEC_DIR := "ds_series" 
endif

NPD_OBJS := $(NPD_COMMON_OBJS)

BUILD_TARGET_OBJS += $(foreach module,$(NPD_OBJS), $(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/$(module))

NPD_DEPS_FILE = npd.dep


DC_LIST := $(patsubst %.o,%.dc,$(NPD_COMMON_OBJS))

.PHONY: all
all: $(NPD_DEPS_FILE) chasm npd 

-include $(NPD_DEPS_FILE)

RMDEP:
	rm -f npd.dep

	
dep: RMDEP $(DC_LIST)
	@echo building file dependency done...
	
$(NPD_DEPS_FILE): dep

chasmdep:
	$(MAKE) -C $(NPD_CHASM_SPEC_DIR) dep

chasm: chasmdep
	$(MAKE) -C $(NPD_CHASM_SPEC_DIR)

npd: $(BUILD_TARGET_OBJS)  
	$(CC) $(LDFLAGS) $(LDLIBS) $(DBUS_LIBS) -o $@ $^ 
	mv npd $(BIN_EXPORT_DIR)/

$(PROJECT_BUILD_DIR)/$(APP_TARGET_NAME)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -o $@ $<

%.dc: %.c
	$(CC) $(CFLAGS) $(DBUS_INCLUDE) $(INCLUDES) -MM $< >> $(NPD_DEPS_FILE)	
	

.PHONY: clean
clean:
	rm -rf *~ $(NPD_OBJS) $(NPD_DEPS_FILE) npd 
	$(MAKE) -C $(NPD_CHASM_SPEC_DIR) clean
