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
set(gamessextension_SRCS gamessextension.cpp gamessinputdialog.cpp gamessinputdata.cpp ../extension.cpp)
qt4_wrap_ui(gamessextension_UIS_H gamessinputdialog.ui)
qt4_automoc(${gamessextension_SRCS})
ADD_LIBRARY(gamessextension SHARED ${gamessextension_SRCS} ${gamessextension_UIS_H})
TARGET_LINK_LIBRARIES(gamessextension 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS gamessextension DESTINATION ${DESTINATION_DIR})

### ghemical
set(ghemicalextension_SRCS ghemicalextension.cpp ../extension.cpp)
qt4_automoc(${ghemicalextension_SRCS})
ADD_LIBRARY(ghemicalextension SHARED ${ghemicalextension_SRCS})
TARGET_LINK_LIBRARIES(ghemicalextension 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS ghemicalextension DESTINATION ${DESTINATION_DIR})

### hydrogens
set(hydrogensextension_SRCS hydrogensextension.cpp ../extension.cpp)
qt4_automoc(${hydrogensextension_SRCS})
ADD_LIBRARY(hydrogensextension SHARED ${hydrogensextension_SRCS})
TARGET_LINK_LIBRARIES(hydrogensextension 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS hydrogensextension DESTINATION ${DESTINATION_DIR})

