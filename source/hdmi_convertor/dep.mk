include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)

DEP += base tem iniparser onvif live555

ifneq ($(interface_rgn), disable)
DEP += rgn
endif

3RD_PARTY_REL0 += libnl openssl
3RD_PARTY_REL1 += wpa_supplicant hostapd dnsmasq wireless_tools