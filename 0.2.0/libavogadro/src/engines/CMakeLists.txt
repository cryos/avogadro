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

# axes engine
set(axesengine_SRCS axesengine.cpp)
qt4_automoc(${axesengine_SRCS})
ADD_LIBRARY(axesengine SHARED ${axesengine_SRCS} )
TARGET_LINK_LIBRARIES(axesengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS axesengine DESTINATION ${DESTINATION_DIR})

# wireframe
set(wireengine_SRCS wireengine.cpp)
qt4_automoc(${wireengine_SRCS})
ADD_LIBRARY(wireengine SHARED ${wireengine_SRCS} )
TARGET_LINK_LIBRARIES(wireengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS wireengine DESTINATION ${DESTINATION_DIR})

# debug engine
set(debugengine_SRCS debugengine.cpp)
qt4_automoc(${debugengine_SRCS})
ADD_LIBRARY(debugengine SHARED ${debugengine_SRCS} )
TARGET_LINK_LIBRARIES(debugengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS debugengine DESTINATION ${DESTINATION_DIR})

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

# Ribbon engine (disabled for 0.2 release -- not fully implemented)
#set(ribbonengine_SRCS ribbonengine.cpp)
#qt4_wrap_ui(ribbonengine_UIS ribbonsettingswidget.ui)
#qt4_automoc(${ribbonengine_SRCS})
#ADD_LIBRARY(ribbonengine SHARED ${ribbonengine_SRCS} ${ribbonengine_UIS} )
#TARGET_LINK_LIBRARIES(ribbonengine avogadro-lib
#                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
#                                ${OPENGL_LIBRARIES}
#)

#INSTALL(TARGETS ribbonengine DESTINATION ${DESTINATION_DIR})
