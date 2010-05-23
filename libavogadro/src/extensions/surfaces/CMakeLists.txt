include_directories(
  ../
  ${CMAKE_CURRENT_BINARY_DIR}
)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS avogadrocore avogadrowidget)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

### Surfaces
set(surfaceextension_SRCS surfaceextension.cpp surfacedialog.cpp
  basisset.cpp gaussianfchk.cpp slaterset.cpp mopacaux.cpp molpro.cpp
  vdwsurface.cpp qtiocompressor/qtiocompressor.cpp)
avogadro_plugin_nogl(surfaceextension "${surfaceextension_SRCS}" surfacedialog.ui)
target_link_libraries(surfaceextension ${LINK_LIBS} ${ZLIB_LIBRARIES})
