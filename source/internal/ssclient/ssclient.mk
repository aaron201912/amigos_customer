include $(BUILD_TOP)/mi_dep.mk

INC  += $(BUILD_TOP)/../common/live555/UsageEnvironment/include
INC  += $(BUILD_TOP)/../common/live555/groupsock/include
INC  += $(BUILD_TOP)/../common/live555/liveMedia/include
INC  += $(BUILD_TOP)/../common/live555/BasicUsageEnvironment/include
INC  += $(BUILD_TOP)/../common/live555/mediaServer/include
INC += $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))
INC += $(MODULE_PATH)/../amigos_base/include/
