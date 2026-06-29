if(DEFINED AA60_PROJECT_FEATURES_INCLUDED)
  return()
endif()
set(AA60_PROJECT_FEATURES_INCLUDED ON)

option(AA60_ENABLE_ASAN "Enable AddressSanitizer" OFF)

add_library(aa60_project_features INTERFACE)
target_compile_features(aa60_project_features INTERFACE cxx_std_20)

if(NOT MSVC)
  target_compile_options(aa60_project_features INTERFACE -march=native)
endif()

if(AA60_ENABLE_ASAN AND NOT MSVC)
  target_compile_options(aa60_project_features INTERFACE -fsanitize=address)
  target_link_options(aa60_project_features INTERFACE -fsanitize=address)
endif()
