include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Also, add the openqube library unless we are using the system openqube
option(Avogadro_USE_SYSTEM_OPENQUBE "Use the system OpenQube?" OFF)
if(Avogadro_USE_SYSTEM_OPENQUBE)
  find_package(OpenQube REQUIRED)

else()
  add_subdirectory(openqube)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(LINK_LIBS openqube avogadro ${ZLIB_LIBRARIES})
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

include_directories(${ZLIB_INCLUDE_DIR})
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

avogadro_plugin_nogl(orbitalextension
  "${orbitalextension_SRCS}"
  "${orbitalextension_UIS}"
)

