ifneq ($(ARCH),x86)
include $(ALKAID_PROJ_CONFIG)
include $(PROJ_ROOT)/release/customer_tailor/$(CUSTOMER_TAILOR)
ifeq ($(CROSS_COMPILE), )
MY_TOOLCHAIN := $(TOOLCHAIN_REL)
else
MY_TOOLCHAIN := $(CROSS_COMPILE)
endif
unexport CROSS_COMPILE
TOOLCHAIN_REL := $(MY_TOOLCHAIN)
CC := $(MY_TOOLCHAIN)gcc
CXX := $(MY_TOOLCHAIN)g++
AR := $(MY_TOOLCHAIN)ar
STRIP := $(MY_TOOLCHAIN)strip
ifeq ($(PRODUCT), xvr)
CUSTOMER_ENABLED := snr9931
endif
INTERFACE_MODULES := vpe vif divp disp vdec venc rgn vdisp sys ai ao gfx ipu sensor vdf ldc shadow panel hdmi isp iqserver
INTERFACE_ENABLED:=$(patsubst %_enable_,%,$(patsubst %__,%,$(filter-out %_disable_, $(foreach n,$(INTERFACE_MODULES),$(n)_$(interface_$(n))_))))
INTERFACE_DISABLED:=$(filter-out $(INTERFACE_ENABLED),$(INTERFACE_MODULES))

CODEDEFINE += $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n)))
CODEDEFINE += -DSSTAR_CHIP_$(shell tr 'a-z' 'A-Z' <<< $(CHIP))
else
CC := gcc
CXX := g++
AR := ar
STRIP := strip
endif
