BUILD_TOP:=$(shell pwd)
ALKAID_PATH ?= ~/ALKAID/
OUT_PATH:=$(BUILD_TOP)/out
ALKAID_PROJ:=$(ALKAID_PATH)/project
ALKAID_PROJ_CONFIG:=$(ALKAID_PATH)/project/configs/current.configs
3PARTY_PATH:=$(BUILD_TOP)/../3rdparty
IMAGE_PATH := $(BUILD_TOP)/release

include $(BUILD_TOP)/mi_dep.mk

MODULES := preview 
ifeq ($(PRODUCT), nvr)
MODULES += rtspclient kvm_host
endif
ifeq ($(PRODUCT), ipc)
MODULES += hdmi_convertor kvm_dev
endif

LIBS_PATH := ../common internal

INTERNAL_LIBS := $(notdir $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
MODULES_CLEAN := $(foreach m,$(MODULES),$(m)_clean)
MODULES_APP_ALL := $(foreach m,$(MODULES),$(m)_app_all)
MODULES_APP_CLEAN := $(foreach m,$(MODULES),$(m)_app_clean)
MODULES_OBJS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_all)
MODULES_OBJS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_clean)
MODULES_LIBS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_all)
MODULES_LIBS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_clean)
MODULES_INSTALL := $(foreach m,$(MODULES),$(m)_install)
3RD_PARTY := $(foreach m,$(notdir $(wildcard $(3PARTY_PATH)/*)),$(m)_3rd_party_all)

export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION TOOLCHAIN_REL KERNEL_VERSION CUSTOMER_TAILOR CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED DUAL_OS CHIP_ALIAS MY_TOOLCHAIN
export BUILD_TOP
export OUT_PATH IMAGE_PATH
export ALKAID_PROJ
export ALKAID_PROJ_CONFIG
export TARGET_OUT
export LIBS_PATH
export 3PARTY_PATH
export ARCH

.PHONY: all 3rdparty lib obj clean install depend_internal depend_internal_clean $(MODULES) $(MODULES_CLEAN) $(MODULES_APP_ALL) $(MODULES_APP_CLEAN) $(MODULES_OBJS_ALL) $(MODULES_OBJS_CLEAN) $(MODULES_LIBS_ALL) $(MODULES_LIBS_CLEAN)

-include $(foreach mod,$(MODULES_IN),$(BUILD_TOP)/$(mod)/dep.mk)

all:
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal
ifneq ($(MODULES_APP_ALL), )
	@$(MAKE) $(MODULES_APP_ALL)
endif

3rdparty:
	@$(MAKE) MODULES_IN="$(MODULES)" depend_3rd_party

clean:
	@$(MAKE) $(foreach mod,$(MODULES),$(patsubst %,%_app_clean,$(mod)))
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal_clean
	@rm $(OUT_PATH) -rf
	@rm $(IMAGE_PATH) -rf

install:
	@rm -rvf $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo
	@mkdir -p $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/
	@$(MAKE) $(MODULES_INSTALL)
ifneq ($(wildcard $(IMAGE_PATH)),)
	@cp -rvf $(IMAGE_PATH)/* $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/
endif

depend_internal:
ifneq ($(DEP), )
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_all)
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_all)
endif

depend_internal_clean:
ifneq ($(DEP), )
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_clean)
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_clean)
endif

depend_3rd_party:
ifneq ($(3RD_PARTY_REL0), )
	@$(MAKE) $(foreach m,$(sort $(3RD_PARTY_REL0)),$(m)_3rd_party_all)
endif
ifneq ($(3RD_PARTY_REL1), )
	@$(MAKE) $(foreach m,$(sort $(3RD_PARTY_REL1)),$(m)_3rd_party_all)
endif
ifneq ($(3RD_PARTY_DEP0), )
	@$(MAKE) $(foreach m,$(sort $(3RD_PARTY_DEP0)),$(m)_3rd_party_all)
endif
ifneq ($(3RD_PARTY_DEP1), )
	@$(MAKE) $(foreach m,$(sort $(3RD_PARTY_DEP1)),$(m)_3rd_party_all)
endif

$(3RD_PARTY):
	@$(MAKE) -C $(3PARTY_PATH)/$(patsubst %_3rd_party_all,%,$@)

$(MODULES_INSTALL):
	@$(MAKE) MODULE_NAME=$(patsubst %_install,%,$@) MODULE_PATH=$(patsubst %_install,%,$@) -f ./build.mk module_install

$(MODULES):
	@$(MAKE) MODULES_IN=$@ depend_internal
	@$(MAKE) $(patsubst %,%_app_all,$@)

$(MODULES_CLEAN):
	@$(MAKE) $(patsubst %_clean,%_app_clean,$@)
	@$(MAKE) MODULES_IN=$(patsubst %_clean,%,$@) depend_internal_clean

$(MODULES_APP_ALL):
	@$(MAKE) MODULE_NAME=$(patsubst %_app_all,%,$@) MODULE_PATH=$(patsubst %_app_all,%,$@) -f ./build.mk gen_exe

$(MODULES_APP_CLEAN):
	@$(MAKE) MODULE_NAME=$(patsubst %_app_clean,%,$@) MODULE_PATH=$(patsubst %_app_clean,%,$@) -f ./build.mk clean_files

$(MODULES_OBJS_ALL):
	@$(MAKE) MODULE_PATH=$(filter %$(patsubst %_obj_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./build.mk gen_obj

$(MODULES_OBJS_CLEAN):
	$(MAKE) MODULE_PATH=$(filter %$(patsubst %_obj_clean,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./build.mk clean_files

$(MODULES_LIBS_ALL):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=static -f ./build.mk gen_lib
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=dynamic -f ./build.mk gen_lib

$(MODULES_LIBS_CLEAN):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=static -f ./build.mk clean_files
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=dynamic -f ./build.mk clean_files
