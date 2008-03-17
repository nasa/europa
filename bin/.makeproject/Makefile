.PHONY: prebuild postbuild project-model project-initial-state project-lib project-rt run-project clean all
all: prebuild project-controller project-lib run-project project-jar postbuild

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

PROJECT_CONTROLLER = $(call findtarget,$(LIB_PREFIX)$(PROJECT)Controller$(BUILD_SUFFIX).$(LIB_EXT))
project-controller: prebuild $(PROJECT_CONTROLLER)
$(PROJECT_CONTROLLER): $(call findtarget,ControllerStub$(BUILD_SUFFIX).o)

PROJECT_MODEL = $(call findtarget,$(PROJECT)-model$(BUILD_SUFFIX).o)
project-model: prebuild $(PROJECT_MODEL)
$(PROJECT_MODEL): $(PROJECT)-model.nddl

PROJECT_INITIAL_STATE = $(PROJECT)-initial-state.xml
project-initial-state: prebuild $(PROJECT_INITIAL_STATE)
$(PROJECT_INITIAL_STATE): $(PROJECT)-initial-state.nddl $(PROJECT)-model.nddl

PROJECT_SWIG = $(call findtarget,$(PROJECT)_wrap$(BUILD_SUFFIX).o)
project-swig: prebuild $(PROJECT_SWIG)
$(PROJECT_SWIG): $(PROJECT).i

PROJECT_LIB = $(call findtarget,$(LIB_PREFIX)$(PROJECT)$(BUILD_SUFFIX).$(LIB_EXT))
project-lib: prebuild $(PROJECT_LIB)
$(PROJECT_LIB): $(PROJECT_MODEL) \
                $(PROJECT_SWIG) \
                $(call findtarget,$(PROJECT)CustomCode$(BUILD_SUFFIX).o)

PROJECT_RT = $(call findtarget,$(PROJECT)-planner$(BUILD_SUFFIX)$(RT_SUFFIX))
project-rt: prebuild $(PROJECT_RT)
$(PROJECT_RT): $(PROJECT_MODEL) \
               $(call findtarget,$(PROJECT)-Main$(BUILD_SUFFIX).o) \
               $(call findtarget,$(PROJECT)CustomCode$(BUILD_SUFFIX).o)

RUN_PROJECT = RUN_$(notdir $(PROJECT_RT)).$(notdir $(PROJECT_INITIAL_STATE)).PlannerConfig.xml.output
run-project: prebuild $(RUN_PROJECT)
$(RUN_PROJECT): $(PROJECT_RT) $(PROJECT_INITIAL_STATE) PlannerConfig.xml
	./$+ > $@

PROJECT_JAR = $(call findtarget,$(PROJECT).jar)
project-jar: prebuild $(PROJECT_JAR)
$(PROJECT_JAR): $(PROJECT_SWIG)
	$(ANT) $(PROJECT) 

clean:
	$(RM) build $(RUN_PROJECT) $(PROJECT)-model.hh $(PROJECT_INITIAL_STATE)
