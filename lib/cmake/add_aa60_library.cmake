include_guard(GLOBAL)

include(add_clang_tidy_target)

function(aa60_add_library)
  set(options)
  set(one_value_args TARGET TYPE OUTPUT_NAME)
  set(multi_value_args
    SOURCES
    PUBLIC_INCLUDE_DIRS
    PRIVATE_INCLUDE_DIRS
    PUBLIC_LIBS
    PRIVATE_LIBS
  )
  cmake_parse_arguments(AA60 "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  if(NOT AA60_TARGET)
    message(FATAL_ERROR "aa60_add_library: TARGET is required")
  endif()

  if(NOT AA60_TYPE)
    set(AA60_TYPE SHARED)
  endif()

  add_library(${AA60_TARGET} ${AA60_TYPE} ${AA60_SOURCES})

  if(AA60_OUTPUT_NAME)
    set_target_properties(${AA60_TARGET} PROPERTIES OUTPUT_NAME ${AA60_OUTPUT_NAME})
  endif()

  target_link_libraries(${AA60_TARGET}
    PRIVATE
      aa60_project_warnings
      aa60_project_features
      aa60_git_revision
      ${AA60_PRIVATE_LIBS}
    PUBLIC
      ${AA60_PUBLIC_LIBS}
  )

  if(AA60_PUBLIC_INCLUDE_DIRS)
    target_include_directories(${AA60_TARGET} PUBLIC ${AA60_PUBLIC_INCLUDE_DIRS})
  endif()

  if(AA60_PRIVATE_INCLUDE_DIRS)
    target_include_directories(${AA60_TARGET} PRIVATE ${AA60_PRIVATE_INCLUDE_DIRS})
  endif()

  if(DEFINED AA60_LIBRARY_INSTALL_DIR)
    install(TARGETS ${AA60_TARGET} DESTINATION ${AA60_LIBRARY_INSTALL_DIR})
  else()
    install(TARGETS ${AA60_TARGET} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  aa60_add_clang_tidy_target(${AA60_TARGET} ${AA60_SOURCES})
endfunction()
