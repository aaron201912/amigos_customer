LINK_TYPE := static
INTER_LINK_TYPE := static

INC  += $(BUILD_TOP)/../common/live555/UsageEnvironment/include
INC  += $(BUILD_TOP)/../common/live555/groupsock/include
INC  += $(BUILD_TOP)/../common/live555/liveMedia/include
INC  += $(BUILD_TOP)/../common/live555/BasicUsageEnvironment/include
INC  += $(BUILD_TOP)/../common/live555/mediaServer/include
INC  += $(BUILD_TOP)/internal/amigos_base/include/

ifneq ($(interface_vif), disable)
LIBS += -lmi_vif
endif

ifneq ($(interface_vpe), disable)
LIBS += -lmi_vpe
endif

ifneq ($(interface_venc), disable)
LIBS += -lmi_venc
endif

ifneq ($(interface_rgn), disable)
LIBS += -lmi_rgn
endif

ifneq ($(interface_divp), disable)
LIBS += -lmi_divp
endif

ifneq ($(interface_isp), disable)
LIBS += -lmi_isp -lcus3a -lispalgo
endif

ifneq ($(interface_disp), disable)
#LIBS += -lmi_disp
endif

ifneq ($(interface_panel), disable)
LIBS += -lmi_panel
endif

ifneq ($(interface_iqserver), disable)
LIBS += -lmi_iqserver
endif

ifneq ($(filter $(CHIP_ALIAS), macaron pudding tiramisu),)
LIBS += -lfbc_decode
endif

ifneq ($(interface_sensor), disable)
LIBS += -lmi_sensor
endif

ifneq ($(interface_vdec), disable)
LIBS += -lmi_vdec
endif

ifneq ($(interface_vdisp), disable)
LIBS += -lmi_vdisp
endif

ifneq ($(interface_ai), disable)
LIBS += -lmi_ai
endif

ifneq ($(interface_ao), disable)
LIBS += -lmi_ao
endif

MODULE_RELEASE_PACKAGE := on
APP_REL_PREFIX := hdmiap
MODULE_REL_FILES += $(MODULE_PATH)/configs/$(CHIP_ALIAS)/2M_AUDIO.ini
MODULE_REL_FILES += $(MODULE_PATH)/configs/wuxinhao.index2
MODULE_REL_FILES += $(MODULE_PATH)/configs/nosignal.index2
MODULE_REL_FILES += $(MODULE_PATH)/configs/dnsmasq.conf
MODULE_REL_FILES += $(MODULE_PATH)/configs/hostapd.conf
MODULE_REL_FILES += $(MODULE_PATH)/configs/wlan.json
MODULE_REL_FILES += $(MODULE_PATH)/configs/run.sh
MODULE_REL_FILES += $(MODULE_PATH)/configs/run.eth0.dhcp.337.ip.sh
MODULE_REL_BIN += $(3PARTY_PATH)/dnsmasq/bin/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dnsmasq
MODULE_REL_BIN += $(3PARTY_PATH)/hostapd/bin/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/hostapd
MODULE_REL_BIN += $(3PARTY_PATH)/hostapd/bin/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/hostapd_cli
MODULE_REL_BIN += $(3PARTY_PATH)/wireless_tools/bin/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/iwlist $(3PARTY_PATH)/wireless_tools/bin/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/iwpriv
MODULE_REL_BIN += $(3PARTY_PATH)/wpa_supplicant/bin/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/wpa_supplicant
MODULE_REL_BIN += $(3PARTY_PATH)/wpa_supplicant/bin/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/wpa_cli
MODULE_REL_LIB += $(3PARTY_PATH)/wireless_tools/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/libiw.so $(3PARTY_PATH)/wireless_tools/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/libiw.so.29
MODULE_REL_LIB += $(3PARTY_PATH)/openssl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libcrypto.so $(3PARTY_PATH)/openssl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libcrypto.so.1.1
MODULE_REL_LIB += $(3PARTY_PATH)/openssl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libssl.so $(3PARTY_PATH)/openssl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libssl.so.1.1
MODULE_REL_LIB += $(3PARTY_PATH)/libnl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/llibnl-3.so $(3PARTY_PATH)/libnl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libnl-3.so.200 $(3PARTY_PATH)/libnl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libnl-3.so.200.26.0
MODULE_REL_LIB += $(3PARTY_PATH)/libnl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libnl-genl-3.so $(3PARTY_PATH)/libnl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libnl-genl-3.so.200 $(3PARTY_PATH)/libnl/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/dynamic/libnl-genl-3.so.200.26.0