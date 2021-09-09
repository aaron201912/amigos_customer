INC  += $(BUILD_TOP)/../common/live555/UsageEnvironment/include
INC  += $(BUILD_TOP)/../common/live555/groupsock/include
INC  += $(BUILD_TOP)/../common/live555/liveMedia/include
INC  += $(BUILD_TOP)/../common/live555/BasicUsageEnvironment/include
INC  += $(BUILD_TOP)/../common/live555/mediaServer/include
INC += $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))
INC += $(MODULE_PATH)/include/

SUBDIRS :=
ifneq ($(interface_rgn), disable)
SRC += $(MODULE_PATH)/src/ui.cpp
endif
ifneq ($(interface_vpe), disable)
SRC += $(MODULE_PATH)/src/vpe.cpp
endif
ifneq ($(interface_vif), disable)
SRC += $(MODULE_PATH)/src/vif.cpp
endif
ifneq ($(interface_divp), disable)
SRC += $(MODULE_PATH)/src/divp.cpp
endif
ifneq ($(interface_venc), disable)
SRC += $(MODULE_PATH)/src/venc.cpp
endif
ifneq ($(interface_disp), disable)
SRC += $(MODULE_PATH)/src/disp.cpp
endif
ifneq ($(interface_iqserver), disable)
SRC += $(MODULE_PATH)/src/iq.cpp
endif
ifneq ($(interface_vdec), disable)
SRC += $(MODULE_PATH)/src/vdec.cpp
endif
ifneq ($(interface_vdisp), disable)
SRC += $(MODULE_PATH)/src/vdisp.cpp
endif
ifneq ($(interface_ai), disable)
SRC += $(MODULE_PATH)/src/ai.cpp
endif
ifneq ($(interface_ao), disable)
SRC += $(MODULE_PATH)/src/ao.cpp
endif
ifneq ($(interface_sensor), disable)
SRC += $(MODULE_PATH)/src/snr.cpp
endif
ifneq ($(interface_sys), disable)
SRC += $(MODULE_PATH)/src/uvc.cpp
SRC += $(MODULE_PATH)/src/uac.cpp
SRC += $(MODULE_PATH)/src/inject.cpp
SRC += $(MODULE_PATH)/src/empty.cpp
SRC += $(MODULE_PATH)/src/slot.cpp
endif
SRC += $(MODULE_PATH)/src/file.cpp
SRC += $(MODULE_PATH)/src/rtsp.cpp
SRC += $(MODULE_PATH)/src/sys.cpp
SRC += $(MODULE_PATH)/src/dynamic.cpp
