if(${APPLE})
  set(JNI_INCLUDE_DIRS /System/Library/Frameworks/JavaVM.framework/Versions/A/Headers)
else(${APPLE})
  find_package(JNI REQUIRED)  
endif(${APPLE})


include(EuropaModule)
set(internal_dependencies NDDL ANML Solvers Resource RulesEngine TemporalNetwork PlanDatabase ConstraintEngine Utils TinyXml)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
set(CMAKE_SWIG_FLAGS "")
#set(CMAKE_SWIG_OUTDIR "${CMAKE_SOURCE_DIR}/src/Java/PSEngine/generated/psengine")
set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_SOURCE_DIR}/base")

set_source_files_properties(base/PSEngine.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(base/PSEngine.i PROPERTIES SWIG_FLAGS "-includeall")
#TEMPORARY

#set(JNI_HOME /System/Library/Frameworks/JavaVM.framework/Versions/A/Headers)
include_directories(${JNI_INCLUDE_DIRS})
swig_add_module(PSEngine java base/PSEngine.i)
foreach(dep ${internal_dependencies})
  add_common_module_include_dep(PSEngine ${dep})
  target_link_libraries(PSEngine "${dep}${EUROPA_SUFFIX}")
endforeach(dep)
target_link_libraries(PSEngine "System${EUROPA_SUFFIX}")
#swig_link_libraries(PSEngine)

set(root_sources "")
get_filename_component(swig_generated_source ${swig_generated_file_fullname} NAME)
#set(base_sources EuropaEngine.cc PSEngineImpl.cc ${swig_generated_source})
set(base_sources EuropaEngine.cc PSEngineImpl.cc)
set(component_sources "")
set(test_sources module-tests.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(System "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")


add_subdirectory(test)