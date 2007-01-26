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


qt4_add_resources(draw_RC_SRCS draw.qrc)
qt4_wrap_cpp(draw_MOC_SRCS draw.h)
ADD_LIBRARY(draw SHARED draw.cpp ${draw_RC_SRCS} ${draw_MOC_SRCS})
TARGET_LINK_LIBRARIES(draw ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES} ${AVOGADRO_LIBRARIES})
INSTALL(TARGETS draw DESTINATION lib/avogadro)


qt4_wrap_cpp(selectrotate_MOC_SRCS selectrotate.h)
ADD_LIBRARY(selectrotate SHARED 
	selectrotate.cpp 
	${selectrotate_RC_SRCS} 
	${selectrotate_MOC_SRCS}
)
TARGET_LINK_LIBRARIES(selectrotate 
	${OPENBABEL2_LIBRARIES} 
	${QT_LIBRARIES} 
	${AVOGADRO_LIBRARIES}
)
INSTALL(TARGETS selectrotate DESTINATION lib/avogadro)

