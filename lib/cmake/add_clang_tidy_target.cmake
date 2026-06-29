function(aa60_add_clang_tidy_target target_name)
  if(NOT DEFINED AA60_LIBRARY_ROOT_DIR)
    message(FATAL_ERROR "AA60_LIBRARY_ROOT_DIR must be set before calling aa60_add_clang_tidy_target()")
  endif()

  find_program(CLANG_TIDY_EXECUTABLE clang-tidy)
  if(NOT CLANG_TIDY_EXECUTABLE)
    message(WARNING "clang-tidy was not found; clang-tidy target will not be available.")
    return()
  endif()

  get_filename_component(aa60_project_root "${AA60_LIBRARY_ROOT_DIR}/.." ABSOLUTE)

  set(clang_tidy_sources)
  foreach(source IN LISTS ARGN)
    if(source MATCHES "\\.cpp$")
      if(IS_ABSOLUTE "${source}")
        list(APPEND clang_tidy_sources "${source}")
      else()
        list(APPEND clang_tidy_sources "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
      endif()
    endif()
  endforeach()

  if(NOT clang_tidy_sources)
    return()
  endif()

  if(NOT TARGET clang-tidy)
    add_custom_target(clang-tidy)
  endif()

  string(MAKE_C_IDENTIFIER "${target_name}" clang_tidy_target_suffix)
  set(clang_tidy_target "clang-tidy-${clang_tidy_target_suffix}")

  if(TARGET ${clang_tidy_target})
    return()
  endif()

  add_custom_target(${clang_tidy_target}
    COMMAND
      /bin/bash -o pipefail -c
      "'${CLANG_TIDY_EXECUTABLE}' --quiet -p '${CMAKE_BINARY_DIR}' \"$@\" 2>&1 | sed -E '/^[0-9]+ warnings generated\\.$/d;/^Suppressed [0-9]+ warnings /d;/^Use -header-filter=/d'"
      clang-tidy
      ${clang_tidy_sources}
    WORKING_DIRECTORY ${aa60_project_root}
    COMMENT "Running clang-tidy for ${target_name}"
    VERBATIM
  )

  add_dependencies(clang-tidy ${clang_tidy_target})
endfunction()
