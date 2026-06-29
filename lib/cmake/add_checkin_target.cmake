function(aa60_add_checkin_target)
  if(TARGET checkin)
    return()
  endif()

  if(ARGC GREATER 0)
    set(checkin_directory ${ARGV0})
  else()
    set(checkin_directory ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  add_custom_target(checkin COMMAND
    find ${checkin_directory}
    "\\(" -name win64 -o -name external -o -name build "\\)" -prune -o
    "\\("
      -name CMakeLists.txt -o
      -name *.cpp -o
      -name *.h -o
      -name *.hpp -o
      -name *.ipp -o
      -name *.conf -o
      -name *.json -o
      -name *.us -o
      -name *.cu
    "\\)" -print -exec ci -l -q -min_debug "{}" "\\;"
  )
endfunction()
