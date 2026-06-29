set(external_dir ${AA60_LIBRARY_ROOT_DIR}/external)

# apps
set(sources
  alienrain/alienrain.cpp 
  basicfbo/basicfbo.cpp 
  bindlesstex/bindlesstex.cpp 
  blendmatrix/blendmatrix.cpp 
  blinnphong/blinnphong.cpp 
  bumpmapping/bumpmapping.cpp 
  clipdistance/clipdistance.cpp 
  csflocking/csflocking.cpp 
  cubemapenv/cubemapenv.cpp 
  cubicbezier/cubicbezier.cpp 
  cullindirect/cullindirect.cpp 
  deferredshading/deferredshading.cpp 
  depthclamp/depthclamp.cpp 
  dflandscape/dflandscape.cpp 
  dispmap/dispmap.cpp 
  dof/dof.cpp 
  envmapsphere/envmapsphere.cpp 
  equirectangular/equirectangular.cpp 
  fragcolorfrompos/fragcolorfrompos.cpp 
  fragmentlist/fragmentlist.cpp 
  grass/grass.cpp 
  gsculling/gsculling.cpp 
  gslayered/gslayered.cpp 
  gsquads/gsquads.cpp 
  gstessellate/gstessellate.cpp 
  hdrbloom/hdrbloom.cpp 
  hdrexposure/hdrexposure.cpp 
  hdrtonemap/hdrtonemap.cpp 
  hqfilter/hqfilter.cpp 
  indexedcube/indexedcube.cpp 
  indirectmaterial/indirectmaterial.cpp 
  instancedattribs/instancedattribs.cpp 
  julia/julia.cpp 
  ktxview/ktxview.cpp 
  linesmooth/linesmooth.cpp 
  mirrorclampedge/mirrorclampedge.cpp 
  movingtri/movingtri.cpp 
  msaanative/msaanative.cpp 
  multidrawindirect/multidrawindirect.cpp 
  multimaterial/multimaterial.cpp 
  multiscissor/multiscissor.cpp 
  multiviewport/multiviewport.cpp 
  noise/noise.cpp 
  noperspective/noperspective.cpp 
  normalviewer/normalviewer.cpp 
  objectexploder/objectexploder.cpp 
  packetbuffer/packetbuffer.cpp 
  pmbstreaming/pmbstreaming.cpp 
  perpixelgloss/perpixelgloss.cpp 
  phonglighting/phonglighting.cpp 
  prefixsum/prefixsum.cpp 
  prefixsum2d/prefixsum2d.cpp 
  programinfo/programinfo.cpp 
  raytracer/raytracer.cpp 
  rimlight/rimlight.cpp 
  sampleshading/sampleshading.cpp 
  sdfdemo/sdfdemo.cpp 
  sb6mrender/sb6mrender.cpp 
  shadowmapping/shadowmapping.cpp 
  shapedpoints/shapedpoints.cpp 
  simpleclear/simpleclear.cpp 
  simpletexcoords/simpletexcoords.cpp 
  simpletexture/simpletexture.cpp 
  singlepoint/singlepoint.cpp 
  singletri/singletri.cpp 
  spinnycube/spinnycube.cpp 
  springmass/springmass.cpp 
  ssao/ssao.cpp 
  starfield/starfield.cpp 
  stereo/stereo.cpp 
  tessellatedcube/tessellatedcube.cpp 
  tessellatedgstri/tessellatedgstri.cpp 
  tessellatedtri/tessellatedtri.cpp 
  tessmodes/tessmodes.cpp 
  tesssubdivmodes/tesssubdivmodes.cpp 
  toonshading/toonshading.cpp 
  tunnel/tunnel.cpp 
  wrapmodes/wrapmodes.cpp 
  sb6ktx.cpp 
  sb6object.cpp 
  base_app.cpp 
  main.cpp )

# not ported yet
if (UNIX)
  set(sources ${sources}
    compressrgtc/compressrgtc.cpp 
    ompparticles/ompparticles.cpp 
    pmbfractal/pmbfractal.cpp 
  )
  
  set(my_link_libraries ${my_link_libraries} omp5 )

endif()



