include_guard(GLOBAL)

include(aa60_executable)

function(aa60_add_application)
  if(NOT DEFINED AA60_LIBRARY_ROOT_DIR)
    message(FATAL_ERROR "AA60_LIBRARY_ROOT_DIR must be set before calling aa60_add_application()")
  endif()

  set(options)
  set(one_value_args TARGET)
  set(multi_value_args SOURCES INCLUDE_DIRS LINK_DIRECTORIES LINK_LIBRARIES COMPILE_OPTIONS)
  cmake_parse_arguments(AA60_ADD_APP "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  if(NOT AA60_ADD_APP_TARGET)
    message(FATAL_ERROR "aa60_add_application: TARGET is required.")
  endif()

  if(NOT AA60_ADD_APP_SOURCES)
    message(FATAL_ERROR "aa60_add_application: SOURCES is required.")
  endif()

  aa60_add_executable(
    TARGET ${AA60_ADD_APP_TARGET}
    SOURCES ${AA60_ADD_APP_SOURCES}
    INCLUDE_DIRS
      ${AA60_LIBRARY_ROOT_DIR}/gsys/shaders
      ${AA60_ADD_APP_INCLUDE_DIRS}
    LINK_DIRECTORIES ${AA60_ADD_APP_LINK_DIRECTORIES}
    LINK_LIBRARIES
      gsys
      ${AA60_ADD_APP_LINK_LIBRARIES}
    COMPILE_OPTIONS ${AA60_ADD_APP_COMPILE_OPTIONS}
  )
endfunction()
