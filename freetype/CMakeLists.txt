cmake_minimum_required(VERSION 3.18)
project(freetype CXX)

set(AA60_LIBRARY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)
list(APPEND CMAKE_MODULE_PATH "${AA60_LIBRARY_ROOT_DIR}/cmake")

include(sources.cmake)
include(aa60_application)

find_library(AA60_FONTCONFIG_LIBRARY NAMES fontconfig REQUIRED)

aa60_add_application(
  TARGET main
  SOURCES ${sources}
  INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}
  LINK_LIBRARIES
    ${AA60_FONTCONFIG_LIBRARY}
  COMPILE_OPTIONS
#    -Werror
)

      
