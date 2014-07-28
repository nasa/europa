if(${APPLE})
  set(JNI_INCLUDE_DIRS /System/Library/Frameworks/JavaVM.framework/Versions/A/Headers)
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


#TODO: let the user configure which language or languages to build for
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
set(CMAKE_SWIG_OUTDIR "${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine")

set_source_files_properties(base/PSEngine.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(base/PSEngine.i PROPERTIES SWIG_FLAGS "-package;psengine;-includeall")
find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)

add_jar(PSEngineGenerated
  SOURCES
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSConstraint.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSConstraintEngineListener.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSConstraintList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSDataType.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSDataTypeList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSEngine.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSEngineInterface.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSEngineInterfaceJNI.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSEntity.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSException.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSIntList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSLanguageException.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSLanguageExceptionList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSObject.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSObjectList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSObjectType.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSObjectTypeList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSPlanDatabaseClient.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSPlanDatabaseListener.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSResource.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSResourceProfile.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSSchema.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSSolver.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSStringList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSTimePointList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSToken.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSTokenList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSTokenState.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSTokenType.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSTokenTypeList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSValueList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSVarType.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSVarValue.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSVariable.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/PSVariableList.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/StringErrorStream.java
  ${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine/EngineConfig.java
  OUTPUT_DIR ${CMAKE_SOURCE_DIR}/src/java/PSEngine)

include_directories(${ANTLR_INCLUDE_DIR})
include_directories(${JNI_INCLUDE_DIRS})
swig_add_module(PSEngine java base/PSEngine.i)
message(STATUS "!! ${System_FULL_DEPENDENCIES}")
foreach(dep ${internal_dependencies})
  add_common_module_include_dep(PSEngine ${dep})
  target_link_libraries(PSEngine "${dep}${EUROPA_SUFFIX}")
endforeach(dep)
add_common_module_include_dep(PSEngine System)
target_link_libraries(PSEngine "System${EUROPA_SUFFIX}")
add_dependencies(PSEngineGenerated PSEngine)

add_subdirectory(test)