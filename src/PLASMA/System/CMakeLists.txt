if(${APPLE})
  set(JNI_INCLUDE_DIRS /System/Library/Frameworks/JavaVM.framework/Versions/A/Headers)
else(${APPLE})
  find_package(JNI REQUIRED)  
endif(${APPLE})
set(ANTLR_FIND_REQUIRED TRUE)
include(FindANTLR)


include(EuropaModule)
set(internal_dependencies NDDL ANML Solvers Resource RulesEngine TemporalNetwork PlanDatabase ConstraintEngine Utils TinyXml)

#TODO: let the user configure which language or languages to build for
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
set(CMAKE_SWIG_OUTDIR "${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine")

set_source_files_properties(base/PSEngine.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(base/PSEngine.i PROPERTIES SWIG_FLAGS "-package;psengine;-includeall")

include_directories(${ANTLR_INCLUDE_DIR})
include_directories(${JNI_INCLUDE_DIRS})
swig_add_module(PSEngine java base/PSEngine.i)
foreach(dep ${internal_dependencies})
  add_common_module_include_dep(PSEngine ${dep})
  target_link_libraries(PSEngine "${dep}${EUROPA_SUFFIX}")
endforeach(dep)
add_common_module_include_dep(PSEngine System)
target_link_libraries(PSEngine "System${EUROPA_SUFFIX}")

set(root_sources "")
get_filename_component(swig_generated_source ${swig_generated_file_fullname} NAME)
set(base_sources EuropaEngine.cc PSEngineImpl.cc)
set(component_sources "")
set(test_sources module-tests.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(System "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")


add_subdirectory(test)