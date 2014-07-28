cmake_minimum_required(VERSION 2.8.8)
project(Light)
include($ENV{EUROPA_HOME}/Europa2.cmake)

if(OPTIMIZE)
  set(EUROPA_SUFFIX "_o")
else()
  set(EUROPA_SUFFIX "_g")
endif(OPTIMIZE)

#project-swig
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_OUTDIR ${CMAKE_SOURCE_DIR})
set_source_files_properties(Light.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(Light.i PROPERTIES SWIG_FLAGS "-package;psengine;-includeall")
swig_add_module(Light java Light.i)

#project-lib
add_library(Light${EUROPA_SUFFIX} ${swig_generated_file_fullname} LightCustomCode.cc ModuleLight.cc)

#project-rt
add_executable(Light-planner_${EUROPA_SUFFIX}_rt Light-Main.cc LightCustomCode.cc ModuleLight.cc)

#run-project
add_custom_target(run-project
  DEPENDS Light-planner_${EUROPA_SUFFIX}_rt
  COMMAND ./Light-planner_${EUROPA_SUFFIX}_rt > RUN_Light-planner_${EUROPA_SUFFIX}_rt.Light-initial-state.nddl.PlannerConfig.xml.output)

#project_jar
find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)

add_jar(LightJar
  SOURCES
  #??
  OUTPUT_NAME Light.jar
  )

add_dependencies(LightJar Light${EUROPA_SUFFIX})

