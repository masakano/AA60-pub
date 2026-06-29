cmake_minimum_required(VERSION 3.18)

project(sb6 CXX)

set(AA60_LIBRARY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)
list(APPEND CMAKE_MODULE_PATH "${AA60_LIBRARY_ROOT_DIR}/cmake")

include(sources.cmake)
include(aa60_executable)

if(UNIX)
  set(_sb6_link_libraries omp5)
else()
  set(_sb6_link_libraries "")
endif()

aa60_add_executable(
  TARGET main
  SOURCES ${sources}
  INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}
    ${AA60_LIBRARY_ROOT_DIR}
    ${external_dir}
  LINK_LIBRARIES
    spu++
    ${_sb6_link_libraries}
)
      
