set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

include_directories(${OPENBABEL_INCLUDE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
)


ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

# ball and stick engine
set(bsengine_SRCS bsengine.cpp)
qt4_automoc(${bsengine_SRCS})
ADD_LIBRARY(bsengine SHARED ${bsengine_SRCS} )
TARGET_LINK_LIBRARIES(bsengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS bsengine DESTINATION lib/avogadro)

# ball and stick dynamic engine
set(bsdyengine_SRCS bsdyengine.cpp)
qt4_automoc(${bsdyengine_SRCS})
ADD_LIBRARY(bsdyengine SHARED ${bsdyengine_SRCS} )
TARGET_LINK_LIBRARIES(bsdyengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS bsdyengine DESTINATION lib/avogadro)

# label engine
set(labelengine_SRCS labelengine.cpp)
qt4_automoc(${labelengine_SRCS})
ADD_LIBRARY(labelengine SHARED ${labelengine_SRCS} )
TARGET_LINK_LIBRARIES(labelengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS labelengine DESTINATION lib/avogadro)

# wireframe
set(wireengine_SRCS wireengine.cpp)
qt4_automoc(${wireengine_SRCS})
ADD_LIBRARY(wireengine SHARED ${wireengine_SRCS} )
TARGET_LINK_LIBRARIES(wireengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS wireengine DESTINATION lib/avogadro)

# sphere
set(sphereengine_SRCS sphereengine.cpp)
qt4_automoc(${sphereengine_SRCS})
ADD_LIBRARY(sphereengine SHARED ${sphereengine_SRCS} )
TARGET_LINK_LIBRARIES(sphereengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)

INSTALL(TARGETS sphereengine DESTINATION lib/avogadro)
