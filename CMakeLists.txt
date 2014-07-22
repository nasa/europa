cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0042 OLD) #TODO: figure out what this means
endif(COMMAND cmake_policy)
add_definitions(-DTIXML_USE_STL=1)
enable_testing()
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
option(SHARED "Build shared libraries" TRUE)
set(BUILD_SHARED_LIBS ${SHARED})
set(EUROPA_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)