set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

include_directories(${OPENBABEL_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GLEW_INCLUDE_DIR}
)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/engines)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

# label engine
set(labelengine_SRCS labelengine.cpp)
qt4_wrap_ui(labelengine_UIS labelsettingswidget.ui)
qt4_automoc(${labelengine_SRCS})
add_library(labelengine MODULE ${labelengine_SRCS} ${labelengine_UIS} )
target_link_libraries(labelengine ${LINK_LIBS})
install(TARGETS labelengine DESTINATION ${DESTINATION_DIR})

# overlay engine
set(overlayengine_SRCS overlayengine.cpp)
qt4_wrap_ui(overlayengine_UIS overlaysettingswidget.ui)
qt4_automoc(${overlayengine_SRCS})
add_library(overlayengine MODULE ${overlayengine_SRCS} ${overlayengine_UIS} )
target_link_libraries(overlayengine ${LINK_LIBS})
install(TARGETS overlayengine DESTINATION ${DESTINATION_DIR})

# wireframe
set(wireengine_SRCS wireengine.cpp)
qt4_automoc(${wireengine_SRCS})
qt4_wrap_ui(wireengine_UIS wiresettingswidget.ui)
add_library(wireengine MODULE ${wireengine_SRCS} ${wireengine_UIS})
target_link_libraries(wireengine ${LINK_LIBS})
install(TARGETS wireengine DESTINATION ${DESTINATION_DIR})

# hydrogen bond
set(hbondengine_SRCS hbondengine.cpp)
qt4_automoc(${hbondengine_SRCS})
qt4_wrap_ui(hbondengine_UIS hbondsettingswidget.ui)
add_library(hbondengine MODULE ${hbondengine_SRCS} ${hbondengine_UIS})
target_link_libraries(hbondengine ${LINK_LIBS})
install(TARGETS hbondengine DESTINATION ${DESTINATION_DIR})

# force engine - DISABLED TEMPORARILY
#set(forceengine_SRCS forceengine.cpp)
#qt4_automoc(${forceengine_SRCS})
#add_library(forceengine MODULE ${forceengine_SRCS} ${forceengine_UIS})
#target_link_libraries(forceengine ${LINK_LIBS})
#install(TARGETS forceengine DESTINATION ${DESTINATION_DIR})

# sphere
set(sphereengine_SRCS sphereengine.cpp)
qt4_wrap_ui(sphereengine_UIS spheresettingswidget.ui)
qt4_automoc(${sphereengine_SRCS})
add_library(sphereengine MODULE ${sphereengine_SRCS} ${sphereengine_UIS} )
target_link_libraries(sphereengine ${LINK_LIBS})
install(TARGETS sphereengine DESTINATION ${DESTINATION_DIR})

# stick (cylinder) engine
set(stickengine_SRCS stickengine.cpp)
qt4_wrap_ui(stickengine_UIS sticksettingswidget.ui)
qt4_automoc(${stickengine_SRCS})
add_library(stickengine MODULE ${stickengine_SRCS} ${stickengine_UIS})
target_link_libraries(stickengine ${LINK_LIBS})
install(TARGETS stickengine DESTINATION ${DESTINATION_DIR})

# Ribbon engine (biological structure)
set(ribbonengine_SRCS ribbonengine.cpp)
qt4_wrap_ui(ribbonengine_UIS ribbonsettingswidget.ui)
qt4_automoc(${ribbonengine_SRCS})
add_library(ribbonengine MODULE ${ribbonengine_SRCS} ${ribbonengine_UIS} )
target_link_libraries(ribbonengine ${LINK_LIBS})
install(TARGETS ribbonengine DESTINATION ${DESTINATION_DIR})

# Ring engine - display cyclic rings
set(ringengine_SRCS ringengine.cpp)
qt4_wrap_ui(ringengine_UIS ringsettingswidget.ui)
qt4_automoc(${ringengine_SRCS})
add_library(ringengine MODULE ${ringengine_SRCS} ${ringengine_UIS} )
target_link_libraries(ringengine ${LINK_LIBS})
install(TARGETS ringengine DESTINATION ${DESTINATION_DIR})

# Polygon engine
set(polygonengine_SRCS polygonengine.cpp)
qt4_automoc(${polygonengine_SRCS})
add_library(polygonengine MODULE ${polygonengine_SRCS} )
target_link_libraries(polygonengine ${LINK_LIBS})
install(TARGETS polygonengine DESTINATION ${DESTINATION_DIR})

# Surface engine - molecular surfaces and 3D functions
set(surfaceengine_SRCS iso.cpp surfaceengine.cpp)
qt4_wrap_ui(surfaceengine_UIS surfacesettingswidget.ui)
qt4_automoc(${surfaceengine_SRCS})
add_library(surfaceengine MODULE ${surfaceengine_SRCS} ${surfaceengine_UIS})
target_link_libraries(surfaceengine ${LINK_LIBS})
install(TARGETS surfaceengine DESTINATION ${DESTINATION_DIR})

# Orbital engine - molecular orbitals
set(orbitalengine_SRCS iso.cpp orbitalengine.cpp)
qt4_wrap_ui(orbitalengine_UIS orbitalsettingswidget.ui)
qt4_automoc(${orbitalengine_SRCS})
add_library(orbitalengine MODULE ${orbitalengine_SRCS} ${orbitalengine_UIS})
target_link_libraries(orbitalengine ${LINK_LIBS})
install(TARGETS orbitalengine DESTINATION ${DESTINATION_DIR})

# Axes engine - display the axes
set(axesengine_SRCS axesengine.cpp)
qt4_automoc(${axesengine_SRCS})
add_library(axesengine MODULE ${axesengine_SRCS})
target_link_libraries(axesengine ${LINK_LIBS})
install(TARGETS axesengine DESTINATION ${DESTINATION_DIR})

# Dipole engine -- dipole moment
set(dipoleengine_SRCS dipoleengine.cpp)
qt4_wrap_ui(dipoleengine_UIS dipolesettingswidget.ui)
qt4_automoc(${dipoleengine_SRCS})
add_library(dipoleengine MODULE ${dipoleengine_SRCS} ${dipoleengine_UIS})
target_link_libraries(dipoleengine ${LINK_LIBS})
install(TARGETS dipoleengine DESTINATION ${DESTINATION_DIR})

if(ALL_PYTHON_FOUND)
  file(GLOB python_SCRIPTS "python/*.py")
  install(FILES ${python_SCRIPTS} DESTINATION share/libavogadro/engineScripts)
endif(ALL_PYTHON_FOUND)
