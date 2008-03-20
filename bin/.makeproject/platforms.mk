UNAME := $(shell uname)

ifeq (1,$(FAST))
  BUILD_SUFFIX := _o
  CXXFLAGS += -O3 -DEUROPA_FAST
  LDFLAGS += -O3
else
  BUILD_SUFFIX := _g
  CXXFLAGS += -ggdb3
endif

ifdef ANT_HOME
  ANT := $(ANT_HOME)/bin/ant
else
  ANT := ant
endif
SWIG := swig
RM := rm -rf
MV := mv
MKDIR := mkdir -p
CXX := g++
ifdef JAVA_HOME
  JAVA := $(JAVA_HOME)/bin/java
else
  JAVA := java
endif


RT_SUFFIX := _rt
LIB_PREFIX := lib
SHARED_LINK_FLAG := -shared
POSITION_INDEPENDENT_FLAG := -fPIC

ifneq (,$(findstring Linux,$(UNAME)))
  LINUX := 1
  LIB_EXT := so
  ifdef JAVA_HOME
    CXXFLAGS += -I"$(JAVA_HOME)/include"
    CXXFLAGS += -I"$(JAVA_HOME)/include/linux"
  endif
endif

ifneq (,$(findstring CYGWIN,$(UNAME)))
  CYGWIN := 1
  LIB_EXT := dll
  RT_SUFFIX := _rt.exe
  POSITION_INDEPENDENT_FLAG := 
  ifdef JAVA_HOME
    CXXFLAGS += -I"$(JAVA_HOME)/include"
    CXXFLAGS += -I"$(JAVA_HOME)/include/win32"
  endif
endif

ifneq (,$(findstring Darwin,$(UNAME)))
  DARWIN := 1
  LIB_EXT := dylib
  SHARED_LINK_FLAG := -dynamiclib
  ifdef $(JAVA_HOME)
    CXXFLAGS += -I"$(JAVA_HOME)/include"
  else
    CXXFLAGS += -I"/System/Library/Frameworks/JavaVM.framework/Headers"
  endif
endif

ifneq (,$(findstring Solaris,$(UNAME)))
  SOLARIS := 1
  LIB_EXT := so
  ifdef $(JAVA_HOME)
    CXXFLAGS += -I"$(JAVA_HOME)/include"
  else
    CXXFLAGS += -I"/System/Library/Frameworks/JavaVM.framework/Headers"
  endif
endif

CXXFLAGS += $(POSITION_INDEPENDENT_FLAG) -I$(EUROPA_HOME)/include/PLASMA
LDFLAGS += $(POSITION_INDEPENDENT_FLAG) -L$(EUROPA_HOME)/lib
LOADLIBS += -lSystem$(BUILD_SUFFIX) \
            -ldl \
            -lANML$(BUILD_SUFFIX) \
            -lAntlr$(BUILD_SUFFIX) \
            -lResource$(BUILD_SUFFIX) \
            -lNDDL$(BUILD_SUFFIX) \
            -lSolvers$(BUILD_SUFFIX) \
            -lRulesEngine$(BUILD_SUFFIX) \
            -lTemporalNetwork$(BUILD_SUFFIX) \
            -lPlanDatabase$(BUILD_SUFFIX) \
            -lConstraintEngine$(BUILD_SUFFIX) \
            -lUtils$(BUILD_SUFFIX) \
            -lTinyXml$(BUILD_SUFFIX)

vpath %.dylib $(EUROPA_HOME)/lib
