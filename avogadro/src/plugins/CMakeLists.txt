set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

include_directories(${OPENBABEL2_INCLUDE_DIR} 
	${EIGEN_INCLUDE_DIR} 
	${AVOGADRO_INCLUDE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
    ../
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
INSTALL(TARGETS draw DESTINATION lib/avogadro)

### navigate
set(navigate_SRCS navigate.cpp)
qt4_automoc(${navigate_SRCS})
ADD_LIBRARY(navigate SHARED navigate.cpp ${navigate_RC_SRCS} )
TARGET_LINK_LIBRARIES(navigate 
    ${OPENBABEL2_LIBRARIES} 
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES} 
    avogadro-lib)
INSTALL(TARGETS navigate DESTINATION lib/avogadro)

### gamess
set(gamess_SRCS gamess.cpp gamessinputdialog.cpp gamessinputdata.cpp)
qt4_wrap_ui(gamess_UIS_H gamessinputdialog.ui)
qt4_automoc(gamess.cpp gamessinputdialog.cpp gamessinputdata.cpp)
ADD_LIBRARY(gamess SHARED ${gamess_SRCS} ${gamess_UIS_H})
TARGET_LINK_LIBRARIES(gamess 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS gamess DESTINATION lib/avogadro)

### ghemical
set(ghemical_SRCS ghemical.cpp)
qt4_automoc(${ghemical_SRCS})
ADD_LIBRARY(ghemical SHARED ${ghemical_SRCS})
TARGET_LINK_LIBRARIES(ghemical 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS ghemical DESTINATION lib/avogadro)

### hydrogens
set(hydrogens_SRCS hydrogens.cpp)
qt4_automoc(${hydrogens_SRCS})
ADD_LIBRARY(hydrogens SHARED ${hydrogens_SRCS})
TARGET_LINK_LIBRARIES(hydrogens 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS hydrogens DESTINATION lib/avogadro)

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
INSTALL(TARGETS selectrotate DESTINATION lib/avogadro)

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
INSTALL(TARGETS navigate DESTINATION lib/avogadro)

