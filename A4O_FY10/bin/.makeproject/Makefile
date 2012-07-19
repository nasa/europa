.PHONY: prebuild postbuild project-model project-initial-state project-lib project-rt run-project clean all
all: prebuild project-lib run-project postbuild

include platforms.mk
include makerules.mk

PROJECT := %%Project%%

.SILENT: prebuild postbuild
prebuild:
	$(MKDIR) build/object
	$(MKDIR) build/lib
	$(MKDIR) build/bin
	$(MKDIR) build/swig
	$(MKDIR) build/swig/$(PROJECT)
	$(if $(findstring undefined,$(origin EUROPA_HOME)),echo EUROPA_HOME not found && false,)

postbuild:
	echo Build Completed Successfully!

PROJECT_SWIG = $(call findtarget,$(PROJECT)_wrap$(BUILD_SUFFIX).o)
project-swig: prebuild $(PROJECT_SWIG)
$(PROJECT_SWIG): $(PROJECT).i

PROJECT_LIB = $(call findtarget,$(LIB_PREFIX)$(PROJECT)$(BUILD_SUFFIX).$(LIB_EXT))
project-lib: prebuild $(PROJECT_LIB)
$(PROJECT_LIB): $(PROJECT_SWIG) \
                $(call findtarget,$(PROJECT)CustomCode$(BUILD_SUFFIX).o) \
                $(call findtarget,Module$(PROJECT)$(BUILD_SUFFIX).o)

PROJECT_RT = $(call findtarget,$(PROJECT)-planner$(BUILD_SUFFIX)$(RT_SUFFIX))
project-rt: prebuild $(PROJECT_RT)
$(PROJECT_RT): $(call findtarget,$(PROJECT)-Main$(BUILD_SUFFIX).o) \
               $(call findtarget,$(PROJECT)CustomCode$(BUILD_SUFFIX).o) \
               $(call findtarget,Module$(PROJECT)$(BUILD_SUFFIX).o)

RUN_PROJECT = RUN_$(notdir $(PROJECT_RT)).$(notdir $(PROJECT)-initial-state.nddl).PlannerConfig.xml.output
run-project: prebuild $(RUN_PROJECT)
$(RUN_PROJECT): $(PROJECT_RT) $(PROJECT)-initial-state.nddl PlannerConfig.xml
	./$+ > $@

PROJECT_JAR = $(call findtarget,$(PROJECT).jar)
project-jar: prebuild $(PROJECT_JAR)
$(PROJECT_JAR): $(PROJECT_SWIG)
	$(ANT) $(PROJECT) 

clean:
	$(RM) build $(RUN_PROJECT) $(PROJECT)-model.hh $(PROJECT)-model.cc $(PROJECT)_wrap.cc
