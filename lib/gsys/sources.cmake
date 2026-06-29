
set(bullet_source_directory
  ${CMAKE_CURRENT_LIST_DIR}/../external/bullet3/src )

set(include_directories
  ${bullet_source_directory} )

set(core_sources
  src/object.cpp 
  src/drawcall.cpp 
  src/painter.cpp 
  src/node.cpp 
  src/canvas.cpp 
)

set(min_sources
  src/decorator/instance.cpp 
  src/decorator/material.cpp 
  src/decorator/shadowmap.cpp 
  src/painter/stdout/stdout.cpp 
  src/painter/sprite/sprite.cpp 
  src/painter/text/text.cpp
  src/painter/pbr/pbr.cpp 
  src/painter/pbr/displaced_pbr.cpp 
  src/painter/pbr/fur_pbr.cpp 
  src/node/camera.cpp 
  src/node/wavefront_obj/parser.cpp 
  src/node/wavefront_obj/wavefront_obj.cpp 
  src/canvas/copy/copy.cpp 
  src/canvas/gauss/gauss_1d.cpp 
  src/canvas/gauss/gauss_2d.cpp 
  src/canvas/tonemap/tonemap.cpp 
  src/canvas/shadowmap/shadowmap.cpp 
  src/canvas/shadowmap/shadowmap_inspector.cpp
  src/canvas/shadowmap/cascade_shadowmap.cpp 
  src/util/lambert_to_pbr.cpp
)

set(painter_sources
  src/painter/pointset/pointset.cpp 
  src/painter/mcube/mcube.cpp 
  src/painter/raycast/raycast.cpp 
  src/painter/toon/toon.cpp 
)

set(node_sources
  src/node/guizmo.cpp
  src/node/manifold_2d.cpp
  src/node/terrain/terrain.cpp
  src/node/volume/particles_to_volume.cpp
  src/node/volume/volume.cpp
  src/node/billboard.cpp 
)

set(canvas_sources
  src/canvas/irradiance/image_irradiance.cpp
  src/canvas/irradiance/cloudy_irradiance.cpp
  src/canvas/irradiance/analizer.cpp
  src/canvas/atlas/atlas.cpp 
  src/canvas/atlas/cube_atlas.cpp 
  src/canvas/lightmap/lightmap.cpp
  src/canvas/lensflare/lensflare1.cpp 
  src/canvas/lensflare/lensflare2.cpp 
  src/canvas/lensflare/lensflare3.cpp 
  src/canvas/ssao/ssao.cpp 
  src/canvas/dof/dof.cpp 
  src/canvas/fog/fog.cpp 
  src/canvas/bloom/bloom.cpp 
  src/canvas/bloom/glare.cpp 
  src/canvas/shadowmap/cube_shadowmap.cpp 
  src/canvas/shadowmap/penumbra_shadowmap.cpp 
)

set(util_sources
  src/util/node_mixer.cpp 
  src/util/gauss_3d.cpp 
  src/util/bullet.cpp 
  ${bullet_source_directory}/btBulletDynamicsAll.cpp 
  ${bullet_source_directory}/btBulletCollisionAll.cpp 
  ${bullet_source_directory}/btLinearMathAll.cpp 
)

set(gui_sources
  src/node/gui/base.cpp 
  src/node/gui/button.cpp 
  src/node/gui/menu.cpp 
  src/node/gui/birdview.cpp 
  src/node/gui/texview.cpp 
  src/node/gui/plotter.cpp 
  src/node/gui/slider.cpp 
  src/node/gui/color_slider.cpp 
  src/node/gui/panel.cpp 
  src/node/gui/tweakbar.cpp 
)

set(dui_sources
  src/node/dui/tweakbar.cpp 
  src/node/dui/button.cpp 
  src/node/dui/container.cpp 
  src/node/dui/popup.cpp 
  src/node/dui/slider.cpp 
  src/node/dui/text.cpp 
  src/node/dui/valuetext.cpp 
  src/node/dui/window.cpp 
  src/node/dui/node.cpp 
  )

set(plane_sources
  src/painter/plane/plane.cpp 
  src/painter/plane/water_plane.cpp 
  src/painter/plane/wave_generator.cpp
)
  
set(mmd_sources
  src/painter/mmd/mmd.cpp 
  src/node/mmd/master.cpp 
  src/node/mmd/actor.cpp 
  src/node/mmd/pmd.cpp 
  src/node/mmd/pmx.cpp 
  src/node/mmd/pmx_file.cpp 
  src/node/mmd/pmd_file.cpp 
  src/node/mmd/vmd_file.cpp 
)

set(freetype_sources
  src/painter/freetype/edtaa3func.cpp
  src/painter/freetype/ft_atlas.cpp
  src/painter/freetype/ft_font.cpp
  src/painter/freetype/ft_font_atlas.cpp
  src/painter/freetype/ft_text.cpp
  src/painter/freetype/utf8_utils.cpp
  src/painter/freetype/distance_field.cpp
)

set(gs_page_sources
  src/canvas/gs_page/gs_site.cpp
  src/canvas/gs_page/gs_page.cpp
)

set(gs_demo_page_sources
  src/canvas/gs_demo_page/gs_demo_page.cpp
  src/canvas/gs_demo_page/postproc.cpp
  src/canvas/gs_demo_page/floor.cpp
  src/canvas/gs_demo_page/node.cpp
  src/canvas/gs_demo_page/painter.cpp
  src/canvas/gs_demo_page/default_inspector.cpp
)

set(sources
  ${core_sources}
  ${min_sources}
  ${painter_sources}
  ${node_sources}
  ${canvas_sources}
  ${util_sources}
  ${gui_sources}  
  ${dui_sources}  
  ${mmd_sources}
  ${plane_sources}
  ${freetype_sources}
  ${gs_page_sources}
  ${gs_demo_page_sources}
)
