cmake_minimum_required(VERSION 3.18)

project(gsys LANGUAGES CXX)

include(sources.cmake)
include(add_aa60_library)
include(dependencies)

aa60_add_library(
  TARGET gsys
  SOURCES ${sources}
  PUBLIC_INCLUDE_DIRS
    ${AA60_LIBRARY_ROOT_DIR}
  PRIVATE_INCLUDE_DIRS
    ${bullet_source_directory}
    ${AA60_LIBRARY_ROOT_DIR}/gsys/shaders
  PUBLIC_LIBS
    aa60_freetype
    ssys
    smath
    spu
    spu++
)

target_compile_options(gsys PRIVATE
#  -Werror
)

set_source_files_properties(
  ${bullet_source_directory}/btBulletDynamicsAll.cpp
  ${bullet_source_directory}/btBulletCollisionAll.cpp
  ${bullet_source_directory}/btLinearMathAll.cpp
  PROPERTIES
    COMPILE_OPTIONS "-w"
)

set_source_files_properties(
  ${CMAKE_CURRENT_LIST_DIR}/src/node/mmd/master.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/node/mmd/actor.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/node/mmd/pmd.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/node/mmd/pmx.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/node/mmd/pmx_file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/node/mmd/pmd_file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/node/mmd/vmd_file.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/painter/mmd/mmd.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/painter/freetype/edtaa3func.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/painter/freetype/ft_font.cpp
  PROPERTIES
    COMPILE_OPTIONS "-Wno-unused-variable;-Wno-sign-compare;-Wno-missing-field-initializers;-Wno-mismatched-tags;-Wno-gnu-anonymous-struct;-Wno-c99-extensions"
)
