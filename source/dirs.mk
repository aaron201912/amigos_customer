LINK_TYPE ?= static
INTER_LINK_TYPE ?= static

ifeq ($(CHIP), i2)
INC  += $(PROJ_ROOT)/release/$(PRODUCT)/include
else
INC  += $(PROJ_ROOT)/release/include
endif
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include

LIBS += -lrt -lpthread -lm -ldl

ifeq ($(CHIP), i2)
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/$(LINK_TYPE)
else
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/$(LINK_TYPE)
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/ex_libs/$(LINK_TYPE)
endif
LIBS += -L./lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(INTER_LINK_TYPE)/

ifeq ($(DUAL_OS), on)
CODEDEFINE += -DLINUX_FLOW_ON_DUAL_OS
endif

-include $(MODULE)/dep.mk
INC += $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))
LIBS += $(foreach m,$(DEP),-l$(m))
INC += $(foreach m,$(3RD_PARTY_DEP1) $(3RD_PARTY_DEP0),$(DB_3PARTY_PATH)/$(m)/include)
LIBS += $(foreach m,$(3RD_PARTY_DEP1) $(3RD_PARTY_DEP0),-l$(m))
LIBS += $(foreach m, $(3RD_PARTY_DEP1) $(3RD_PARTY_DEP0),-L$(DB_3PARTY_PATH)/$(m)/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(LINK_TYPE))