cmake_minimum_required(VERSION 3.18)
project(simples CXX)

set(AA60_LIBRARY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../lib)
list(APPEND CMAKE_MODULE_PATH "${AA60_LIBRARY_ROOT_DIR}/cmake")

include(aa60_application)

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} --use_fast_math)

macro(ADD_APP _target)
  aa60_add_application(
    TARGET ${_target}
    SOURCES ${_target}.cpp
    INCLUDE_DIRS
      ${CMAKE_CURRENT_LIST_DIR}
    COMPILE_OPTIONS
      -Wno-ignored-qualifiers
      -Wno-missing-field-initializers
      -Wno-sign-compare
      -Wno-type-limits
      -Wno-unused-parameter
      -Wno-unused-variable
      -Wno-unused-but-set-variable
      "$<$<COMPILE_LANGUAGE:CUDA>:--use_fast_math>"
  )
endmacro()

add_app(main_spu)
add_app(main_spu_3d)
add_app(main_spu++)
add_app(main_glfw)
add_app(main_gsys)
add_app(main_dec)
add_app(main_headless)



      
