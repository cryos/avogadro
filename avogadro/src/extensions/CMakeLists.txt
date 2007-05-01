set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR lib/avogadro/extensions)

include_directories(
    ../
	${CMAKE_CURRENT_BINARY_DIR}
	${OPENBABEL2_INCLUDE_DIR} 
	${EIGEN_INCLUDE_DIR} 
	${AVOGADRO_INCLUDE_DIR} 
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

### gamess
set(gamess_SRCS gamess.cpp gamessinputdialog.cpp gamessinputdata.cpp)
qt4_wrap_ui(gamess_UIS_H gamessinputdialog.ui)
qt4_automoc(${gamess_SRCS})
ADD_LIBRARY(gamess SHARED ${gamess_SRCS} ${gamess_UIS_H})
TARGET_LINK_LIBRARIES(gamess 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS gamess DESTINATION ${DESTINATION_DIR})

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
INSTALL(TARGETS ghemical DESTINATION ${DESTINATION_DIR})

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
INSTALL(TARGETS hydrogens DESTINATION ${DESTINATION_DIR})

