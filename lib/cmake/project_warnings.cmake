if(DEFINED AA60_PROJECT_WARNINGS_INCLUDED)
  return()
endif()
set(AA60_PROJECT_WARNINGS_INCLUDED ON)

add_library(aa60_project_warnings INTERFACE)

if(MSVC)
  target_compile_options(aa60_project_warnings INTERFACE
    /utf-8
    /EHsc
    /W4
  )
else()
  target_compile_options(aa60_project_warnings INTERFACE
    -Wall
    -Wextra
    -Wno-overloaded-virtual
    # -Wpedantic
  )
endif()
