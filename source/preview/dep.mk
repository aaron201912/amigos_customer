DEP += amigos_base tem iniparser onvif live555

ifneq ($(interface_rgn), disable)
DEP += rgn
endif

#3RD_PARTY_DEP0 += z png jpeg