cmake_minimum_required(VERSION 3.18)

project(smath LANGUAGES CXX)

include(sources.cmake)
include(add_aa60_library)

aa60_add_library(
  TARGET smath
  SOURCES ${sources}
  PUBLIC_INCLUDE_DIRS
    ${AA60_LIBRARY_ROOT_DIR}
  PRIVATE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_LIBS
    ssys
)
