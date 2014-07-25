cmake_minimum_required(VERSION 2.8.8)
if((CMAKE_MAJOR_VERSION GREATER 3) OR (CMAKE_MAJOR_VERSION EQUAL 3))
  if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0042 OLD) #TODO: figure out what this means
  endif(COMMAND cmake_policy)
endif((CMAKE_MAJOR_VERSION GREATER 3) OR (CMAKE_MAJOR_VERSION EQUAL 3))

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(Europa2 CXX Java)
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
# * modules?

set(EUROPA_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(CppUnit_FIND_QUIETLY TRUE)
add_subdirectory(src)
install(EXPORT Europa2 DESTINATION ${EUROPA_ROOT}/dist/europa)
