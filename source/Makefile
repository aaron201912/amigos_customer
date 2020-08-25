DB_BUILD_TOP:=$(shell pwd)
DB_OUT_PATH:=$(DB_BUILD_TOP)/out
DB_ALKAID_PROJ:=~/203_sdk/project/configs/current.configs
DB_3PARTY_PATH:=$(DB_BUILD_TOP)/../3rdparty
DB_IMAGE_PATH := $(DB_BUILD_TOP)/image
include $(DB_ALKAID_PROJ)
-include $(MODULE)/$(MODULE).mk

MODULES := preview 
ifeq ($(PRODUCT), nvr)
MODULES += rtspclient
endif
ifeq ($(PRODUCT), ipc)
MODULES += hdmi_convertor
endif

LIBS_PATH := ../common/ ./internal/

INTERNAL_LIBS := $(notdir $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
MODULES_CLEAN := $(foreach m,$(MODULES),$(m)_clean)
MODULES_APP_ALL := $(foreach m,$(MODULES),$(m)_app_all)
MODULES_APP_CLEAN := $(foreach m,$(MODULES),$(m)_app_clean)
MODULES_OBJS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_all)
MODULES_OBJS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_clean)
MODULES_LIBS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_all)
MODULES_LIBS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_clean)
MODULES_INSTALL := $(foreach m,$(MODULES),$(m)_install)
3RD_PARTY := $(foreach m,$(notdir $(wildcard $(DB_3PARTY_PATH)/*)),$(m)_3rd_party_all)

export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION TOOLCHAIN_REL KERNEL_VERSION CUSTOMER_TAILOR CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED DUAL_OS CHIP_ALIAS
export DB_BUILD_TOP
export DB_OUT_PATH DB_IMAGE_PATH
export DB_PROJECT
export DB_ALKAID_PROJ
export TARGET_OUT
export LIBS_PATH
export DB_3PARTY_PATH

.PHONY: all 3rdparty lib obj clean install depend_internal depend_internal_clean depend_3rd_party $(MODULES) $(MODULES_CLEAN) $(MODULES_APP_ALL) $(MODULES_APP_CLEAN) $(MODULES_OBJS_ALL) $(MODULES_OBJS_CLEAN) $(MODULES_LIBS_ALL) $(MODULES_LIBS_CLEAN)

-include $(foreach mod,$(MODULES_IN),$(mod)/dep.mk)

all:
	#@$(MAKE) MODULES_IN="$(MODULES)" depend_3rd_party
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal
ifneq ($(MODULES_APP_ALL), )
	@$(MAKE) $(MODULES_APP_ALL)
endif

3rdparty:
	@$(MAKE) MODULES_IN="$(MODULES)" depend_3rd_party

clean:
	@$(MAKE) $(foreach mod,$(MODULES),$(patsubst %,%_clean,$(mod)))
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal_clean
	@rm $(DB_OUT_PATH) -rf
	@rm ./obj/ -rf

install:
	@$(MAKE) $(MODULES_INSTALL)

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

module_install:
ifneq ($(APP_PREFIX), )
	@mkdir -p $(APP_PREFIX)/bin/
	@mkdir -p $(APP_PREFIX)/lib/
	@cp -vrf $(DB_OUT_PATH)/app/$(MODULE) $(APP_PREFIX)/
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(APP_PREFIX)/$(MODULE)
ifneq ($(MODULE_REL_FILES), )
	$(foreach n,$(MODULE_REL_FILES),cp -rfvd $(n) $(APP_PREFIX)/;)
endif
ifneq ($(MODULE_REL_LIB), )
	$(foreach n,$(MODULE_REL_LIB),cp -rfvd $(n) $(APP_PREFIX)/lib;)
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(APP_PREFIX)/lib/*
endif
ifneq ($(MODULE_REL_BIN), )
	$(foreach n,$(MODULE_REL_BIN),cp -rfvd $(n) $(APP_PREFIX)/bin;)
	@$(TOOLCHAIN_REL)strip --strip-unneeded $(APP_PREFIX)/bin/*
endif
	@rm -rfv $(DB_IMAGE_PATH)/$(shell basename $(APP_PREFIX)).sqfs
	@$(PROJ_ROOT)/image/build/mksquashfs_xz $(APP_PREFIX) $(DB_IMAGE_PATH)/$(shell basename $(APP_PREFIX)).sqfs -comp xz -all-root
endif

$(3RD_PARTY):
	@$(MAKE) -C $(DB_3PARTY_PATH)/$(patsubst %_3rd_party_all,%,$@)

$(MODULES_INSTALL):
	@$(MAKE) MODULE=$(patsubst %_install,%,$@) module_install

$(MODULES):
	#@$(MAKE) MODULES_IN=$@ depend_3rd_party
	@$(MAKE) MODULES_IN=$@ depend_internal
	@$(MAKE) $(patsubst %,%_app_all,$@)

$(MODULES_CLEAN):
	@$(MAKE) $(patsubst %_clean,%_app_clean,$@)
	@$(MAKE) MODULES_IN=$@ depend_internal_clean

$(MODULES_APP_ALL):
	@$(MAKE) MODULE=$(patsubst %_app_all,%,$@) -f ./MakeApp.mk all

$(MODULES_APP_CLEAN):
	@$(MAKE) MODULE=$(patsubst %_app_clean,%,$@) -f ./MakeApp.mk clean

$(MODULES_OBJS_ALL):
	@$(MAKE) MODULE_OBJ=$(patsubst %_obj_all,%,$@) MODULE_OBJ_SRC=$(filter %$(patsubst %_obj_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./MakeObj.mk all

$(MODULES_OBJS_CLEAN):
	@$(MAKE) MODULE_OBJ=$(patsubst %_obj_clean,%,$@) MODULE_OBJ_SRC=$(filter %$(patsubst %_obj_clean,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./MakeObj.mk clean

$(MODULES_LIBS_ALL):
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_all,%,$@) LIB_TYPE=static -f ./MakeLib.mk all
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_all,%,$@) LIB_TYPE=dynamic -f ./MakeLib.mk all

$(MODULES_LIBS_CLEAN):
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_clean,%,$@) LIB_TYPE=static -f ./MakeLib.mk clean
	@$(MAKE) MODULE_LIB=$(patsubst %_lib_clean,%,$@) LIB_TYPE=dynamic -f ./MakeLib.mk clean
