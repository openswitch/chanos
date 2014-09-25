ifndef CC
CC = $(COMPILER_PREFIX)gcc
endif
ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall 
endif
ifeq ($(COMPILER_PREFIX),mips-openwrt-linux-)
CFLAGS += -fPIC 
endif

QUAGGA_DIR=$(PROJECT_TOP_SRC)/quagga/debsrc/quagga-0.99.16-GR

INCLUDES = -I$(QUAGGA_DIR) -I$(QUAGGA_DIR)/${BUILDDIR} -I$(QUAGGA_DIR)/lib -I$(QUAGGA_DIR)/debsrc/quagga-0.99.16-GR -I$(QUAGGA_DIR)/debsrc/quagga-0.99.16-GR/lib 


INCLUDES += -I./ -I$(ACCAPI_DIR) -I$(ACCAPI_DIR)/lib -I$(ACCAPI_DIR)/dbus/npd -I$(MANAPI_DIR)
INCLUDES += -I$(USER_ADDITION_LIB_ROOT_PATH)/include/dbus-1.0 -I$(USER_ADDITION_LIB_ROOT_PATH)/include
CFLAGS += $(INCLUDES) $(INCLUDE_CHANOS_MODULES)
CFLAGS += -g -DHAVE_CONFIG_H
LD_PATH = -L$(BUILD_ROOTFS_DIR)/opt/lib -L$(USER_ADDITION_LIB_ROOT_PATH)/lib -L$(BUILD_BUILDROOT_DIR)/export/opt/lib
LDFLAGS = -g -L$(USER_ADDITION_LIB_ROOT_PATH)/lib #-lpthread -ltipc_api #-lnpdlib #-lzebra
LDFLAGS += $(LD_PATH) -ldbus-1 #-lxml2

ALL = man
all: $(ALL)
OBJS = man_db.o man_dbus.o man_product.o man_sysconf.o man_dhcp.o man_dhcpv6.o man_dhclient.o man_dhcp_relay.o man_dhcp_snp.o man_eth_port.o man_intf.o man_lldp.o man_lacp.o \
       man_qos.o man_syslog.o man_trunk.o man_user.o man_vlan.o man_acl.o man_str_parse.o man_security.o man_mirror.o man_portal.o \
       man_fdb.o man_igmpsnp.o man_mldsnp.o man_radius.o man_vrrp.o man_udld.o man_snmpd_engine.o man_pim.o man_dvmrp.o man_stp.o man_tracking.o man_smart_link.o man_poe.o man_dhcpv6_relay.o man_erpp.o man_tunnel.o man_sflow.o
ifneq ($(findstring AAA,$(INCLUDE_CHANOS_MODULES)),)
OBJS += man_sta.o
endif
ifneq ($(findstring WCPSS,$(INCLUDE_CHANOS_MODULES)),)
OBJS += man_wtp.o man_wlan.o man_ac.o man_radio.o man_license.o man_ac_ip_list.o man_ac_roaming.o man_ebr.o man_led.o man_wqos.o man_wsm.o  
endif

MANLIB_OBJS += $(foreach module,$(OBJS), $(PROJECT_BUILD_DIR)/manlib/$(module))

man: $(MANLIB_OBJS)
	$(CC) -shared $(LDFLAGS) $^ -o $(PROJECT_BUILD_DIR)/manlib/libman.so
	cp $(PROJECT_BUILD_DIR)/manlib/libman.so $(BUILD_ROOTFS_DIR)/opt/lib
	cp $(PROJECT_BUILD_DIR)/manlib/libman.so $(USER_ADDITION_LIB_ROOT_PATH)/lib
install: all
	cp $(PROJECT_BUILD_DIR)/manlib/libman.so $(BUILD_ROOTFS_DIR)/opt/lib
$(PROJECT_BUILD_DIR)/manlib/%.o: %.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(CFLAGS) -o $@ $<
clean:
	@rm -f *~ *.so *.o config_test $(PROJECT_BUILD_DIR)/manlib/*.d $(PROJECT_BUILD_DIR)/manlib/*.o $(PROJECT_BUILD_DIR)/manlib/*.so*

