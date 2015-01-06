cmake_minimum_required(VERSION 2.8.8)
if((CMAKE_MAJOR_VERSION GREATER 3) OR (CMAKE_MAJOR_VERSION EQUAL 3))
  if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0042 OLD) #TODO: figure out what this means
  endif(COMMAND cmake_policy)
endif((CMAKE_MAJOR_VERSION GREATER 3) OR (CMAKE_MAJOR_VERSION EQUAL 3))

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(Europa2 C CXX Java)
add_definitions(-DTIXML_USE_STL=1)
enable_testing()
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
#Options to support: 
# * optimized/not
option(OPTIMIZE "Build optimized" FALSE)
# * shared/not
option(SHARED "Build shared libraries" TRUE)
set(BUILD_SHARED_LIBS ${SHARED})
# * 64/32-bit
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
 set(SIXTYFOUR TRUE)
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
option(SIXTYFOUR "Build for 64-bit" ${SIXTYFOUR})
# * modules?
# * coverage
option(COVERAGE "Build with coverage info" FALSE)

set(EUROPA_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CppUnit_FIND_QUIETLY TRUE)

if(OPTIMIZE)
  set(EUROPA_SUFFIX "_o")
else()
  set(EUROPA_SUFFIX "_g")
endif(OPTIMIZE)

add_subdirectory(src)
install(EXPORT Europa2 DESTINATION ${EUROPA_ROOT}/dist/europa)
install(DIRECTORY ${EUROPA_ROOT}/bin DESTINATION ${EUROPA_ROOT}/dist/europa)
install(DIRECTORY ${EUROPA_ROOT}/config DESTINATION ${EUROPA_ROOT}/dist/europa)
install(DIRECTORY ${EUROPA_ROOT}/bin/.makeproject DESTINATION ${EUROPA_ROOT}/dist/bin)