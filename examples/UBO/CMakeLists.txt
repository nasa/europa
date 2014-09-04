cmake_minimum_required(VERSION 2.8.8)
set(PROJECT UBO)

project(${PROJECT})
include($ENV{EUROPA_HOME}/Europa2.cmake)

if(${APPLE})
  set(JNI_INCLUDE_DIRS /System/Library/Frameworks/JavaVM.framework/Versions/A/Headers)
else(${APPLE})
  find_package(JNI REQUIRED)  
endif(${APPLE})
include_directories(${JNI_INCLUDE_DIRS})

if(OPTIMIZE)
  set(EUROPA_SUFFIX "_o")
else()
  set(EUROPA_SUFFIX "_g")
endif(OPTIMIZE)

file(GLOB children RELATIVE $ENV{EUROPA_HOME} $ENV{EUROPA_HOME}/*)
foreach(child ${children})
  if(IS_DIRECTORY $ENV{EUROPA_HOME}/${child})
    include_directories($ENV{EUROPA_HOME}/${child})
    include_directories($ENV{EUROPA_HOME}/${child}/base)
    include_directories($ENV{EUROPA_HOME}/${child}/component)
  endif(IS_DIRECTORY $ENV{EUROPA_HOME}/${child})
endforeach(child)

#project-swig
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_OUTDIR ${CMAKE_SOURCE_DIR})
set_source_files_properties(${PROJECT}.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${PROJECT}.i PROPERTIES SWIG_FLAGS "-package;psengine;-includeall")
swig_add_module(${PROJECT} java ${PROJECT}.i)


#project-lib
add_library(${PROJECT}${EUROPA_SUFFIX} ${swig_generated_file_fullname} ${PROJECT}CustomCode.cc Module${PROJECT}.cc)


#project-rt
add_executable(${PROJECT}-planner${EUROPA_SUFFIX}_rt ${PROJECT}-Main.cc ${PROJECT}CustomCode.cc Module${PROJECT}.cc)
target_link_libraries(${PROJECT}-planner${EUROPA_SUFFIX}_rt System${EUROPA_SUFFIX} Resource${EUROPA_SUFFIX} Solvers${EUROPA_SUFFIX} NDDL${EUROPA_SUFFIX} TemporalNetwork${EUROPA_SUFFIX} RulesEngine${EUROPA_SUFFIX} ConstraintEngine${EUROPA_SUFFIX} Utils${EUROPA_SUFFIX} TinyXml${EUROPA_SUFFIX})

#run-project
add_custom_target(run-project
  DEPENDS ${PROJECT}-planner${EUROPA_SUFFIX}_rt
  COMMAND ./${PROJECT}-planner${EUROPA_SUFFIX}_rt ${PROJECT}-initial-state.nddl PlannerConfig.xml > RUN_${PROJECT}-planner${EUROPA_SUFFIX}_rt.${PROJECT}-initial-state.nddl.PlannerConfig.xml.output)

#project_jar
find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)

#find_jar(europa_jar PSEngine.jar PATHS $ENV{EUROPA_HOME})
add_jar(${PROJECT}Jar
  SOURCES
  ${PROJECT}.java
  ${PROJECT}JNI.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/ExhaustiveSolver.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/HybridSolver.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/IFlatIRelaxSolver.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/Main.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/PSResourceWrapper.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/PlanHistoryEntry.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/Precedence.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/RCPSPResource.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/RCPSPSolver.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/RCPSPSolverBase.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/RCPSPUtil.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/Resource.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/ResourceBase.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/ResourceProfile.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/ResourceViolationInfo.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/SolDistExhaustiveSolver.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/UBOSolverListener.java
  ${CMAKE_CURRENT_SOURCE_DIR}/java/UBO/Main.java
  INCLUDE_JARS 
  $ENV{EUROPA_HOME}/PSEngine.jar
  $ENV{EUROPA_HOME}/PSUISwing.jar
  OUTPUT_NAME ${PROJECT}
  ENTRY_POINT UBO/Main
  )

add_dependencies(${PROJECT}Jar ${PROJECT}${EUROPA_SUFFIX})

