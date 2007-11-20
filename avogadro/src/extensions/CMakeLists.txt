set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/extensions)

include_directories(
    ../
	${CMAKE_CURRENT_BINARY_DIR}
	${OPENBABEL2_INCLUDE_DIR}
	${EIGEN_INCLUDE_DIR}
	${AVOGADRO_INCLUDE_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

### gamess
set(gamessextension_SRCS gamessextension.cpp
  gamessinputdialog.cpp
  gamessinputdata.cpp
  gamessefpmatchdialog.cpp
)
qt4_wrap_ui(gamessextension_UIS_H gamessinputdialog.ui gamessefpmatchdialog.ui)
qt4_automoc(${gamessextension_SRCS})
ADD_LIBRARY(gamessextension SHARED ${gamessextension_SRCS} ${gamessextension_UIS_H})
TARGET_LINK_LIBRARIES(gamessextension
	${OPENBABEL2_LIBRARIES}
	${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS gamessextension DESTINATION ${DESTINATION_DIR})

### Molecular Mechanics force fields
set(forcefieldextension_SRCS forcefieldextension.cpp forcefielddialog.cpp)
qt4_wrap_ui(forcefieldextension_UIS_H forcefielddialog.ui)
qt4_automoc(${forcefieldextension_SRCS})
ADD_LIBRARY(forcefieldextension SHARED ${forcefieldextension_SRCS} ${forcefieldextension_UIS_H})
TARGET_LINK_LIBRARIES(forcefieldextension
	${OPENBABEL2_LIBRARIES}
	${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS forcefieldextension DESTINATION ${DESTINATION_DIR})

### hydrogens
set(hydrogensextension_SRCS hydrogensextension.cpp)
qt4_automoc(${hydrogensextension_SRCS})
ADD_LIBRARY(hydrogensextension SHARED ${hydrogensextension_SRCS})
TARGET_LINK_LIBRARIES(hydrogensextension
	${OPENBABEL2_LIBRARIES}
	${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS hydrogensextension DESTINATION ${DESTINATION_DIR})

### Selections
set(selectextension_SRCS selectextension.cpp)
qt4_automoc(${selectextension_SRCS})
ADD_LIBRARY(selectextension SHARED ${selectextension_SRCS})
TARGET_LINK_LIBRARIES(selectextension
	${OPENBABEL2_LIBRARIES}
	${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS selectextension DESTINATION ${DESTINATION_DIR})

### unit cells
set(unitcellextension_SRCS unitcellextension.cpp unitcellparamdialog.cpp)
qt4_wrap_ui(unitcellextension_UIS_H unitcellparamdialog.ui)
qt4_automoc(${unitcellextension_SRCS})
ADD_LIBRARY(unitcellextension SHARED ${unitcellextension_SRCS} ${unitcellextension_UIS_H})
TARGET_LINK_LIBRARIES(unitcellextension
	${OPENBABEL2_LIBRARIES}
	${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS unitcellextension DESTINATION ${DESTINATION_DIR})

### h->methyl
set(h2methylextension_SRCS h2methylextension.cpp)
qt4_automoc(${h2methylextension_SRCS})
ADD_LIBRARY(h2methylextension SHARED ${h2methylextension_SRCS})
TARGET_LINK_LIBRARIES(h2methylextension
	${OPENBABEL2_LIBRARIES}
	${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS h2methylextension DESTINATION ${DESTINATION_DIR})
