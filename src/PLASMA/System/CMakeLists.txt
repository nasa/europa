#14 -> Yosemite
#13 -> Mavericks
#12 -> Mountain Lion
#11 -> Lion
#10 -> Snow Leopard
if(${APPLE})
  exec_program(uname ARGS -r  OUTPUT_VARIABLE DARWIN_VERSION)

  if(DARWIN_VERSION VERSION_GREATER 14)
    message(STATUS "Shooting in the dark about where jni.h is")
    set(JNI_INCLUDE_DIRS $ENV{JAVA_HOME}/include)
  elseif(DARWIN_VERSION VERSION_EQUAL 14)
    message(STATUS "Setting Yosemite include")
    set(JNI_INCLUDE_DIRS /Library/Java/JavaVirtualMachines/jdk1.7.0_45.jdk/Contents/Home/include)
  else(DARWIN_VERSION)
    message(STATUS "Setting pre-Yosemite include")
    set(JNI_INCLUDE_DIRS /System/Library/Frameworks/JavaVM.framework/Versions/A/Headers)
  endif(DARWIN_VERSION VERSION_GREATER 14)
else(${APPLE})
  find_package(JNI REQUIRED)  
endif(${APPLE})
set(ANTLR_FIND_REQUIRED TRUE)
include(FindANTLR)


include(EuropaModule)
#Currently, this has to be the whole set of dependencies because of the loop later.
#I'm not sure why it is that System_FULL_DEPENDENCIES isn't visible in this scope
set(internal_dependencies NDDL ANML Solvers Resource RulesEngine TemporalNetwork PlanDatabase ConstraintEngine Utils TinyXml)

set(root_sources "")
set(base_sources EuropaEngine.cc PSEngineImpl.cc)
set(component_sources "")
set(test_sources module-tests.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(System "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/base)
#TODO: let the user configure which language or languages to build for
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/psengine")

set_source_files_properties(base/PSEngine.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(base/PSEngine.i PROPERTIES SWIG_FLAGS "-package;psengine;-includeall")
find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)

if(NOT EUROPA_ROOT)
message(FATAL_ERROR "EUROPA_ROOT not set")
endif(NOT EUROPA_ROOT)
message(STATUS "EUROPA_ROOT: ${EUROPA_ROOT}")

set(SWIG_GENERATED_JAVA
  psengine/PSConstraint.java
  psengine/PSConstraintEngineListener.java
  psengine/PSConstraintList.java
  psengine/PSDataType.java
  psengine/PSDataTypeList.java
  psengine/PSEngine.java
  psengine/PSEngineInterface.java
  psengine/PSEngineInterfaceJNI.java
  psengine/PSEntity.java
  psengine/PSException.java
  psengine/PSIntList.java
  psengine/PSLanguageException.java
  psengine/PSLanguageExceptionList.java
  psengine/PSObject.java
  psengine/PSObjectList.java
  psengine/PSObjectType.java
  psengine/PSObjectTypeList.java
  psengine/PSPlanDatabaseClient.java
  psengine/PSPlanDatabaseListener.java
  psengine/PSResource.java
  psengine/PSResourceProfile.java
  psengine/PSSchema.java
  psengine/PSSolver.java
  psengine/PSStringList.java
  psengine/PSTimePointList.java
  psengine/PSToken.java
  psengine/PSTokenList.java
  psengine/PSTokenState.java
  psengine/PSTokenType.java
  psengine/PSTokenTypeList.java
  psengine/PSValueList.java
  psengine/PSVarType.java
  psengine/PSVarValue.java
  psengine/PSVariable.java
  psengine/PSVariableList.java
  psengine/StringErrorStream.java
  psengine/EngineConfig.java
)

set(SWIG_GENERATED_JAVA_FULL_PATH "")
foreach(f ${SWIG_GENERATED_JAVA})
  list(APPEND SWIG_GENERATED_JAVA_FULL_PATH ${CMAKE_CURRENT_BINARY_DIR}/${f})
endforeach(f)

add_jar(PSEngineGenerated
  SOURCES ${SWIG_GENERATED_JAVA_FULL_PATH})
  #OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${ANTLR_INCLUDE_DIR})
include_directories(${JNI_INCLUDE_DIRS})
set(PSEngine_target PSEngine${EUROPA_SUFFIX})
# file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/base)
swig_add_module(${PSEngine_target} java base/PSEngine.i)
# add_dependencies(${PSEngine_target} ${CMAKE_CURRENT_BINARY_DIR}/base)
#add_custom_command(TARGET ${PSEngine_target} PRE_LINK
#  COMMAND mkdir base)
  
#add_dependencies(${PSEngine_target} base)

foreach(dep ${internal_dependencies})
  add_common_module_include_dep(${PSEngine_target} ${dep})
  target_link_libraries(${PSEngine_target} "${dep}${EUROPA_SUFFIX}")
endforeach(dep)
add_common_module_include_dep(${PSEngine_target} System)
target_link_libraries(${PSEngine_target} "System${EUROPA_SUFFIX}")
add_dependencies(PSEngineGenerated ${PSEngine_target})
install(TARGETS ${PSEngine_target}
  EXPORT Europa2
  DESTINATION ${EUROPA_ROOT}/dist/europa)

add_subdirectory(test)
