include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Also, add the openqube library.
add_subdirectory(openqube)

set(LINK_LIBS avogadro openqube)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

### Surfaces
set(surfaceextension_SRCS
  surfaceextension.cpp
  surfacedialog.cpp
  vdwsurface.cpp
  qtiocompressor/qtiocompressor.cpp
)

set(orbitalextension_SRCS
  orbitalextension.cpp
  orbitalsettingsdialog.cpp
  orbitaltablemodel.cpp
  orbitalwidget.cpp
  vdwsurface.cpp
  qtiocompressor/qtiocompressor.cpp
)

set(orbitalextension_UIS
   orbitalwidget.ui
   orbitalsettingsdialog.ui
)

avogadro_plugin_nogl(surfaceextension
  "${surfaceextension_SRCS}"
  surfacedialog.ui
)

target_link_libraries(surfaceextension
   ${LINK_LIBS}
   ${ZLIB_LIBRARIES}
)

avogadro_plugin_nogl(orbitalextension
  "${orbitalextension_SRCS}"
  "${orbitalextension_UIS}"
)

target_link_libraries(orbitalextension
  ${LINK_LIBS}
  ${ZLIB_LIBRARIES}
)
