include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

### Surfaces
set(surfaceextension_SRCS
    surfaceextension.cpp
    surfacedialog.cpp
    basisset.cpp
    gaussianset.cpp
    gaussianfchk.cpp
    slaterset.cpp
    mopacaux.cpp
    molpro.cpp
    vdwsurface.cpp
    qtiocompressor/qtiocompressor.cpp
)

set(orbitalextension_SRCS
    orbitalextension.cpp
    orbitalsettingsdialog.cpp
    orbitaltablemodel.cpp
    orbitalwidget.cpp
    basisset.cpp
    gaussianset.cpp
    gaussianfchk.cpp
    slaterset.cpp
    mopacaux.cpp
    molpro.cpp
    vdwsurface.cpp
    qtiocompressor/qtiocompressor.cpp
)

set( orbitalextension_UIS
     orbitalwidget.ui
     orbitalsettingsdialog.ui
)

avogadro_plugin_nogl(surfaceextension
	            "${surfaceextension_SRCS}"
		    surfacedialog.ui
)

target_link_libraries( surfaceextension
		       ${LINK_LIBS}
		       ${ZLIB_LIBRARIES}
)

avogadro_plugin_nogl(orbitalextension
	            "${orbitalextension_SRCS}"
		    "${orbitalextension_UIS}"
)

target_link_libraries( orbitalextension
		       ${LINK_LIBS}
		       ${ZLIB_LIBRARIES}
)
