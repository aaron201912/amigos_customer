INC += $(MODULE_PATH)/BasicUsageEnvironment/include
INC += $(MODULE_PATH)/groupsock/include
INC += $(MODULE_PATH)/liveMedia/include
INC += $(MODULE_PATH)/UsageEnvironment/include
INC += $(MODULE_PATH)/mediaServer/include

SUBDIRS += $(MODULE_PATH)/BasicUsageEnvironment
SUBDIRS += $(MODULE_PATH)/groupsock
SUBDIRS += $(MODULE_PATH)/liveMedia
SUBDIRS += $(MODULE_PATH)/UsageEnvironment
SUBDIRS += $(MODULE_PATH)/mediaServer

LOCAL_CXXFLAGS += -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DBSD=1 -DLOCALE_NOT_USED -DSO_REUSEPORT -DALLOW_SERVER_PORT_REUSE -DALLOW_RTSP_SERVER_PORT_REUSE