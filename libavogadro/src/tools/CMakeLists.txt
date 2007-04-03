set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

### draw
set(draw_SRCS draw.cpp)
qt4_add_resources(draw_RC_SRCS draw.qrc)
qt4_automoc(${draw_SRCS})
ADD_LIBRARY(draw SHARED draw.cpp ${draw_RC_SRCS} )
TARGET_LINK_LIBRARIES(draw 
    ${OPENBABEL2_LIBRARIES} 
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES} 
    avogadro-lib)
INSTALL(TARGETS draw DESTINATION lib/avogadro/tools)

### measure
set(measure_SRCS measure.cpp)
qt4_add_resources(measure_RC_SRCS measure.qrc)
qt4_automoc(${measure_SRCS})
ADD_LIBRARY(measure SHARED measure.cpp ${measure_RC_SRCS} )
TARGET_LINK_LIBRARIES(measure 
    ${OPENBABEL2_LIBRARIES} 
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES} 
    avogadro-lib)
INSTALL(TARGETS measure DESTINATION lib/avogadro/tools)

### selectrotate
set(selectrotate_SRCS selectrotate.cpp)
qt4_automoc(${selectrotate_SRCS})
ADD_LIBRARY(selectrotate SHARED ${selectrotate_SRCS})
TARGET_LINK_LIBRARIES(selectrotate 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS selectrotate DESTINATION lib/avogadro/tools)

### navigate
set(navigate_SRCS navigate.cpp)
qt4_automoc(${navigate_SRCS})
ADD_LIBRARY(navigate SHARED ${navigate_SRCS})
TARGET_LINK_LIBRARIES(navigate 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS navigate DESTINATION lib/avogadro/tools)

