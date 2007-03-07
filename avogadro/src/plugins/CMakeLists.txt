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
qt4_add_resources(draw_RC_SRCS draw.qrc)
qt4_wrap_cpp(draw_MOC_SRCS draw.h)
ADD_LIBRARY(draw SHARED 
    draw.cpp 
    ${draw_RC_SRCS} 
    ${draw_MOC_SRCS}
)
TARGET_LINK_LIBRARIES(draw 
    ${OPENBABEL2_LIBRARIES} 
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES} 
    avogadro-lib)
INSTALL(TARGETS draw DESTINATION lib/avogadro)

### gamess
qt4_wrap_cpp(gamess_MOC_SRCS gamess.h gamessinputdialog.h)
qt4_wrap_ui(gamess_UIS_H gamessinputdialog.ui)
ADD_LIBRARY(gamess SHARED 
	gamess.cpp 
	gamessinputdialog.cpp 
    gamessinputdata.cpp
	${gamess_MOC_SRCS}
    ${gamess_UIS_H}
)
TARGET_LINK_LIBRARIES(gamess 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS gamess DESTINATION lib/avogadro)

### ghemical
qt4_wrap_cpp(ghemical_MOC_SRCS ghemical.h)
ADD_LIBRARY(ghemical SHARED 
	ghemical.cpp 
	${ghemical_MOC_SRCS}
)
TARGET_LINK_LIBRARIES(ghemical 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS ghemical DESTINATION lib/avogadro)

### hydrogens
qt4_wrap_cpp(hydrogens_MOC_SRCS hydrogens.h)
ADD_LIBRARY(hydrogens SHARED 
	hydrogens.cpp 
	${hydrogens_MOC_SRCS}
)
TARGET_LINK_LIBRARIES(hydrogens 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS hydrogens DESTINATION lib/avogadro)

### selectrotate
qt4_wrap_cpp(selectrotate_MOC_SRCS selectrotate.h)
ADD_LIBRARY(selectrotate SHARED 
	selectrotate.cpp 
	${selectrotate_MOC_SRCS}
)
TARGET_LINK_LIBRARIES(selectrotate 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
    ${OPENGL_LIBRARIES}
	avogadro-lib
)
INSTALL(TARGETS selectrotate DESTINATION lib/avogadro)

