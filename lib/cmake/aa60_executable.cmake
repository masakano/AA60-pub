include_guard(GLOBAL)

include(project_options)
include(project_warnings)
include(project_features)
include(git_revision)
include(aa60_libraries)
include(add_clang_tidy_target)

function(aa60_add_executable)
  if(NOT DEFINED AA60_LIBRARY_ROOT_DIR)
    message(FATAL_ERROR "AA60_LIBRARY_ROOT_DIR must be set before calling aa60_add_executable()")
  endif()

  set(options)
  set(one_value_args TARGET)
  set(multi_value_args SOURCES INCLUDE_DIRS LINK_DIRECTORIES LINK_LIBRARIES COMPILE_OPTIONS)
  cmake_parse_arguments(AA60_ADD_EXEC "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  if(NOT AA60_ADD_EXEC_TARGET)
    message(FATAL_ERROR "aa60_add_executable: TARGET is required.")
  endif()

  if(NOT AA60_ADD_EXEC_SOURCES)
    message(FATAL_ERROR "aa60_add_executable: SOURCES is required.")
  endif()

  add_executable(${AA60_ADD_EXEC_TARGET} ${AA60_ADD_EXEC_SOURCES})

  target_include_directories(${AA60_ADD_EXEC_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${AA60_LIBRARY_ROOT_DIR}
    ${AA60_ADD_EXEC_INCLUDE_DIRS}
  )

  target_link_libraries(${AA60_ADD_EXEC_TARGET} PRIVATE
    aa60_project_warnings
    aa60_project_features
    aa60_git_revision
    ${AA60_ADD_EXEC_LINK_LIBRARIES}
  )

  if(AA60_ADD_EXEC_LINK_DIRECTORIES)
    target_link_directories(${AA60_ADD_EXEC_TARGET} PRIVATE
      ${AA60_ADD_EXEC_LINK_DIRECTORIES}
    )
  endif()

  if(AA60_ADD_EXEC_COMPILE_OPTIONS)
    target_compile_options(${AA60_ADD_EXEC_TARGET} PRIVATE
      ${AA60_ADD_EXEC_COMPILE_OPTIONS}
    )
  endif()

  set_target_properties(${AA60_ADD_EXEC_TARGET} PROPERTIES
    BUILD_RPATH ${AA60_LIBRARY_ROOT_DIR}
  )

  install(TARGETS ${AA60_ADD_EXEC_TARGET}
    DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
  )

  aa60_add_clang_tidy_target(${AA60_ADD_EXEC_TARGET} ${AA60_ADD_EXEC_SOURCES})
endfunction()
