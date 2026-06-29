cmake_minimum_required(VERSION 3.18)

project(spu LANGUAGES CXX CUDA)

include(sources.cmake)
include(add_aa60_library)
include(dependencies)

aa60_add_library(
  TARGET spu
  SOURCES ${sources}
  PUBLIC_INCLUDE_DIRS
    ${AA60_LIBRARY_ROOT_DIR}
  PRIVATE_INCLUDE_DIRS
    ${spu_private_include_directories}
  PUBLIC_LIBS
    ssys
)

set_target_properties(spu PROPERTIES
  CUDA_STANDARD 20
  CUDA_STANDARD_REQUIRED ON
)

set(nvcodec_sdk_sources
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/NvDecoder/NvDecoder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/NvEncoder/NvEncoder.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/NvEncoder/NvEncoderCuda.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/Utils/Logger.cpp
)

set_source_files_properties(
  ${nvcodec_sdk_sources}
  PROPERTIES
    COMPILE_OPTIONS -w
)

aa60_link_spu_dependencies(spu)

set_source_files_properties(
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/color_space.cu
  PROPERTIES
    COMPILE_OPTIONS "-Xcompiler=-w"
)

target_link_options(spu
  PRIVATE -Wl,--version-script,${CMAKE_CURRENT_LIST_DIR}/src/libspu.map)
