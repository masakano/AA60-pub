set(external_dir ${AA60_LIBRARY_ROOT_DIR}/external)

set(sources

  ${external_dir}/imgui/imgui.cpp
  ${external_dir}/imgui/imgui_tables.cpp
  ${external_dir}/imgui/imgui_widgets.cpp
  ${external_dir}/imgui/imgui_draw.cpp
  ${external_dir}/ImGuizmo/src/ImGuizmo.cpp

  src/imgui/imgui_impl_spu.cpp
  src/spu_gesture.cpp
  src/spu_shader.cpp
  src/spu_frame.cpp
  src/spu_texture.cpp
  src/spu_renderstate.cpp
  src/spu_backoffice.cpp
  src/spu_site.cpp
  src/dds/dds.cpp
)
