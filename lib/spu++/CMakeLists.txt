cmake_minimum_required(VERSION 3.18)

project(spu++ LANGUAGES CXX)

include(sources.cmake)
include(add_aa60_library)

aa60_add_library(
  TARGET spu++
  SOURCES ${sources}
  PUBLIC_INCLUDE_DIRS
    ${AA60_LIBRARY_ROOT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${external_dir}/imgui
    ${external_dir}/ImGuizmo/src
  PUBLIC_LIBS
    ssys
    smath
    spu
)
