# 
# 
# This file intends for make software system 

export MAKE=make

export PROJECT_TOP_DIR=${shell pwd}

ifeq ($(PRODUCT_SERIES),)
export PRODUCT_SERIES=as9600
endif

ifeq ($(TARGET_NAME),)
export TARGET_NAME=$(PRODUCT_SERIES)
endif

export PROJECT_BUILD_DIR=$(PROJECT_TOP_DIR)/build/$(TARGET_NAME)
$(shell \
if [ ! -d $(PROJECT_TOP_DIR)/build ]; then \
	mkdir $(PROJECT_TOP_DIR)/build; \
        fi \
)
$(shell \
if [ ! -d $(PROJECT_BUILD_DIR) ]; then \
	mkdir $(PROJECT_BUILD_DIR); \
        fi \
)
export BUILD_TARGET_DIR=$(PROJECT_TOP_DIR)/../$(TARGET_NAME)
export BUILD_SYSROOT_DIR=$(BUILD_TARGET_DIR)/sysroot
export BUILD_ROOTFS_DIR=$(BUILD_TARGET_DIR)/rootfs/files
export BUILD_BUILDROOT_DIR=$(BUILD_TARGET_DIR)/buildroot

export USER_ADDITION_LIB_ROOT_PATH=$(BUILD_SYSROOT_DIR)
export PROJECT_TOP_SRC=$(PROJECT_TOP_DIR)/src
include $(PROJECT_TOP_DIR)/make/make.common

export EXPORT_OPT_DIR=${BUILD_BUILDROOT_DIR}/export/opt
export LIB_EXPORT_DIR=${BUILD_BUILDROOT_DIR}/export/opt/lib
export BIN_EXPORT_DIR=${BUILD_BUILDROOT_DIR}/export/opt/bin
export WWW_EXPORT_DIR=${BUILD_BUILDROOT_DIR}/export/opt/www

export ROOTFS_KMOD_DIR=${BUILD_ROOTFS_DIR}/lib/modules/$(LINUX_KERNEL_MODULE_DIR)

export KMOD_EXPORT_DIR=${BUILD_BUILDROOT_DIR}/export/kmod

export MOTDFILE=${BUILD_ROOTFS_DIR}/etc/motd
export BUILDERFILE=${BUILD_ROOTFS_DIR}/etc/version/builder
export VERFILE=${BUILD_ROOTFS_DIR}/etc/version/version
export NAMFILE=${BUILD_ROOTFS_DIR}/etc/version/name
export PRODUCTFILE=${BUILD_ROOTFS_DIR}/etc/version/products
export BUILDNOFILE=${BUILD_ROOTFS_DIR}/etc/version/buildno_v1.3
export AWNAME=$(shell echo "AW`cat ${VERFILE}`.`cat ${BUILDNOFILE}`.`cat ${PRODUCTFILE}`")
export IMGDIR=imgdir

export DBUS_INCLUDE=-I$(USER_ADDITION_LIB_ROOT_PATH)/include/dbus-1.0 -I$(USER_ADDITION_LIB_ROOT_PATH)/lib/dbus-1.0/include
export DBUS_LIBS=-ldbus-1

$(shell \
	if [ -d .git ] ; then \
		cat ${VERFILE} > ${IMGDIR} ; \
	elif [ ! -e ${IMGDIR} ] ; then \
		echo "`date +%Y%m%d`" > ${IMGDIR};  \
	elif [ `cat ${IMGDIR}` != `cat ${VERFILE}` ]; then \
		echo "`date +%Y%m%d`" > ${IMGDIR}; \
	fi )
export TARGETDIR=$(shell echo "/var/lib/tftpboot/`cat ${IMGDIR}`")

$(info =========================================================================)
$(info ROOTFS_DIR = ${BUILD_ROOTFS_DIR})
$(info PATH is ${PATH} )
$(info CFLAGS=${CFLAGS} )
$(info TARGETDIR is ${TARGETDIR})
$(info =========================================================================)

export ACCAPI_DIR=${PROJECT_TOP_SRC}/include
export MANAPI_DIR=${ACCAPI_DIR}/man
export BASHTOOLS_DIR=${PROJECT_TOP_SRC}/bashtools
export NPDSUIT_MOD=${PROJECT_TOP_SRC}/npdsuit
export QUAGGA_MOD=${PROJECT_TOP_SRC}/quagga
export DCLI_MOD=${PROJECT_TOP_SRC}/man/cli
export OCTETH_KMOD=${PROJECT_TOP_SRC}/cavium-ethernet
export WCPSS_MOD=${PROJECT_TOP_SRC}/wcpss
export WTPVERFILE=${WCPSS_MOD}/src/res/wtpcompatible*
export ASD_MOD=${PROJECT_TOP_SRC}/asd
export STPSUIT_MOD=${PROJECT_TOP_SRC}/stpsuit
export ERPP_MOD=${PROJECT_TOP_SRC}/erpp
export HAD_MOD=${PROJECT_TOP_SRC}/had
export SMART_LINK_MOD=${PROJECT_TOP_SRC}/smart-link
export HBIP_MOD=${PROJECT_TOP_SRC}/hbip
export IGMP_MOD=${PROJECT_TOP_SRC}/igmp-snooping
export MLD_MOD=${PROJECT_TOP_SRC}/mld-snooping
export DLDP_MOD=${PROJECT_TOP_SRC}/dldp
export UDLD_MOD=${PROJECT_TOP_SRC}/udld
export SFLOW_MOD=${PROJECT_TOP_SRC}/sflow
export CCGI_MOD=${PROJECT_TOP_SRC}/man/webui
export SRVM_MOD=${PROJECT_TOP_SRC}/service_management
export IPTABLES_MOD=${PROJECT_TOP_SRC}/iptables
export EBTABLES_MOD=${PROJECT_TOP_SRC}/ebtables
export CAPTIVE_MOD=${PROJECT_TOP_SRC}/captive_portal
export EAG_MOD=${PROJECT_TOP_SRC}/eag
export SDK_MOD=${NPDSUIT_MOD}/$(SDK_TYPE)_$(SDK_VERSION)
export SDK_TOPDIR=${PROJECT_TOP_SRC}/npdsuit/$(SDK_TYPE)_$(SDK_VERSION)
export SNMP_ROOTDIR=${PROJECT_TOP_SRC}/net-snmp-5.7.1
export SNMPMIBS_DIR=${PROJECT_TOP_SRC}/net-snmp-5.7.1/mibs
export SNMP_MOD=${PROJECT_TOP_SRC}/net-snmp-5.7.1
export TRAP_HELPER_MOD=${PROJECT_TOP_SRC}/net-snmp/trap-helper
export SUBAGENT_MOD=${PROJECT_TOP_SRC}/net-snmp/subagent
export DHCP_MOD=${PROJECT_TOP_SRC}/dhcp-4.1.1
export DCCN_MOD=${PROJECT_TOP_SRC}/dccnetlink
export DCCN_TOPSRC=${PROJECT_TOP_SRC}/dccnetlink
export PIMD_MOD=${PROJECT_TOP_SRC}/pimd
export DVMRP_MOD=${PROJECT_TOP_SRC}/dvmrp
export TIPC_API_MOD=${PROJECT_TOP_SRC}/tipc_api
export DBUS_RELAY_MOD=${PROJECT_TOP_SRC}/dbus_relay
export PSTACK_MOD=${PROJECT_TOP_SRC}/pstack
export LIBNPDLIB_MOD=$(NPDSUIT_MOD)/lib
export LLDP_APP_MOD=${PROJECT_TOP_SRC}/lldp/src
export FILE_SPLIT_MOD=${PROJECT_TOP_SRC}/man/file_split
export CRYPT_3DES_MOD=${PROJECT_TOP_SRC}/man/decrypt
export PAM_RADIUS_MOD=${PROJECT_TOP_SRC}/pam_radius
export PAM_TACPLUS_MOD=${PROJECT_TOP_SRC}/pam_tacplus
export MAN_MOD=${PROJECT_TOP_SRC}/man/lib
export UNPACK_MOD=${PROJECT_TOP_SRC}/man/unpack
export PAM_CHECK_PASSWD_MOD=${PROJECT_TOP_SRC}/checkpassword-pam-0.99
export COMMAND_DB_SYNC_MOD=${PROJECT_TOP_SRC}/man/syncd
export RADVD_MOD=${PROJECT_TOP_SRC}/radvd

.SILENT: default
.PHONY: default
default:
	echo "==========================================================="
	echo "Following targets are supported:"
	echo
	echo "==CVS Source code management targets"
	echo 
	echo "updatesrc		-Update cvs source code of apps"
	echo "update			-Update cvs source code of apps and buildtools"
	echo "archives		-Compress all source code and build an archive" 
	echo
	echo "==Application&related kernel modules targets"
	echo 
	echo "pubapps			-Compile all userspace applications and related kernel modules."
	echo "dcli			-Make dcli module."
	echo "npdsuit bcm or mv		-Make npd suit."
	echo "wcpss			-Make wcpss."
	echo "asd			-Make asd."
	echo "cavium-ethernet		-Make cavium-ethernet."
	echo "stpsuit			-Make rstp and mstp."
	echo "had			-Make HA daemon."
	echo "smartlink			-Make Smart-Link daemon."
	echo "tipc_api			-Make tipc api."
	echo "libnpd                    -Make bit operation and database."
	echo "dbus_relay		-Make dbus relay."
	echo "igmp                      -Make IGMP Snooping."
	echo
	echo "==Cleanning targets"
	echo 
	echo "cleanapps		-Clean apps."
	echo "cleandcli		-Clean dcli."
	echo "cleannpdsuit		-Clean npdsuit."
	echo "cleanwcpss		-Clean wcpss."
	echo "cleanasd		-Clean asd."
	echo "cleancavium-ethernet	-Clean Cavium-ethernet."
	echo "cleanstpsuit		-Clean stpsuit."
	echo "cleanhad			-Clean HA daemon."
	echo "cleansmartlink		-Clean Smart-Link."
	echo "cleanccgi		-Clean ccgi."
	echo "cleanbcm_npdsuit 		-Clean npdsuit bcm utilities."
	echo "cleantipc_api 		-Clean tipc api."
	echo "cleandbus_relay 		-Clean dbus relay."
	echo "cleanigmp                 -Clean IGMP Snooping."
	echo "cleanpstack -Clean pstack."
	echo 
	echo "==System level targets"
	echo 
	echo "For other possible targets, please try: make TABKEY"
	echo "==========================================================="

versioncheck:
	@echo "Checking which gcc in path is in use..."
	@which mips64-octeon-linux-gnu-gcc
	@echo "Checking which gcc version is in use..."
	@mips64-octeon-linux-gnu-gcc -v

define checkquagga
	if [ -d ${QUAGGA_MOD} ]; \
	then \
		echo "Use private quagga header files." ;\
		export QUAGGA_DIR=${QUAGGA_MOD} ;\
	elif [ -n "${QUAGGA_DIR}" ] ; \
	then \
		echo "No private quagga found, using public quagga header files env";  \
		echo "Public QUAGGA_DIR is ${QUAGGA_DIR}"; \
	else \
		echo "No local or public quagga found. exit..."; \
		exit 1; \
	fi
endef

tipc_api:
	@echo "Building tipc api module..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${TIPC_API_MOD}
	rm -rf ${BUILD_ROOTFS_DIR}/usr/lib/libtipc_api.so
	cp $(USER_ADDITION_LIB_ROOT_PATH)/lib/libtipc_api.so ${BUILD_ROOTFS_DIR}/usr/lib
	chmod 777 ${BUILD_ROOTFS_DIR}/usr/lib/libtipc_api.so
	
lldp:
	@echo "Building lldp module..."
	-rm -rf ${BUILD_ROOTFS_DIR}/usr/bin/lldpd
ifeq ($(findstring -DHAVE_LLDP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_LLDP)
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${LLDP_APP_MOD} install
	cp $(USER_ADDITION_LIB_ROOT_PATH)/bin/lldpd ${BUILD_ROOTFS_DIR}/usr/bin
endif	

file_split:
	@echo "Building file-split ..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${FILE_SPLIT_MOD} install

cleanfile_split:
	@echo "Cleaning file-split ..."
	$(MAKE) -C ${FILE_SPLIT_MOD} clean

decrypt:
	@echo "Building decrypt ..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${CRYPT_3DES_MOD} install

cleandecrypt:
	@echo "Cleaning decrypt ..."
	$(MAKE) -C ${CRYPT_3DES_MOD} clean

pam_radius:
	@echo "Building pam_radius module..."
	-rm -rf ${BUILD_ROOTFS_DIR}/lib/security/pam_radius_auth.so
ifeq ($(findstring -DHAVE_PAM,$(INCLUDE_CHANOS_MODULES)), -DHAVE_PAM)
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${PAM_RADIUS_MOD} install
	cp $(USER_ADDITION_LIB_ROOT_PATH)/lib/pam_radius_auth.so ${BUILD_ROOTFS_DIR}/lib/security
endif
	
pam_tacplus:
	@echo "Building pam_tacplus module..."
	-rm -rf ${BUILD_ROOTFS_DIR}/lib/security/pam_tacplus.so
ifeq ($(findstring -DHAVE_PAM,$(INCLUDE_CHANOS_MODULES)), -DHAVE_PAM)
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${PAM_TACPLUS_MOD} install
	cp $(USER_ADDITION_LIB_ROOT_PATH)/lib/pam_tacplus.so ${BUILD_ROOTFS_DIR}/lib/security
endif
	
manlib:
	@echo "Building UI lib..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${MAN_MOD} install
	
confsyncd:
	@echo "Building command_db_sync module..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${COMMAND_DB_SYNC_MOD} install
	
dbus_relay:
	@echo "Building dbus relay module..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${DBUS_RELAY_MOD}
	cp $(USER_ADDITION_LIB_ROOT_PATH)/lib/libdbus_relay.so ${BUILD_ROOTFS_DIR}/usr/lib
	cp $(USER_ADDITION_LIB_ROOT_PATH)/bin/dbus_relay ${BUILD_ROOTFS_DIR}/usr/bin
	
unpack:
	@echo "Building unpack image tool ..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${UNPACK_MOD}
	
cleanunpack:
	@echo "Cleaning unpack image tool ..."
	$(MAKE) -C ${UNPACK_MOD} clean
	
cleantipc_api:
	@echo "Cleaning tipc api module..."
	$(MAKE) -C ${TIPC_API_MOD} clean
	
cleanlldp:
	@echo "Cleaning lldp module..."
	$(MAKE) -C ${LLDP_APP_MOD} clean
	
cleanpam_radius:
	@echo "Cleaning pam_radius module..."
	$(MAKE) -C ${PAM_RADIUS_MOD} clean
	
cleanpam_tacplus:
	@echo "Cleaning pam_tacplus module..."
	$(MAKE) -C ${PAM_TACPLUS_MOD} clean
	
cleanmanlib:
	@echo "Cleaning UI lib..."
	$(MAKE) -C ${MAN_MOD} clean
	
cleanconfsyncd:
	@echo "Cleaning command_db_sync module..."
	$(MAKE) -C ${COMMAND_DB_SYNC_MOD} clean
	
cleandbus_relay:
	@echo "Cleaning dbus relay module..."
	$(MAKE) -C ${DBUS_RELAY_MOD} clean
	
libnpdlib:
	@echo "Building libnpdlib module..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${LIBNPDLIB_MOD}/src
	cp $(USER_ADDITION_LIB_ROOT_PATH)/lib/libnpdlib.so ${BUILD_ROOTFS_DIR}/opt/lib

cleannpdlib:
	@echo "Cleaning libnpdlib module..."
	$(MAKE) -C ${LIBNPDLIB_MOD}/src clean


dcli: manlib
	@echo "Building dcli module..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	@$(checkquagga) && $(MAKE) -C ${DCLI_MOD}/src/lib
	cp -d ${LIB_EXPORT_DIR}/libdcli* ${BUILD_ROOTFS_DIR}/opt/lib/

ccgi:
	@echo "Building ccgi ..."
	-rm -rf ${WWW_EXPORT_DIR}/ccgi-bin/*.cgi
	-rm -rf ${BUILD_ROOTFS_DIR}/opt/www/ccgi-bin/*.cgi	
ifeq ($(findstring -DHAVE_WEBMNG,$(INCLUDE_CHANOS_MODULES)), -DHAVE_WEBMNG)	
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${CCGI_MOD}/cgic205 OLDEAG=$(OLDEAG)
	cp ${CCGI_MOD}/cgic205/*.cgi ${WWW_EXPORT_DIR}/ccgi-bin
	cp ${CCGI_MOD}/cgic205/*.cgi ${BUILD_ROOTFS_DIR}/opt/www/ccgi-bin
	chmod 777 ${BUILD_ROOTFS_DIR}/opt/www/ccgi-bin/*.cgi
	cp -d ${CCGI_MOD}/cgic205/lib* ${LIB_EXPORT_DIR}
	cp -d ${CCGI_MOD}/cgic205/lib* ${BUILD_ROOTFS_DIR}/opt/lib/
endif

srvm:
	@echo "Building service management ..."
	$(MAKE) -C ${SRVM_MOD}/app
	cp ${SRVM_MOD}/app/srvload ${BIN_EXPORT_DIR}
	cp ${SRVM_MOD}/app/srvsave ${BIN_EXPORT_DIR}
	cp ${SRVM_MOD}/app/srvcmd ${BIN_EXPORT_DIR}

	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ] ; then \
		cd ${QUAGGA_MOD} ; \
		./configpkg ${TARGET_NAME}; \
		cd - ; \
	fi 	

	
snmpd:
	@echo "Building net-snmp ..."
	@echo $(SNMP_CONFIG_HOSTCC)
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/snmpd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/snmptrap
	rm -rf ${BUILD_ROOTFS_DIR}/usr/lib/libnetsnmpagent.so* 
	rm -rf $(USER_ADDITION_LIB_ROOT_PATH)/lib/libnetsnmpagent.so*
	rm -rf ${BUILD_ROOTFS_DIR}/opt/lib/libnetsnmpmibs.so*
	rm -rf $(USER_ADDITION_LIB_ROOT_PATH)/lib/libnetsnmpmibs.so*
	rm -rf ${BUILD_ROOTFS_DIR}/usr/lib/libnetsnmphelpers.so*
	rm -rf $(USER_ADDITION_LIB_ROOT_PATH)/lib/libnetsnmphelpers.so*
	rm -rf ${BUILD_ROOTFS_DIR}/usr/lib/libnetsnmptrapd.so*
	rm -rf $(USER_ADDITION_LIB_ROOT_PATH)/lib/libnetsnmptrapd.so*
	rm -rf ${BUILD_ROOTFS_DIR}/usr/lib/libnetsnmp.so* 
	rm -rf $(USER_ADDITION_LIB_ROOT_PATH)/lib/libnetsnmp.so*
ifeq ($(findstring -DHAVE_MAN_SNMP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_MAN_SNMP)	
	chmod 777 ${SNMP_MOD} -R
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ] ; then \
		cd ${SNMP_MOD} ; \
		./configpkg ${TARGET_NAME}; \
		cd - ; \
	fi
	$(MAKE) -C ${PROJECT_BUILD_DIR}/$@
	cp ${PROJECT_BUILD_DIR}/$@/agent/.libs/snmpd ${BUILD_ROOTFS_DIR}/opt/bin
	cp ${PROJECT_BUILD_DIR}/$@/apps/.libs/snmptrap ${BUILD_ROOTFS_DIR}/opt/bin
	cp -d ${PROJECT_BUILD_DIR}/$@/agent/.libs/libnetsnmpagent.so* ${BUILD_ROOTFS_DIR}/usr/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/agent/.libs/libnetsnmpagent.so* $(USER_ADDITION_LIB_ROOT_PATH)/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/agent/.libs/libnetsnmpmibs.so* ${BUILD_ROOTFS_DIR}/opt/lib/
	cp -d ${PROJECT_BUILD_DIR}/$@/agent/.libs/libnetsnmpmibs.so* $(USER_ADDITION_LIB_ROOT_PATH)/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/agent/helpers/.libs/libnetsnmphelpers.so* ${BUILD_ROOTFS_DIR}/usr/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/agent/helpers/.libs/libnetsnmphelpers.so* $(USER_ADDITION_LIB_ROOT_PATH)/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/apps/.libs/libnetsnmptrapd.so* ${BUILD_ROOTFS_DIR}/usr/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/apps/.libs/libnetsnmptrapd.so* $(USER_ADDITION_LIB_ROOT_PATH)/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/snmplib/.libs/libnetsnmp.so* ${BUILD_ROOTFS_DIR}/usr/lib
	cp -d ${PROJECT_BUILD_DIR}/$@/snmplib/.libs/libnetsnmp.so* $(USER_ADDITION_LIB_ROOT_PATH)/lib
	chmod +x ${BUILD_ROOTFS_DIR}/opt/bin/snmptrap
	chmod +x ${BUILD_ROOTFS_DIR}/opt/bin/snmpd
endif
	
cleansnmpd:
	rm -rf ${PROJECT_BUILD_DIR}/snmpd
  
snmp: snmpd trap-helper
	@echo "Building snmp extentions ..."
	rm -rf ${LIB_EXPORT_DIR}/subagent_plugin.so
	rm -rf ${BIN_EXPORT_DIR}/subagent
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/subagent
ifeq ($(findstring -DHAVE_MAN_SNMP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_MAN_SNMP)	
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi	
	$(MAKE) -C ${SUBAGENT_MOD} subagent_plugin.so
	cp ${SUBAGENT_MOD}/subagent_plugin.so ${LIB_EXPORT_DIR}
	#cp ${SNMP_ROOTDIR}/mibs/* ${BUILD_ROOTFS_DIR}/opt/share/snmp/mibs/
        #cp -rf ${SNMPMIBS_DIR}/* ${BUILD_ROOTFS_DIR}/opt/share/snmp/mibs/
	$(MAKE) -C ${SUBAGENT_MOD} 
	cp ${SUBAGENT_MOD}/subagent ${BIN_EXPORT_DIR}
	cp ${SUBAGENT_MOD}/subagent ${BUILD_ROOTFS_DIR}/opt/bin
endif

trap-helper:
	@echo "Building trap-helper..."
	rm -rf $(BIN_EXPORT_DIR)/trap-helper
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/trap-helper
ifeq ($(findstring -DHAVE_MAN_SNMP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_MAN_SNMP)		
	$(MAKE) -C ${TRAP_HELPER_MOD}
	cp ${TRAP_HELPER_MOD}/trap-helper $(BIN_EXPORT_DIR)
	cp ${TRAP_HELPER_MOD}/trap-helper ${BUILD_ROOTFS_DIR}/opt/bin
endif

asd:
	@echo "Building asd..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/asd
ifeq ($(findstring -DHAVE_AAA,$(INCLUDE_CHANOS_MODULES)), -DHAVE_AAA)		
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${ASD_MOD}/src/app
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BUILD_ROOTFS_DIR}/opt/bin/
	#cp -P ${ASD_MOD}/src/app/wapi/libw* ${BUILD_ROOTFS_DIR}/opt/lib/
	#chmod 755 ${BUILD_ROOTFS_DIR}/opt/lib/libwssl.so*
	#ln -sf libwcrypto.so.0.9.8 ${BUILD_ROOTFS_DIR}/opt/lib/libwcrypto.so
	#ln -sf libwssl.so.0.9.8 ${BUILD_ROOTFS_DIR}/opt/lib/libwssl.so
endif

wcpss: wid wsm
	echo "Finished making wcpss."
	rm -rf ${BUILD_ROOTFS_DIR}/etc/version/wtpcompatible*
ifeq ($(findstring -DHAVE_WCPSS,$(INCLUDE_CHANOS_MODULES)), -DHAVE_WCPSS)	
	cp ${WTPVERFILE} ${BUILD_ROOTFS_DIR}/etc/version/
endif

wsm: wcpsspub_ac
	@echo "Building wsm ..."
	rm -rf $(BIN_EXPORT_DIR)/wsm
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/wsm
ifeq ($(findstring -DHAVE_WCPSS,$(INCLUDE_CHANOS_MODULES)), -DHAVE_WCPSS)	
	$(MAKE) -C ${WCPSS_MOD}/src/app/wsm
	cp ${WCPSS_MOD}/src/app/wsm/wsm $(BIN_EXPORT_DIR)
	cp ${WCPSS_MOD}/src/app/wsm/wsm ${BUILD_ROOTFS_DIR}/opt/bin/
endif

wid: wcpsspub_ac
	@echo "Building wid ..."
	rm -rf $(BIN_EXPORT_DIR)/wid
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/wid
ifeq ($(findstring -DHAVE_WCPSS,$(INCLUDE_CHANOS_MODULES)), -DHAVE_WCPSS)	
	$(MAKE) -C ${WCPSS_MOD}/src/app/wid
	cp ${WCPSS_MOD}/src/app/wid/wid $(BIN_EXPORT_DIR)
	cp ${WCPSS_MOD}/src/app/wid/wid ${BUILD_ROOTFS_DIR}/opt/bin/
endif

wtpd: wcpsspub_wtp
	@echo "Building wtpd ..."
	rm -rf $(BIN_EXPORT_DIR)/wtpd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/wtpd
ifeq ($(findstring -DHAVE_WCPSS,$(INCLUDE_CHANOS_MODULES)), -DHAVE_WCPSS)		
	$(MAKE) -C ${WCPSS_MOD}/src/app/wtpd
	cp ${WCPSS_MOD}/src/app/wtpd/wtpd $(BIN_EXPORT_DIR)
	cp ${WCPSS_MOD}/src/app/wtpd/wtpd ${BUILD_ROOTFS_DIR}/opt/bin/
endif

wcpsspub_ac:
	@echo "Building wcpss public lib for AC side..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/lib/*.a
ifeq ($(findstring -DHAVE_WCPSS,$(INCLUDE_CHANOS_MODULES)), -DHAVE_WCPSS)		
	$(MAKE) -C ${WCPSS_MOD}/src/app/pub
	cp ${WCPSS_MOD}/src/app/pub/*.a ${BUILD_ROOTFS_DIR}/opt/lib/
endif
	
wcpsspub_wtp:
	@echo "Building wcpss public lib for WTP side ..."
	$(MAKE) -C ${WCPSS_MOD}/src/app/pub -f Makefile_wtp
	
pimd:
	@echo "Building pimd suit ..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/pimd

ifeq ($(findstring -DHAVE_PIM,$(INCLUDE_CHANOS_MODULES)), -DHAVE_PIM)		
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${PIMD_MOD}
	cp ${PROJECT_BUILD_DIR}/$@/pimd ${BUILD_ROOTFS_DIR}/opt/bin/
	#cp ${PIMD_MOD}/res/pimd ${BUILD_ROOTFS_DIR}/etc/init.d/
	#chmod +x ${BUILD_ROOTFS_DIR}/etc/init.d/pimd
	#ln -sf ../init.d/pimd  ${BUILD_ROOTFS_DIR}/etc/rc2.d/S64pimd
endif

dvmrp:
	@echo "Building dvmrp suit ..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/dvmrp
ifeq ($(findstring -DHAVE_DVMRP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_DVMRP)		
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${DVMRP_MOD} 
	cp ${PROJECT_BUILD_DIR}/$@/dvmrp ${BUILD_ROOTFS_DIR}/opt/bin/
endif

quagga:
	@echo "Building quagga suit ..."
	chmod 777 ${QUAGGA_MOD} -R
ifeq ($(findstring -DHAVE_OSPF_GR,$(INCLUDE_CHANOS_MODULES)), -DHAVE_OSPF_GR)
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ] ; then \
		cd ${QUAGGA_MOD} ; \
		./configpkg_gr ${TARGET_NAME}; \
		cd - ; \
	fi 	
else 
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ] ; then \
		cd ${QUAGGA_MOD} ; \
		./configpkg ${TARGET_NAME}; \
		cd - ; \
	fi 	
endif 
	$(MAKE) -C ${PROJECT_BUILD_DIR}/$@
	rm -rf $(BIN_EXPORT_DIR)/zebra
	rm -rf $(BIN_EXPORT_DIR)/vtysh
	rm -rf $(BIN_EXPORT_DIR)/ripd
	rm -rf $(BIN_EXPORT_DIR)/ospfd
	rm -rf $(BIN_EXPORT_DIR)/ripngd
	rm -rf $(BIN_EXPORT_DIR)/ospf6d
	rm -rf $(BIN_EXPORT_DIR)/bgpd
	rm -rf $(BIN_EXPORT_DIR)/isisd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/zebra
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/vtysh
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/ripd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/ospfd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/ripngd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/ospf6d
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/bgpd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/isisd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/lib/libzebra.so*
	rm -rf ${BUILD_ROOTFS_DIR}/opt/lib/libzcommon.so*  
	rm -rf ${BUILD_ROOTFS_DIR}/opt/lib/libospf.so*     
  
ifeq ($(findstring -DHAVE_ZEBRA,$(INCLUDE_CHANOS_MODULES)), -DHAVE_ZEBRA)  
	if [ -e ${PROJECT_BUILD_DIR}/$@/zebra/.libs/zebra ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/zebra/.libs/zebra $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/zebra ${BUILD_ROOTFS_DIR}/opt/bin/ ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/zebra ; \
	fi
endif	
	if [ -e ${PROJECT_BUILD_DIR}/$@/vtysh/.libs/vtysh ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/vtysh/.libs/vtysh $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/vtysh ${BUILD_ROOTFS_DIR}/opt/bin/ ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/vtysh ; \
	fi
ifeq ($(findstring -DHAVE_RIP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_RIP)  	
	if [ -e ${PROJECT_BUILD_DIR}/$@/ripd/.libs/ripd ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/ripd/.libs/ripd $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/ripd ${BUILD_ROOTFS_DIR}/opt/bin/  ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/ripd ; \
	fi
endif
ifeq ($(findstring -DHAVE_OSPF,$(INCLUDE_CHANOS_MODULES)), -DHAVE_OSPF)	
	if [ -e ${PROJECT_BUILD_DIR}/$@/ospfd/.libs/ospfd ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/ospfd/.libs/ospfd $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/ospfd ${BUILD_ROOTFS_DIR}/opt/bin/  ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/ospfd ; \
	fi
endif
ifeq ($(findstring -DHAVE_OSPF6,$(INCLUDE_CHANOS_MODULES)), -DHAVE_OSPF6)		
	if [ -e ${PROJECT_BUILD_DIR}/$@/ospf6d/.libs/ospf6d ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/ospf6d/.libs/ospf6d $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/ospf6d ${BUILD_ROOTFS_DIR}/opt/bin/  ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/ospf6d ; \
	fi
endif
ifeq ($(findstring -DHAVE_BGP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_BGP)			
	if [ -e ${PROJECT_BUILD_DIR}/$@/bgpd/.libs/bgpd ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/bgpd/.libs/bgpd $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/bgpd ${BUILD_ROOTFS_DIR}/opt/bin/  ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/bgpd ; \
	fi
endif
ifeq ($(findstring -DHAVE_RIPNG,$(INCLUDE_CHANOS_MODULES)), -DHAVE_RIPNG)		
	if [ -e ${PROJECT_BUILD_DIR}/$@/ripngd/.libs/ripngd ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/ripngd/.libs/ripngd $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/ripngd ${BUILD_ROOTFS_DIR}/opt/bin/  ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/ripngd ; \
	fi
endif
ifeq ($(findstring -DHAVE_ISIS,$(INCLUDE_CHANOS_MODULES)), -DHAVE_ISIS)		
	if [ -e ${PROJECT_BUILD_DIR}/$@/isisd/.libs/isisd ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/isisd/.libs/isisd $(BIN_EXPORT_DIR) ; \
		cp -d ${BIN_EXPORT_DIR}/isisd ${BUILD_ROOTFS_DIR}/opt/bin/  ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/bin/isisd ; \
	fi
endif	
	if [ -e ${PROJECT_BUILD_DIR}/$@/lib/.libs/libzebra.so.0.0.0 ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/lib/.libs/libzebra.so.0.0.0 $(LIB_EXPORT_DIR) ; \
		ln -sf libzebra.so.0.0.0 $(LIB_EXPORT_DIR)/libzebra.so ; \
		ln -sf libzebra.so.0.0.0 $(LIB_EXPORT_DIR)/libzebra.so.0 ; \
		ln -sf libzebra.so.0.0.0 $(LIB_EXPORT_DIR)/libzebra.so.0.0 ; \
		cp -d $(LIB_EXPORT_DIR)/libzebra.so* ${BUILD_ROOTFS_DIR}/opt/lib/ ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/lib/libzebra.so* ; \
	fi 
	if [ -e ${PROJECT_BUILD_DIR}/$@/lib/.libs/libzcommon.so.0.0.0 ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/lib/.libs/libzcommon.so.0.0.0 $(LIB_EXPORT_DIR) ; \
		ln -sf libzcommon.so.0.0.0 $(LIB_EXPORT_DIR)/libzcommon.so ; \
		ln -sf libzcommon.so.0.0.0 $(LIB_EXPORT_DIR)/libzcommon.so.0 ; \
		ln -sf libzcommon.so.0.0.0 $(LIB_EXPORT_DIR)/libzcommon.so.0.0 ; \
		cp -d $(LIB_EXPORT_DIR)/libzcommon.so* ${BUILD_ROOTFS_DIR}/opt/lib/ ; \
		cp -d $(LIB_EXPORT_DIR)/libzcommon.so* $(USER_ADDITION_LIB_ROOT_PATH)/lib/ ; \
	    chmod 777 ${BUILD_ROOTFS_DIR}/opt/lib/libzcommon.so*   ; \
	fi 
ifeq ($(findstring -DHAVE_OSPF,$(INCLUDE_CHANOS_MODULES)), -DHAVE_OSPF)			
	if [ -e ${PROJECT_BUILD_DIR}/$@/ospfd/.libs/libospf.so.0.0.0 ] ; then \
		cp ${PROJECT_BUILD_DIR}/$@/ospfd/.libs/libospf.so.0.0.0 $(LIB_EXPORT_DIR) ; \
		ln -sf libospf.so.0.0.0 $(LIB_EXPORT_DIR)/libospf.so ; \
		ln -sf libospf.so.0.0.0 $(LIB_EXPORT_DIR)/libospf.so.0 ; \
		cp -d $(LIB_EXPORT_DIR)/libospf.so* ${BUILD_ROOTFS_DIR}/opt/lib/ ; \
	fi
	-chmod 777 ${BUILD_ROOTFS_DIR}/opt/lib/libospf.so*
endif

pstack:
	@echo "Building pstack suit ..."
	$(MAKE) -C $(PSTACK_MOD)
npdsuit: bm libnpdlib sdk nam nbm npd

kap:
	@echo "Building KAP kernel module ..."
	$(MAKE) -C $(NPDSUIT_MOD)/kap/src/kmod
	cp -d ${KMOD_EXPORT_DIR}/kapDrv.ko ${ROOTFS_KMOD_DIR}/misc/
cleankap:
	@echo "Cleaning KAP kernel module ..."
	$(MAKE) -C $(NPDSUIT_MOD)/kap/src/kmod clean
bmapp: 
	$(MAKE) -C ${NPDSUIT_MOD}/bm/src/app

bm: 
	@echo "Building bm ..."
	$(MAKE) -C ${NPDSUIT_MOD}/bm/src/kmod
	$(MAKE) -C ${NPDSUIT_MOD}/bm/src/app
	cp -d ${KMOD_EXPORT_DIR}/bm.ko ${ROOTFS_KMOD_DIR}/misc/

cleanbm: 
	@echo "Cleaning bm ..."
	$(MAKE) -C ${NPDSUIT_MOD}/bm/src/kmod clean
	$(MAKE) -C ${NPDSUIT_MOD}/bm/src/app clean

sdk:
	@echo "Building SDK ($(SDK_TYPE) $(SDK_VERSION) $(SDK_PLATFORM))..."
	if [ -x ${NPDSUIT_MOD}/$(SDK_TYPE)_$(SDK_VERSION)/make.sh ] ; then \
		${NPDSUIT_MOD}/$(SDK_TYPE)_$(SDK_VERSION)/make.sh ; \
	fi

cleansdk:
	@echo "Cleaning SDK ($(SDK_TYPE) $(SDK_VERSION) $(SDK_PLATFORM))..."
	if [ -x ${NPDSUIT_MOD}/$(SDK_TYPE)_$(SDK_VERSION)/make.sh ] ; then \
		${NPDSUIT_MOD}/$(SDK_TYPE)_$(SDK_VERSION)/make.sh clean ; \
	fi

nam:
	@echo "Building nam module..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${NPDSUIT_MOD}/nam_$(SDK_TYPE)/src/lib
	cp -d ${LIB_EXPORT_DIR}/libnam* ${BUILD_ROOTFS_DIR}/opt/lib/

cleannam:
	@echo "Cleaning nam module..."
	$(MAKE) -C ${NPDSUIT_MOD}/nam_$(SDK_TYPE)/src/lib clean

nbm:
	@echo "Building nbm module..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${NPDSUIT_MOD}/nbm/src/lib
	cp -d ${LIB_EXPORT_DIR}/libnbm* ${BUILD_ROOTFS_DIR}/opt/lib/

cleannbm:
	@echo "Cleaning nbm module..."
	$(MAKE) -C ${NPDSUIT_MOD}/nbm/src/lib clean

npd: 
	@echo "Building npd ......"
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${NPDSUIT_MOD}/npd/src/app
	cp -d ${BIN_EXPORT_DIR}/npd ${BUILD_ROOTFS_DIR}/opt/bin/
	cp -d ${LIB_EXPORT_DIR}/* ${BUILD_ROOTFS_DIR}/opt/lib/

udld:
	@echo "Building UDLD..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/udld
ifeq ($(findstring -DHAVE_UDLD,$(INCLUDE_CHANOS_MODULES)), -DHAVE_UDLD)				
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${UDLD_MOD}/src
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BIN_EXPORT_DIR}
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BUILD_ROOTFS_DIR}/opt/bin
endif
	
cleanudld:
	@echo "Cleaning UDLD..."
	$(MAKE) -C ${UDLD_MOD}/src clean

sflow:
	@echo "Building SFLOW..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/sflow
#ifeq ($(findstring -DHAVE_SFLOW,$(INCLUDE_CHANOS_MODULES)), -DHAVE_SFLOW)				
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${SFLOW_MOD}/src
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BIN_EXPORT_DIR}
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BUILD_ROOTFS_DIR}/opt/bin
#endif
	
cleansflow:
	@echo "Cleaning SFLOW..."
	$(MAKE) -C ${SFLOW_MOD}/src clean

cleannpd: 
	@echo "cleaning npd ......"
	if [ ! -d ${PROJECT_BUILD_DIR}/npd ]; then mkdir ${PROJECT_BUILD_DIR}/npd; fi
	$(MAKE) -C ${NPDSUIT_MOD}/npd/src/app clean

stpsuit:
	@echo "Building stp suit..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/bridge
ifeq ($(findstring -DHAVE_BRIDGE_STP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_BRIDGE_STP)			
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${STPSUIT_MOD}/src
	cp ${STPSUIT_MOD}/src/OBJ/bridge ${BUILD_ROOTFS_DIR}/opt/bin/
endif

had:
	@echo "Building HA daemon..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/had
ifeq ($(findstring -DHAVE_VRRP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_VRRP)
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${HAD_MOD}/src/lib
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BUILD_ROOTFS_DIR}/opt/bin/
endif

smartlink:
	@echo "Building Smart-Link daemon..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/smartlink
ifeq ($(findstring -DHAVE_SMART_LINK,$(INCLUDE_CHANOS_MODULES)), -DHAVE_SMART_LINK)
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${SMART_LINK_MOD}/src
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BUILD_ROOTFS_DIR}/opt/bin/
endif

erpp:
	@echo "Building erpp daemon..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/erpp
ifeq ($(findstring -DHAVE_ERPP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_ERPP)
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${ERPP_MOD}/src
	cp ${PROJECT_BUILD_DIR}/$@/$@ ${BUILD_ROOTFS_DIR}/opt/bin/
endif
	
hbip:
	@echo "Building hbip deamon..."
	$(MAKE) -C ${HBIP_MOD}/src/
	cp ${HBIP_MOD}/src/obj/hbip ${BIN_EXPORT_DIR}/
	
igmp:
	@echo "Building igmp snooping ..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/igmp_snoop
ifeq ($(findstring -DHAVE_IGMP_SNP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_IGMP_SNP)		
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${IGMP_MOD}/
	cp ${PROJECT_BUILD_DIR}/$@/igmp_snoop ${BUILD_ROOTFS_DIR}/opt/bin/
endif

mld:
	@echo "Building mld snooping ..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/mld_snp
ifeq ($(findstring -DHAVE_MLD_SNP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_MLD_SNP)		
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${MLD_MOD}/
	cp ${PROJECT_BUILD_DIR}/$@/mld_snp ${BUILD_ROOTFS_DIR}/opt/bin/
endif

radvd:
	@echo "Building radvd ..."
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	$(MAKE) -C ${RADVD_MOD}
	cp ${RADVD_MOD}/radvd ${BUILD_ROOTFS_DIR}/opt/bin/
	cp ${RADVD_MOD}/radvdump ${BUILD_ROOTFS_DIR}/usr/bin/

dldp:
	@echo "Building DLDP..."
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/dldp
ifeq ($(findstring -DHAVE_DLDP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_DLDP)		
	$(MAKE) -C ${DLDP_MOD}/src/lib
	cp ${DLDP_MOD}/src/lib/dldp ${BIN_EXPORT_DIR}/
	cp ${DLDP_MOD}/src/lib/dldp ${BUILD_ROOTFS_DIR}/opt/bin/
endif

chkpwd:
	@echo "Building checkpasswd-pam ..."
	$(MAKE) -C ${PAM_CHECK_PASSWD_MOD}
	cp ${PAM_CHECK_PASSWD_MOD}/checkpassword-pam ${BIN_EXPORT_DIR}
	cp ${PAM_CHECK_PASSWD_MOD}/checkpassword-pam ${BUILD_ROOTFS_DIR}/usr/bin

dhcp:
	@echo "Building dhcp ..."
	@echo $(DHCP_CONFIG_HOSTCC)
ifeq ($(findstring -DHAVE_DHCP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_DHCP)		
	if [ ! -d ${PROJECT_BUILD_DIR}/$@ ]; then mkdir ${PROJECT_BUILD_DIR}/$@; fi
	if [ ! -f ${PROJECT_BUILD_DIR}/$@/Makefile ] ; then 		\
		${DHCP_MOD}/configpkg ;                                   \
	fi
endif
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/dhcpd
	rm -rf ${BUILD_ROOTFS_DIR}/opt/dhclient
	rm -rf ${BUILD_ROOTFS_DIR}/opt/bin/dhcprelay
	$(MAKE) -C ${PROJECT_BUILD_DIR}/$@
	
ifeq ($(findstring -DHAVE_DHCP,$(INCLUDE_CHANOS_MODULES)), -DHAVE_DHCP)	
	cp ${PROJECT_BUILD_DIR}/$@/server/dhcpd ${BUILD_ROOTFS_DIR}/opt/bin
endif
ifeq ($(findstring -DHAVE_DHCP_CLIENT,$(INCLUDE_CHANOS_MODULES)), -DHAVE_DHCP_CLIENT)		
	cp ${PROJECT_BUILD_DIR}/$@/client/dhclient ${BUILD_ROOTFS_DIR}/opt/bin
endif	
	cp ${DHCP_MOD}/server/dhcpd.conf ${BUILD_ROOTFS_DIR}/etc/
#	cp ${PROJECT_BUILD_DIR}/$@/dhcrelay ${BUILD_ROOTFS_DIR}/opt/bin


kmod: ${KERNEL_ROOT}/.config ${MAKE_KERNEL_BEFORE_MODULES}
	@echo "Making kernel built-in modules ..."
	make -C ${KERNEL_ROOT} modules
	[ -d ${ROOTFS_KMOD_DIR} ] || mkdir -p ${ROOTFS_KMOD_DIR}
	[ -d ${ROOTFS_KMOD_DIR}/kernel ] || mkdir -p ${ROOTFS_KMOD_DIR}/kernel
	[ -d ${ROOTFS_KMOD_DIR}/misc ] || mkdir -p ${ROOTFS_KMOD_DIR}/misc

cavium-ethernet: kmod
	@echo "Building cavium-ethernet driver kernel module."
	$(MAKE) -C ${OCTETH_KMOD} 
	cp ${OCTETH_KMOD}/*.ko ${KMOD_EXPORT_DIR}/
	cp ${OCTETH_KMOD}/*.ko ${ROOTFS_KMOD_DIR}/misc/
	#@echo "Building ipfwd kernel module for enhance cavium-ethernet driver."
	#$(MAKE) -C src/ipfwd
	#cp src/ipfwd/*.ko ${KMOD_EXPORT_DIR}/

wifikmod:
	@echo "Building wifi-ehternet ..."
	rm -rf ${ROOTFS_KMOD_DIR}/misc/wifi-ethernet.ko
ifeq ($(findstring -DHAVE_WCPSS,$(INCLUDE_CHANOS_MODULES)), -DHAVE_WCPSS)		
	$(MAKE) -C ${WCPSS_MOD}/src/kmod/wifi-ethernet/
	cp ${WCPSS_MOD}/src/kmod/wifi-ethernet/wifi-ethernet.ko ${KMOD_EXPORT_DIR}/
	cp ${WCPSS_MOD}/src/kmod/wifi-ethernet/wifi-ethernet.ko ${ROOTFS_KMOD_DIR}/misc/
endif

cleandcli:
	$(MAKE) -C ${DCLI_MOD}/src/lib clean
cleandclipub_ac:
	$(MAKE) -C ${DCLI_MOD}/src/pub clean


cleanchkpwd:
	$(MAKE) -C ${PAM_CHECK_PASSWD_MOD} clean

cleannpdsuit: cleannam cleannpd

cleantrap-helper:
	$(MAKE) -C ${TRAP_HELPER_MOD}/ clean

cleansnmp: cleansnmpd cleantrap-helper
	$(MAKE) -C ${SUBAGENT_MOD} clean

cleanwcpss:
	$(MAKE) -C ${WCPSS_MOD}/src/app/pub clean
	$(MAKE) -C ${WCPSS_MOD}/src/app/wid clean
	$(MAKE) -C ${WCPSS_MOD}/src/app/wsm clean

cleanwifikmod:
	$(MAKE) -C ${WCPSS_MOD}/src/kmod/wifi-ethernet/ clean

cleanasd:
	$(MAKE) -C ${ASD_MOD}/src/app clean

cleanquagga:
	echo "Cleanning quagga ..."
	rm -rf ${PROJECT_BUILD_DIR}/quagga

cleancavium-ethernet:
	echo "Cleanning cavium ethernet ..."
	$(MAKE) -C ${OCTETH_KMOD} clean
	#$(MAKE) -C src/ipfwd clean

cleanstpsuit:		
	echo "Cleaning stp suit..."
	$(MAKE) -C ${STPSUIT_MOD}/src clean

cleanhad:
	echo "Cleaning HA daemon..."
	${MAKE} -C ${HAD_MOD}/src/lib clean

cleansmartlink:
	echo "Cleaning Smart-Link daemon..."
	${MAKE} -C ${SMART_LINK_MOD}/src clean

cleanhbip:
	echo "Cleaning HBIP daemon..."
	${MAKE} -C ${HBIP_MOD}/src/ clean

cleanigmp:
	echo "Cleaning igmp snooping..."
	$(MAKE) -C ${IGMP_MOD}/ clean

cleanmld:
	echo "Cleaning mld snooping..."
	$(MAKE) -C ${MLD_MOD}/ clean
	
cleanradvd:
	echo "Cleaning radvd..."
	$(MAKE) -C ${RADVD_MOD}/ clean

cleandldp:
	echo "Cleaning DLDP..."
	$(MAKE) -C ${DLDP_MOD}/src/lib clean

cleanccgi:
	echo "Cleanning ccgi ..."  
	$(MAKE) -C ${CCGI_MOD}/cgic205 clean 
	-$(MAKE) -C ${CCGI_MOD}/cgic205/portal clean
	-$(MAKE) -C ${CCGI_MOD}/cgic205/snmp_agent clean

cleankernel:
	$(MAKE) -C ${KERNEL_ROOT} clean

cleandhcp:
	@echo "Cleanning dhcp ..."
	rm -rf ${PROJECT_BUILD_DIR}/dhcp

cleanpimd:
	echo "Cleanning pimd ..."
	$(MAKE) -C ${PIMD_MOD} clean
cleandvmrp:
	$(MAKE) -C ${DVMRP_MOD} clean
	
cleanerpp:
	$(MAKE) -C ${ERPP_MOD}/src clean

cleanrootfs:
	rm .busyroot2rootfstimestamp
	rm .buildroot2rootfstimestamp


