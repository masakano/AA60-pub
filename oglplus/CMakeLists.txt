cmake_minimum_required(VERSION 3.18)

project(oglplus CXX)

set(AA60_LIBRARY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)
list(APPEND CMAKE_MODULE_PATH "${AA60_LIBRARY_ROOT_DIR}/cmake")

include(sources.cmake)
include(aa60_executable)

aa60_add_executable(
  TARGET main
  SOURCES ${sources}
  INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/implement
    ${AA60_LIBRARY_ROOT_DIR}/external
    ${AA60_LIBRARY_ROOT_DIR}
  LINK_LIBRARIES
    spu++
)
