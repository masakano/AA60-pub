cmake_minimum_required(VERSION 3.18)

project(glsamples CXX)

set(AA60_LIBRARY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)
list(APPEND CMAKE_MODULE_PATH "${AA60_LIBRARY_ROOT_DIR}/cmake")

include(sources.cmake)
include(aa60_executable)

aa60_add_executable(
  TARGET main
  SOURCES ${sources}
  LINK_LIBRARIES
    spu++
  COMPILE_OPTIONS
#     -Werror
)
