set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

include_directories(${OPENBABEL_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/engines)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

# ball and stick dynamic engine
set(bsdyengine_SRCS bsdyengine.cpp)
qt4_wrap_ui(bsdyengine_UIS bsdysettingswidget.ui)
qt4_automoc(${bsdyengine_SRCS})
ADD_LIBRARY(bsdyengine SHARED ${bsdyengine_SRCS} ${bsdyengine_UIS})
TARGET_LINK_LIBRARIES(bsdyengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS bsdyengine DESTINATION ${DESTINATION_DIR})

# label engine
set(labelengine_SRCS labelengine.cpp)
qt4_wrap_ui(labelengine_UIS labelsettingswidget.ui)
qt4_automoc(${labelengine_SRCS})
ADD_LIBRARY(labelengine SHARED ${labelengine_SRCS} ${labelengine_UIS} )
TARGET_LINK_LIBRARIES(labelengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS labelengine DESTINATION ${DESTINATION_DIR})

# wireframe
set(wireengine_SRCS wireengine.cpp)
qt4_automoc(${wireengine_SRCS})
qt4_wrap_ui(wireengine_UIS wiresettingswidget.ui)
ADD_LIBRARY(wireengine SHARED ${wireengine_SRCS} ${wireengine_UIS})
TARGET_LINK_LIBRARIES(wireengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS wireengine DESTINATION ${DESTINATION_DIR})

# hydrogen bond
set(hbondengine_SRCS hbondengine.cpp)
qt4_automoc(${hbondengine_SRCS})
qt4_wrap_ui(hbondengine_UIS hbondsettingswidget.ui)
ADD_LIBRARY(hbondengine SHARED ${hbondengine_SRCS} ${hbondengine_UIS})
TARGET_LINK_LIBRARIES(hbondengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS hbondengine DESTINATION ${DESTINATION_DIR})

# sphere
set(sphereengine_SRCS sphereengine.cpp)
qt4_wrap_ui(sphereengine_UIS spheresettingswidget.ui)
qt4_automoc(${sphereengine_SRCS})
ADD_LIBRARY(sphereengine SHARED ${sphereengine_SRCS} ${sphereengine_UIS} )
TARGET_LINK_LIBRARIES(sphereengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS sphereengine DESTINATION ${DESTINATION_DIR})

# stick (cylinder) engine
set(stickengine_SRCS stickengine.cpp)
qt4_automoc(${stickengine_SRCS})
ADD_LIBRARY(stickengine SHARED ${stickengine_SRCS} )
TARGET_LINK_LIBRARIES(stickengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS stickengine DESTINATION ${DESTINATION_DIR})

# Ribbon engine (biological structure)
set(ribbonengine_SRCS ribbonengine.cpp)
qt4_wrap_ui(ribbonengine_UIS ribbonsettingswidget.ui)
qt4_automoc(${ribbonengine_SRCS})
ADD_LIBRARY(ribbonengine SHARED ${ribbonengine_SRCS} ${ribbonengine_UIS} )
TARGET_LINK_LIBRARIES(ribbonengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS ribbonengine DESTINATION ${DESTINATION_DIR})

# Ring engine - display cyclic rings
set(ringengine_SRCS ringengine.cpp)
qt4_wrap_ui(ringengine_UIS ringsettingswidget.ui)
qt4_automoc(${ringengine_SRCS})
ADD_LIBRARY(ringengine SHARED ${ringengine_SRCS} ${ringengine_UIS} )
TARGET_LINK_LIBRARIES(ringengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS ringengine DESTINATION ${DESTINATION_DIR})

# Polygon engine
set(polygonengine_SRCS polygonengine.cpp)
qt4_automoc(${polygonengine_SRCS})
ADD_LIBRARY(polygonengine SHARED ${polygonengine_SRCS} )
TARGET_LINK_LIBRARIES(polygonengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS polygonengine DESTINATION ${DESTINATION_DIR})

# Surface engine - molecular surfaces and 3D functions
set(surfaceengine_SRCS iso.cpp surfaceengine.cpp)
qt4_wrap_ui(surfaceengine_UIS surfacesettingswidget.ui)
qt4_automoc(${surfaceengine_SRCS})
ADD_LIBRARY(surfaceengine SHARED ${surfaceengine_SRCS} ${surfaceengine_UIS})
TARGET_LINK_LIBRARIES(surfaceengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS surfaceengine DESTINATION ${DESTINATION_DIR})
