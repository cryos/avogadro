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

qt4_wrap_cpp(bsengine_MOC_SRCS bsengine.h)
ADD_LIBRARY(bsengine SHARED 
    bsengine.cpp 
    ${bsengine_MOC_SRCS}
)
TARGET_LINK_LIBRARIES(bsengine avogadro-lib
                                ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                ${OPENGL_LIBRARIES}
)
INSTALL(TARGETS bsengine DESTINATION lib/avogadro)
