cmake_minimum_required(VERSION 3.18)

project(lib LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(AA60_LIBRARY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(AA60_LIBRARY_INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR})

include(project_options)
include(project_warnings)
include(project_features)
include(git_revision)
include(add_checkin_target)

aa60_add_checkin_target(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(ssys)
add_subdirectory(smath)
add_subdirectory(spu)
add_subdirectory(spu++)
add_subdirectory(gsys)

