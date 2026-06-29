include_guard(GLOBAL)

find_package(CUDAToolkit REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

find_library(AA60_EGL_LIBRARY NAMES EGL REQUIRED)
find_library(AA60_FREEIMAGE_LIBRARY NAMES freeimage REQUIRED)
find_library(AA60_GL_LIBRARY NAMES GL REQUIRED)
find_library(AA60_NVCUVID_LIBRARY NAMES nvcuvid REQUIRED)
find_library(AA60_NVIDIA_ENCODE_LIBRARY NAMES nvidia-encode REQUIRED)

if(TARGET Freetype::Freetype)
  add_library(aa60_freetype INTERFACE)
  target_link_libraries(aa60_freetype INTERFACE Freetype::Freetype)
else()
  add_library(aa60_freetype INTERFACE)
  target_include_directories(aa60_freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
  target_link_libraries(aa60_freetype INTERFACE ${FREETYPE_LIBRARIES})
endif()

function(aa60_link_spu_dependencies target_name)
  target_link_libraries(${target_name} PRIVATE
    CUDA::cuda_driver
    CUDA::cudart_static
    Freetype::Freetype
    glfw
    ${AA60_EGL_LIBRARY}
    ${AA60_FREEIMAGE_LIBRARY}
    ${AA60_GL_LIBRARY}
    ${AA60_NVCUVID_LIBRARY}
    ${AA60_NVIDIA_ENCODE_LIBRARY}
    m
  )
endfunction()
