cmake_minimum_required(VERSION 3.18)

project(streamer CXX)

get_filename_component(AA60_LIBRARY_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../lib" ABSOLUTE)
get_filename_component(STREAMER_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(STREAMER_BIN_DIR ${STREAMER_ROOT_DIR}/bin)
list(APPEND CMAKE_MODULE_PATH "${AA60_LIBRARY_ROOT_DIR}/cmake")

include(sources.cmake)
include(project_options)
include(project_warnings)
include(project_features)
include(git_revision)
include(aa60_libraries)
include(add_clang_tidy_target)

add_executable(streamer ${sources})

target_include_directories(streamer PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${AA60_LIBRARY_ROOT_DIR}
  ${AA60_LIBRARY_ROOT_DIR}/gsys/shaders
  ${my_include_directories}
)

target_link_libraries(streamer PRIVATE
  aa60_project_warnings
  aa60_project_features
  aa60_git_revision
  gsys
  turbojpeg
)

set_target_properties(streamer PROPERTIES
  BUILD_RPATH ${AA60_LIBRARY_ROOT_DIR}
  INSTALL_RPATH "\$ORIGIN/../../lib"
)

install(TARGETS streamer
  DESTINATION ${STREAMER_BIN_DIR}
)

aa60_add_clang_tidy_target(streamer ${sources})
