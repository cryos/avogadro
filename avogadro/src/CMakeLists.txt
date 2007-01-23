set(QT_USE_QTOPENGL TRUE)

LINK_DIRECTORIES(${LINK_DIRECTORIES} ${libavogadro_BINARY_DIR}/src)

include_directories(
	${OPENBABEL2_INCLUDE_DIR} 
	${EIGEN_INCLUDE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
	${libavogadro_SOURCE_DIR}/include
)
include(${QT_USE_FILE})

set(avogadro_SRCS
  flowlayout.cpp
	mainwindow.cpp
	main.cpp
#	tool.cpp
)

set(avogadro_MOC_HDRS
	mainwindow.h
)

set(avogadro_RCS
        avogadro.qrc
)

#set(avogadro_UIS
#	mainwindow.ui
#)


# designer has some flaws.
#qt4_wrap_ui(avogadro_UIS_H ${avogadro_UIS})
qt4_wrap_cpp(avogadro_MOC_SRCS ${avogadro_MOC_HDRS})
qt4_add_resources(avogadro_RC_SRCS ${avogadro_RCS})
#qt4_automoc(${avogadro_SRCS})

#ADD_EXECUTABLE(avogadro-app ${avogadro_SRCS} ${avogadro_UIS_H} ${avogadro_RC_SRCS})
ADD_EXECUTABLE(avogadro-app ${avogadro_SRCS} ${avogadro_MOC_SRCS} ${avogadro_RC_SRCS})

SET_TARGET_PROPERTIES(avogadro-app PROPERTIES OUTPUT_NAME avogadro)

TARGET_LINK_LIBRARIES(avogadro-app ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES} avogadro)

INSTALL(TARGETS avogadro-app DESTINATION bin)

add_subdirectory(tools)

