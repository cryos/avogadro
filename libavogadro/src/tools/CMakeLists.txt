set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/tools)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

### drawtool
set(drawtool_SRCS drawtool.cpp periodictabledialog.cpp)
qt4_add_resources(drawtool_RC_SRCS draw.qrc)
qt4_wrap_ui(drawtool_UIS_H periodictabledialog.ui)
qt4_automoc(${drawtool_SRCS})
ADD_LIBRARY(drawtool SHARED drawcommand.cpp drawtool.cpp 
                     periodictabledialog.cpp 
                     ${drawtool_UIS_H} ${drawtool_RC_SRCS} )
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
qt4_add_resources(navigatetool_RC_SRCS navigate.qrc)
qt4_automoc(${navigatetool_SRCS})
ADD_LIBRARY(navigatetool SHARED ${navigatetool_SRCS} ${navigatetool_RC_SRCS})
TARGET_LINK_LIBRARIES(navigatetool 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS navigatetool DESTINATION ${DESTINATION_DIR})

### manipulatetool
set(manipulatetool_SRCS manipulatetool.cpp)
qt4_add_resources(manipulatetool_RC_SRCS manipulate.qrc)
qt4_automoc(${manipulatetool_SRCS})
ADD_LIBRARY(manipulatetool SHARED ${manipulatetool_SRCS} ${manipulatetool_RC_SRCS})
TARGET_LINK_LIBRARIES(manipulatetool 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS manipulatetool DESTINATION ${DESTINATION_DIR})

### autorotatetool
set(autorotatetool_SRCS autorotatetool.cpp)
#qt4_add_resources(autorotatetool_RC_SRCS autorotate.qrc)
qt4_automoc(${autorotatetool_SRCS})
ADD_LIBRARY(autorotatetool SHARED ${autorotatetool_SRCS} ${autorotatetool_RC_SRCS})
TARGET_LINK_LIBRARIES(autorotatetool
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
	${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS autorotatetool DESTINATION ${DESTINATION_DIR})
