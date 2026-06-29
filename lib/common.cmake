#
# default CMake
#
# CUDA
enable_language(CUDA)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
  
if (NOT DEFINED CMAKE_CUDA_STANDARD)
  set(CMAKE_CUDA_STANDARD 20)
  set(CMAKE_CUDA_STANDARD_REQUIRED TRUE)
endif()

find_library(CUDART_LIB 
  NAMES cudart
  PATHS ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})

include_directories(
  SYSTEM ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

#
# Linux
#
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)

  set(my_bin_type SHARED)

#  set(CMAKE_CXX_FLAGS "-Wall -Wextra -march=native")
#  set(CMAKE_CXX_FLAGS "-Wall -march=native")
  set(CMAKE_CXX_FLAGS "-march=native")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
  set(CMAKE_CXX_FLAGS_RELWITHASAN "-O3 -g -fsanitize=address")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  
  set(my_install_directory
    ${CMAKE_CURRENT_LIST_DIR})
  
  set(my_include_directories
    /usr/include/freetype2
    ${CMAKE_CURRENT_LIST_DIR}/gsys/shaders
    ${CMAKE_CURRENT_LIST_DIR})    
    
  set(my_link_directories
    ${CUDART_LIB} 
    ${my_install_directory})
  
  set(my_link_libraries
    nvidia-encode    
    nvcuvid
    cuda
    freetype
    freeimage
    glfw
    EGL       
    GL
    m )

#
# Visual Studio
#
elseif (WIN32)

  set(my_link_type STATIC)

  # force to set even when there is no '*.cu'
  if (NOT DEFINED CUDA_TOOLKIT_ROOT_DIR)
    set(CUDA_TOOLKIT_ROOT_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.8")
  endif()
  
  # no window: (does not seem working  because of console base cpp
  # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
  
  # see warning C4530
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /EHsc /FS")

  # "_iterator_debug_level-の不一致"
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /MP")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /MD /MP")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /MD /MP")

  set(my_install_directory
    ${CMAKE_CURRENT_LIST_DIR}/bin/${CMAKE_BUILD_TYPE})    
  
  set(my_include_directories
    ${CMAKE_CURRENT_LIST_DIR}/win64/include
    ${CUDA_TOOLKIT_ROOT_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR})

  set(my_link_directories
    ${CMAKE_CURRENT_LIST_DIR}/win64/lib
    ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64
    ${my_install_directory})

  set(my_link_libraries
    nvencodeapi.lib    
    nvcuvid.lib
    cudart_static.lib
    cuda.lib  
    ws2_32.lib
    libiconv-2
    libfreeimage-3
    libfontconfig-1
    libfreetype-6
    libharfbuzz-0  
    glfw3.lib )
  
endif()

# checkin
if (NOT TARGET checkin)
  if (NOT DEFINED my_checkin_directory)
    set(my_checkin_directory ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  add_custom_target(checkin COMMAND
    "find" ${my_checkin_directory}
    "'('" "-name" "win64" "-o" "-name" "external" "-o" "-name" "build" "')'" "-prune" "-o" "'('" 
    "-name" "CMakeLists.txt" "-o" 
    "-name" "'*.cmake'" "-o" 
    "-name" "'*.cpp'"  "-o" 
    "-name" "'*.h'"    "-o" 
    "-name" "'*.hpp'"  "-o" 
    "-name" "'*.ipp'"  "-o" 
    "-name" "'*.conf'" "-o" 
    "-name" "'*.json'" "-o" 
    "-name" "'*.us'"   "-o" 
    "-name" "'*.cu'"      
    "')'" "-print" "-exec" "ci" "-l" "-q" "-min_debug" "'{}'" "';'" )
endif()

# revision
execute_process(
  COMMAND 
  git rev-parse --short HEAD OUTPUT_VARIABLE GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)

add_compile_definitions(GIT_REVISION="${GIT_REVISION}")

# report
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} : ${CMAKE_CURRENT_SOURCE_DIR}")

# debug
# get_cmake_property(_variableNames VARIABLES)
#   foreach(_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
#   endforeach()

