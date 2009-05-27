include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(DESTINATION_DIR ${Avogadro_PLUGIN_INSTALL_DIR}/engines)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro")

# label engine
avogadro_plugin(labelengine labelengine.cpp labelsettingswidget.ui)

# overlay engine
avogadro_plugin(overlayengine overlayengine.cpp overlaysettingswidget.ui)

# wireframe
avogadro_plugin(wireengine wireengine.cpp wiresettingswidget.ui)

# hydrogen bond
avogadro_plugin(hbondengine hbondengine.cpp hbondsettingswidget.ui)

# force engine
avogadro_plugin(forceengine forceengine.cpp)

# sphere
avogadro_plugin(sphereengine sphereengine.cpp spheresettingswidget.ui)

# stick (cylinder) engine
avogadro_plugin(stickengine stickengine.cpp sticksettingswidget.ui)

# Ribbon engine (biological structure)
avogadro_plugin(ribbonengine ribbonengine.cpp ribbonsettingswidget.ui)

# Cartoon engine (biological structure)
avogadro_plugin(cartoonengine "cartoonengine.cpp;cartoonmeshgenerator.cpp" cartoonsettingswidget.ui)

# Ring engine - display cyclic rings
avogadro_plugin(ringengine ringengine.cpp ringsettingswidget.ui)

# Polygon engine
avogadro_plugin(polygonengine polygonengine.cpp)

# Surface engine - molecular surfaces and 3D functions
avogadro_plugin(surfaceengine surfaceengine.cpp surfacesettingswidget.ui)

# Orbital engine - molecular orbitals
avogadro_plugin(orbitalengine orbitalengine.cpp orbitalsettingswidget.ui)

# Axes engine - display the axes
avogadro_plugin(axesengine axesengine.cpp)

# Dipole engine -- dipole moment
avogadro_plugin(dipoleengine dipoleengine.cpp dipolesettingswidget.ui)

# Simple wire engine - designed for efficiency with really big systems
avogadro_plugin(simplewireengine simplewireengine.cpp)

if(ALL_PYTHON_FOUND)
  file(GLOB python_SCRIPTS "python/*.py")
  install(FILES ${python_SCRIPTS} DESTINATION share/libavogadro/engineScripts)
endif(ALL_PYTHON_FOUND)
