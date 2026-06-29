include_guard(GLOBAL)

include(dependencies)

function(aa60_add_imported_shared target_name library_path)
  if(TARGET ${target_name})
    return()
  endif()

  add_library(${target_name} SHARED IMPORTED GLOBAL)
  set_target_properties(${target_name} PROPERTIES
    IMPORTED_LOCATION ${library_path}
  )
endfunction()

if(NOT TARGET aa60_spu_runtime)
  add_library(aa60_spu_runtime INTERFACE)
  target_link_libraries(aa60_spu_runtime INTERFACE
    CUDA::cuda_driver
    CUDA::cudart_static
    glfw
    aa60_freetype
    ${AA60_EGL_LIBRARY}
    ${AA60_FREEIMAGE_LIBRARY}
    ${AA60_GL_LIBRARY}
    ${AA60_NVCUVID_LIBRARY}
    ${AA60_NVIDIA_ENCODE_LIBRARY}
    m
  )
endif()

aa60_add_imported_shared(ssys "${AA60_LIBRARY_ROOT_DIR}/libssys.so")
set_target_properties(ssys PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${AA60_LIBRARY_ROOT_DIR}"
)

aa60_add_imported_shared(smath "${AA60_LIBRARY_ROOT_DIR}/libsmath.so")
set_target_properties(smath PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${AA60_LIBRARY_ROOT_DIR}"
  INTERFACE_LINK_LIBRARIES ssys
)

aa60_add_imported_shared(spu "${AA60_LIBRARY_ROOT_DIR}/libspu.so")
set_target_properties(spu PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${AA60_LIBRARY_ROOT_DIR}"
  INTERFACE_LINK_LIBRARIES "ssys;aa60_spu_runtime"
)

aa60_add_imported_shared("spu++" "${AA60_LIBRARY_ROOT_DIR}/libspu++.so")
set_target_properties("spu++" PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES
    "${AA60_LIBRARY_ROOT_DIR};${AA60_LIBRARY_ROOT_DIR}/spu++;${AA60_LIBRARY_ROOT_DIR}/spu++/imgui;${AA60_LIBRARY_ROOT_DIR}/external/imgui;${AA60_LIBRARY_ROOT_DIR}/external/ImGuizmo/src"
  INTERFACE_LINK_LIBRARIES "ssys;smath;spu"
)

aa60_add_imported_shared(gsys "${AA60_LIBRARY_ROOT_DIR}/libgsys.so")
set_target_properties(gsys PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES
    "${AA60_LIBRARY_ROOT_DIR};${AA60_LIBRARY_ROOT_DIR}/gsys/shaders"
  INTERFACE_LINK_LIBRARIES "aa60_freetype;ssys;smath;spu;spu++"
)
