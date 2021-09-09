LINK_TYPE ?= static
INTER_LINK_TYPE ?= static

ifneq ($(MODULE_PATH), )
EXEFILE:=$(MODULE_NAME)
LIB_NAME=$(MODULE_NAME)
SUBDIRS:=$(MODULE_PATH)
include $(BUILD_TOP)/mi_dep.mk
-include $(MODULE_PATH)/$(notdir $(MODULE_PATH)).mk
-include $(MODULE_PATH)/dep.mk
LIBS := $(foreach m,$(DEP),-l$(m)) $(LIBS)
INC += $(filter $(foreach d,$(DEP),%$(d)), $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
endif
INC  += $(BUILD_TOP)/common
INC  += $(ALKAID_PROJ)/release/include
ifneq ($(PROJ_ROOT), )
INC  += $(PROJ_ROOT)/release/include
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include

ifeq ($(CHIP), i2)
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/$(LINK_TYPE)
else
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/$(LINK_TYPE)
LIBS += -L$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/ex_libs/$(LINK_TYPE)
LIBS +=  -lcam_fs_wrapper -lcam_os_wrapper
endif
LIBS += -lmi_sys -lmi_common
endif
LIBS += -lrt -lpthread -lm -ldl
LIBS += -L$(BUILD_TOP)/prebuild_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(INTER_LINK_TYPE)/
LIBS += -L$(OUT_PATH)/$(ARCH)/libs/$(INTER_LINK_TYPE)/

ifeq ($(DUAL_OS), on)
CODEDEFINE += -DLINUX_FLOW_ON_DUAL_OS
endif
CODEDEFINE += -DTRANS_BUFFER=480 -DSOCKET_ADDR='"/tmp/cmd_base"'

ifeq ($(CHIP), i2)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I2=1
else ifeq ($(CHIP), i2m)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I2M=1
else ifeq ($(CHIP), i6e)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I6E=1
else ifeq ($(CHIP), i6b0)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I6B0=1
endif

MODULE_RELEASE_PACKAGE ?= off
#APP_REL_PREFIX ?= $(MODULE_NAME)

module_install:
ifneq ($(APP_REL_PREFIX), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/
	@cp -vrf $(OUT_PATH)/$(ARCH)/app/$(EXEFILE) $(IMAGE_PATH)/$(APP_REL_PREFIX)/
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/$(EXEFILE)
ifneq ($(MODULE_REL_FILES), )
	$(foreach n,$(MODULE_REL_FILES),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/;)
endif
ifneq ($(MODULE_REL_LIB), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib/
	$(foreach n,$(MODULE_REL_LIB),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib;)
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib/*
endif
ifneq ($(MODULE_REL_BIN), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin/
	$(foreach n,$(MODULE_REL_BIN),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin;)
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin/*
endif
ifeq ($(MODULE_RELEASE_PACKAGE), on)
	@rm -rfv $(IMAGE_PATH)/$(APP_REL_PREFIX).sqfs
	@mksquashfs $(IMAGE_PATH)/$(APP_REL_PREFIX) $(IMAGE_PATH)/$(APP_REL_PREFIX).sqfs -comp xz -all-root
endif
endif

include $(BUILD_TOP)/compile.mk
