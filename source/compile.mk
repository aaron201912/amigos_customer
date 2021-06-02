.PHONY :all clean gen_exe gen_obj clean_files gen_lib

include $(DB_ALKAID_PROJ)
ifneq ($(USE_X86), 1)
CC := $(MY_TOOLCHAIN)gcc
CXX := $(MY_TOOLCHAIN)g++
AR := $(MY_TOOLCHAIN)ar
STRIP := $(MY_TOOLCHAIN)strip
else
CC := gcc
CXX := g++
AR := ar
STRIP := strip
GCCFLAGS := -Wall -g
endif

#GCCFLAGS := -Wall -g -Werror
GCCFLAGS ?= -Wall -g -pipe -fPIC
CXXFLAGS := $(GCCFLAGS) $(LOCAL_CXXFLAGS)
CXXFLAGS += $(CODEDEFINE) -DLINUX_OS -std=gnu++11
CXXFLAGS += $(foreach dir,$(INC),-I$(dir))

CFLAGS := $(GCCFLAGS) $(LOCAL_CFLAGS)
CFLAGS += $(CODEDEFINE) -DLINUX_OS -DGIT_COMMIT=\"$(shell git rev-parse HEAD)\" -DBUILD_OWNER=\"$(shell whoami)\" -DBUILD_DATE="\"$(shell date +"%Y-%m-%d %T")\""
CFLAGS += $(foreach dir,$(INC),-I$(dir))

SRC    +=  $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c)) $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp))
OBJS_CXX := $(patsubst %.cpp,%.o,$(filter %.cpp, $(SRC)))
OBJS     := $(patsubst %.c,%.o,$(filter %.c, $(SRC)))


DFILES := $(foreach f,$(OBJS_CXX) $(OBJS),$(patsubst %.o,%.d,$(f)))
sinclude $(DFILES)

$(OBJS):%.o:%.c
	@echo compile $(notdir $<)...
	@$(CC) $(CFLAGS) -MM $< | sed -e '1s/$(notdir $@):/$(subst /,\/,$@): $(subst /,\/,$(@:.o=.d))/' > $(@:.o=.d)
	@$(CC) $(CFLAGS) -c -ffunction-sections -fdata-sections $< -o $@
$(OBJS_CXX):%.o:%.cpp
	@echo compile $(notdir $<)...
	@$(CXX) $(CXXFLAGS) -MM $< | sed -e '1s/$(notdir $@):/$(subst /,\/,$@): $(subst /,\/,$(@:.o=.d))/' > $(@:.o=.d)
	@$(CXX) $(CXXFLAGS) -c -ffunction-sections -fdata-sections $< -o $@

gen_exe:$(OBJS_CXX) $(OBJS)
ifneq ($(OBJS_CXX), )
	@mkdir -p $(OUTPUT_DIR)
	@$(CXX) $(CXXFLAGS) -Wl,--gc-sections $(OBJS_CXX) $(OBJS) $(LIBS) -o $(OUTPUT_DIR)/$(EXEFILE)
else
ifneq ($(OBJS), )
	@mkdir -p $(OUTPUT_DIR)
	@$(CC) $(CFLAGS) -Wl,--gc-sections $(OBJS) $(LIBS) -o $(OUTPUT_DIR)/$(EXEFILE)
endif
endif

gen_obj:$(OBJS_CXX) $(OBJS)
ifneq ($(OBJS_CXX), )
	@mkdir -p $(OUTPUT_DIR)
	@cp $(OBJS_CXX) $(OUTPUT_DIR)
endif
ifneq ($(OBJS), )
	@mkdir -p $(OUTPUT_DIR)
	@cp $(OBJS) $(OUTPUT_DIR)
endif

gen_lib:
ifeq ($(LIB_TYPE), static)
	@$(AR) sq lib$(LIB_NAME).a $(OBJ_FILES)
ifneq ($(OUTPUT_DIR), )
	@mkdir -p $(OUTPUT_DIR)
	@mv ./lib$(LIB_NAME).a $(OUTPUT_DIR)
endif
endif
ifeq ($(LIB_TYPE), dynamic)
	@$(CC) -shared -fPIC -o lib$(LIB_NAME).so $(OBJ_FILES)
ifneq ($(OUTPUT_DIR), )
	@mkdir -p $(OUTPUT_DIR)
	@mv ./lib$(LIB_NAME).so $(OUTPUT_DIR)
endif
endif

clean_files:
	@rm -rf $(OBJS_CXX) $(OBJS) $(OBJS_CXX:.o=.d) $(OBJS:.o=.d)
	@rm -rf $(OUTPUT_DIR)
ifneq ($(EXEFILE), )
	@rm -rf $(OUTPUT_DIR)/$(EXEFILE)
endif
