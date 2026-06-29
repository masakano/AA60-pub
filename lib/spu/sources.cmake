set(tinyexr_dir
  ${CMAKE_CURRENT_LIST_DIR}/../external/tinyexr )

set(stb_dir
  ${CMAKE_CURRENT_LIST_DIR}/../external/stb )

set(spu_private_include_directories
  ${tinyexr_dir}
  ${tinyexr_dir}/deps/miniz
  ${stb_dir}
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/Interface
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/Utils
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/NvDecoder
  ${CMAKE_CURRENT_LIST_DIR}/src/cuda/NV/NvEncoder)

set(sys_sources
  src/GLFW/dev.cpp 
  src/EGL/dev.cpp  
  src/gl.cpp 
  src/spu_renderstate.cpp 
  src/spu_frame.cpp 
  src/spu_graphics.cpp 
  src/spu_inventory.cpp 
  src/spu_array.cpp 
  src/spu_print.cpp 
  src/spu_shader.cpp 
  src/spu_texture.cpp
  src/spu_query.cpp 
  src/spu_texture_save.cpp
  src/resource_object.cpp
  src/resource_image.cpp 
  src/resource_texture.cpp 
  src/resource_shader.cpp
  src/shader_loader.cpp
  src/spu_shader_gathered_uniform_source.cpp 
  src/image_loader.cpp 
  ${tinyexr_dir}/tinyexr.cc
  src/miniz.cpp 
)

set(cuda_sources
  src/cuda/embed/embed.cpp 
  src/cuda/NV/NvDecoder/NvDecoder.cpp 
  src/cuda/NV/NvEncoder/NvEncoder.cpp 
  src/cuda/NV/NvEncoder/NvEncoderCuda.cpp
  src/cuda/NV/Utils/Logger.cpp
  src/cuda/pipe.cpp
  src/cuda/spu_video.cpp
  src/cuda/color_space.cu )

set(sources
  ${sys_sources}
  ${cuda_sources} 
  ${aux_sources} 
)
