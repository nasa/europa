include(FindCppUnit)

function(append_target_property target propname value)
  get_target_property(current_val ${target} ${propname})
  set_target_properties(${target} PROPERTIES ${propname} "${current_val}${value}")
endfunction(append_target_property)

function(prepend_all prefix input output)
  set(temp "")
  foreach(entry ${input})
    list(APPEND temp ${prefix}${entry})
  endforeach(entry)
  set(${output} ${temp} PARENT_SCOPE)
endfunction(prepend_all)

function(common_module_prepends base_sources component_sources test_sources
    base_dest component_dest test_dest)
  prepend_all(base/ "${base_sources}" base_int)
  prepend_all(component/ "${component_sources}" component_int)
  prepend_all(test/ "${test_sources}" test_int)
  set(${base_dest} ${base_int} PARENT_SCOPE)
  set(${component_dest} ${component_int} PARENT_SCOPE)
  set(${test_dest} ${test_int} PARENT_SCOPE)
endfunction(common_module_prepends)


function(echo_target_property tgt prop)
  # v for value, d for defined, s for set
  get_property(v TARGET ${tgt} PROPERTY ${prop})
  get_property(d TARGET ${tgt} PROPERTY ${prop} DEFINED)
  get_property(s TARGET ${tgt} PROPERTY ${prop} SET)
 
  # only produce output for values that are set
  if(s)
    message("tgt='${tgt}' prop='${prop}'")
    message("  value='${v}'")
    message("  defined='${d}'")
    message("  set='${s}'")
    message("")
  endif()
endfunction()
 
function(echo_target tgt)
  if(NOT TARGET ${tgt})
    message("There is no target named '${tgt}'")
    return()
  endif()
 
  set(props
DEBUG_OUTPUT_NAME
DEBUG_POSTFIX
RELEASE_OUTPUT_NAME
RELEASE_POSTFIX
ARCHIVE_OUTPUT_DIRECTORY
ARCHIVE_OUTPUT_DIRECTORY_DEBUG
ARCHIVE_OUTPUT_DIRECTORY_RELEASE
ARCHIVE_OUTPUT_NAME
ARCHIVE_OUTPUT_NAME_DEBUG
ARCHIVE_OUTPUT_NAME_RELEASE
AUTOMOC
AUTOMOC_MOC_OPTIONS
BUILD_WITH_INSTALL_RPATH
BUNDLE
BUNDLE_EXTENSION
COMPILE_DEFINITIONS
COMPILE_DEFINITIONS_DEBUG
COMPILE_DEFINITIONS_RELEASE
COMPILE_FLAGS
DEBUG_POSTFIX
RELEASE_POSTFIX
DEFINE_SYMBOL
ENABLE_EXPORTS
EXCLUDE_FROM_ALL
EchoString
FOLDER
FRAMEWORK
Fortran_FORMAT
Fortran_MODULE_DIRECTORY
GENERATOR_FILE_NAME
GNUtoMS
HAS_CXX
IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
IMPORTED
IMPORTED_CONFIGURATIONS
IMPORTED_IMPLIB
IMPORTED_IMPLIB_DEBUG
IMPORTED_IMPLIB_RELEASE
IMPORTED_LINK_DEPENDENT_LIBRARIES
IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG
IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE
IMPORTED_LINK_INTERFACE_LANGUAGES
IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG
IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE
IMPORTED_LINK_INTERFACE_LIBRARIES
IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG
IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE
IMPORTED_LINK_INTERFACE_MULTIPLICITY
IMPORTED_LINK_INTERFACE_MULTIPLICITY_DEBUG
IMPORTED_LINK_INTERFACE_MULTIPLICITY_RELEASE
IMPORTED_LOCATION
IMPORTED_LOCATION_DEBUG
IMPORTED_LOCATION_RELEASE
IMPORTED_NO_SONAME
IMPORTED_NO_SONAME_DEBUG
IMPORTED_NO_SONAME_RELEASE
IMPORTED_SONAME
IMPORTED_SONAME_DEBUG
IMPORTED_SONAME_RELEASE
IMPORT_PREFIX
IMPORT_SUFFIX
INCLUDE_DIRECTORIES
INSTALL_NAME_DIR
INSTALL_RPATH
INSTALL_RPATH_USE_LINK_PATH
INTERPROCEDURAL_OPTIMIZATION
INTERPROCEDURAL_OPTIMIZATION_DEBUG
INTERPROCEDURAL_OPTIMIZATION_RELEASE
LABELS
LIBRARY_OUTPUT_DIRECTORY
LIBRARY_OUTPUT_DIRECTORY_DEBUG
LIBRARY_OUTPUT_DIRECTORY_RELEASE
LIBRARY_OUTPUT_NAME
LIBRARY_OUTPUT_NAME_DEBUG
LIBRARY_OUTPUT_NAME_RELEASE
LINKER_LANGUAGE
LINK_DEPENDS
LINK_FLAGS
LINK_FLAGS_DEBUG
LINK_FLAGS_RELEASE
LINK_INTERFACE_LIBRARIES
LINK_INTERFACE_LIBRARIES_DEBUG
LINK_INTERFACE_LIBRARIES_RELEASE
LINK_INTERFACE_MULTIPLICITY
LINK_INTERFACE_MULTIPLICITY_DEBUG
LINK_INTERFACE_MULTIPLICITY_RELEASE
LINK_SEARCH_END_STATIC
LINK_SEARCH_START_STATIC
LOCATION
LOCATION_DEBUG
LOCATION_RELEASE
MACOSX_BUNDLE
MACOSX_BUNDLE_INFO_PLIST
MACOSX_FRAMEWORK_INFO_PLIST
MAP_IMPORTED_CONFIG_DEBUG
MAP_IMPORTED_CONFIG_RELEASE
OSX_ARCHITECTURES
OSX_ARCHITECTURES_DEBUG
OSX_ARCHITECTURES_RELEASE
OUTPUT_NAME
OUTPUT_NAME_DEBUG
OUTPUT_NAME_RELEASE
POST_INSTALL_SCRIPT
PREFIX
PRE_INSTALL_SCRIPT
PRIVATE_HEADER
PROJECT_LABEL
PUBLIC_HEADER
RESOURCE
RULE_LAUNCH_COMPILE
RULE_LAUNCH_CUSTOM
RULE_LAUNCH_LINK
RUNTIME_OUTPUT_DIRECTORY
RUNTIME_OUTPUT_DIRECTORY_DEBUG
RUNTIME_OUTPUT_DIRECTORY_RELEASE
RUNTIME_OUTPUT_NAME
RUNTIME_OUTPUT_NAME_DEBUG
RUNTIME_OUTPUT_NAME_RELEASE
SKIP_BUILD_RPATH
SOURCES
SOVERSION
STATIC_LIBRARY_FLAGS
STATIC_LIBRARY_FLAGS_DEBUG
STATIC_LIBRARY_FLAGS_RELEASE
SUFFIX
TYPE
VERSION
VS_DOTNET_REFERENCES
VS_GLOBAL_WHATEVER
VS_GLOBAL_KEYWORD
VS_GLOBAL_PROJECT_TYPES
VS_KEYWORD
VS_SCC_AUXPATH
VS_SCC_LOCALPATH
VS_SCC_PROJECTNAME
VS_SCC_PROVIDER
VS_WINRT_EXTENSIONS
VS_WINRT_REFERENCES
WIN32_EXECUTABLE
XCODE_ATTRIBUTE_WHATEVER
)
 
  message("======================== ${tgt} ========================")
  foreach(p ${props})
    echo_target_property("${t}" "${p}")
  endforeach()
  message("")
endfunction()


#TODO: change this to "internal", to distinguish internal modules from external ones
function(add_common_module_include_dep target dep)
  append_target_property(${target} INCLUDE_DIRECTORIES ";${PLASMA_SRC_DIR}/${dep}")
  append_target_property(${target} INCLUDE_DIRECTORIES ";${PLASMA_SRC_DIR}/${dep}/base")
  append_target_property(${target} INCLUDE_DIRECTORIES ";${PLASMA_SRC_DIR}/${dep}/component")
endfunction(add_common_module_include_dep)

function(add_common_module_deps target deps)
  foreach(dep ${deps})
    add_common_module_include_dep(${target} ${dep})
    target_link_libraries(${target} "${dep}${EUROPA_SUFFIX}")
  endforeach(dep)
endfunction(add_common_module_deps target deps)

#NOTE:  This adds dependencies in every direction, so including something
#from component/ in base/ will still work.  Need to think about how to 
#deal with that
function(add_common_local_include_deps target)
  append_target_property(${target} INCLUDE_DIRECTORIES ";${CMAKE_CURRENT_SOURCE_DIR}/")
  append_target_property(${target} INCLUDE_DIRECTORIES ";${CMAKE_CURRENT_SOURCE_DIR}/base")
  append_target_property(${target} INCLUDE_DIRECTORIES ";${CMAKE_CURRENT_SOURCE_DIR}/component")
endfunction(add_common_local_include_deps)

function(declare_module name root_srcs base_srcs component_srcs test_srcs module_dependencies module_components)
  set(libname "${name}${EUROPA_SUFFIX}")
  set(testname ${name}-test${EUROPA_SUFFIX})

  set(full_dependencies ${module_dependencies})
  foreach(mod ${module_dependencies})
    list(APPEND full_dependencies ${${mod}_FULL_DEPENDENCIES})
  endforeach(mod)
  if(full_dependencies)
    list(REMOVE_DUPLICATES full_dependencies)
  endif(full_dependencies)
  set(${name}_FULL_DEPENDENCIES ${full_dependencies} PARENT_SCOPE)
  

  add_library(${libname} ${root_srcs} ${base_srcs} ${component_srcs})
  add_common_local_include_deps(${libname})
  foreach(dep ${module_components})
    append_target_property(${libname} INCLUDE_DIRECTORIES ";${CMAKE_CURRENT_SOURCE_DIR}/component/${dep}")
  endforeach(dep)

  #add_common_module_deps(${libname} "${module_dependencies}")
  add_common_module_deps(${libname} "${full_dependencies}")

  #why did I end up having to do all this?
  if(test_srcs OR (NOT ("${test_srcs}" STREQUAL "" OR test_srcs MATCHES "^$")))
    add_executable(${testname} ${test_srcs})
    target_link_libraries(${testname} ${libname})
    append_target_property(${testname} INCLUDE_DIRECTORIES ";${CppUnit_INCLUDE_DIRS}")
    add_common_local_include_deps(${testname})
    
    #add_common_module_deps(${testname} "${module_dependencies}")
    add_common_module_deps(${testname} "${full_dependencies}")
    foreach(dep ${module_components})
      append_target_property(${testname} INCLUDE_DIRECTORIES ";${CMAKE_CURRENT_SOURCE_DIR}/component/${dep}")
    endforeach(dep)
    
    target_link_libraries(${testname} ${CppUnit_LIBRARIES})
    add_test(NAME ${name}Test
      COMMAND ${testname})

    set(${name}_TEST ${testname} PARENT_SCOPE)
  endif()

  #install EXPORT?
  install(
    TARGETS ${libname} 
    EXPORT Europa2
    DESTINATION ${EUROPA_ROOT}/dist/europa 
    #TODO: figure out why this isn't working
    #INCLUDES DESTINATION ${EUROPA_ROOT}/dist/europa/${name}
    )
  #file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR} *.hh)
  #message(STATUS "Headers: ${headers}")
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${EUROPA_ROOT}/dist/europa
    FILES_MATCHING PATTERN "*.hh" PATTERN "*.h")
endfunction(declare_module)

function(list_jar jar contents)
  set(jar_files "")
  set(jar_files_ndir "")
  set(jar_files_str "")
  # message(STATUS ${Java_JAR_EXECUTABLE} tf ${jar})
  execute_process(
    COMMAND ${Java_JAR_EXECUTABLE} tf ${jar}
    OUTPUT_VARIABLE jar_files_str
    RESULT_VARIABLE rv)
  string(REPLACE "\n" ";" jar_files_ndir ${jar_files_str})

  foreach(fname ${jar_files_ndir})
    get_filename_component(n ${fname} NAME)
    # message(STATUS ${n})
    if(NOT "${n}" STREQUAL "")
      # message(STATUS ${n})
      list(APPEND jar_files ${fname})
    endif(NOT "${n}" STREQUAL "")
  endforeach(fname)
  list(REMOVE_ITEM jar_files "META-INF/MANIFEST.MF")
  set(${contents} ${jar_files} PARENT_SCOPE)

  # list(REMOVE_ITEM jar_files_ndir "META-INF/")
  # list(REMOVE_ITEM jar_files_ndir "META-INF/MANIFEST.MF")
  # set(${contents} ${jar_files_ndir} PARENT_SCOPE)
endfunction(list_jar)

function(unjar target jar dest_dir contents)
  set(jar_contents "")

  list_jar(${jar} jar_contents)

  # message(STATUS ${jar_contents})
  set(${contents} ${jar_contents} PARENT_SCOPE)
  add_custom_target(${target})
  add_custom_command(
    TARGET ${target}
    COMMAND ${Java_JAR_EXECUTABLE}
    xf ${jar}
    WORKING_DIRECTORY ${dest_dir}
    )

endfunction(unjar)