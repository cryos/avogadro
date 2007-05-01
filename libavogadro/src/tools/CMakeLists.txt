set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR lib/avogadro/tools)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

### drawtool
set(drawtool_SRCS drawtool.cpp)
qt4_add_resources(drawtool_RC_SRCS draw.qrc)
qt4_automoc(${drawtool_SRCS})
ADD_LIBRARY(drawtool SHARED drawtool.cpp ${drawtool_RC_SRCS} )
TARGET_LINK_LIBRARIES(drawtool 
    ${OPENBABEL2_LIBRARIES} 
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES} 
    avogadro-lib)
INSTALL(TARGETS drawtool DESTINATION ${DESTINATION_DIR})

### clickmeasuretool
set(clickmeasuretool_SRCS clickmeasuretool.cpp)
qt4_add_resources(clickmeasuretool_RC_SRCS measure.qrc)
qt4_automoc(${clickmeasuretool_SRCS})
ADD_LIBRARY(clickmeasuretool SHARED ${clickmeasuretool_SRCS} ${clickmeasuretool_RC_SRCS} )
TARGET_LINK_LIBRARIES(clickmeasuretool 
    ${OPENBABEL2_LIBRARIES} 
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES} 
    avogadro-lib)
INSTALL(TARGETS clickmeasuretool DESTINATION ${DESTINATION_DIR})

### selectrotatetool
set(selectrotatetool_SRCS selectrotatetool.cpp)
qt4_add_resources(selectrotatetool_RC_SRCS select.qrc)
qt4_automoc(${selectrotatetool_SRCS})
ADD_LIBRARY(selectrotatetool SHARED ${selectrotatetool_SRCS} ${selectrotatetool_RC_SRCS})
TARGET_LINK_LIBRARIES(selectrotatetool 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS selectrotatetool DESTINATION ${DESTINATION_DIR})

### navigatetool
set(navigatetool_SRCS navigatetool.cpp)
qt4_automoc(${navigatetool_SRCS})
ADD_LIBRARY(navigatetool SHARED ${navigatetool_SRCS})
TARGET_LINK_LIBRARIES(navigatetool 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS navigatetool DESTINATION ${DESTINATION_DIR})

