set(QT_USE_QTOPENGL TRUE)

LINK_DIRECTORIES(${LINK_DIRECTORIES} ${libavogadro_BINARY_DIR}/src)

include(${QT_USE_FILE})

set(avogadro_SRCS
	mainwindow.cpp
	main.cpp
	tool.cpp
)

include_directories(${OPENBABEL2_INCLUDE_DIR} ${EIGEN_INCLUDE_DIR})

qt4_automoc(${avogadro_SRCS})

ADD_EXECUTABLE(avogadro-app ${avogadro_SRCS})

SET_TARGET_PROPERTIES(avogadro-app PROPERTIES OUTPUT_NAME avogadro)

TARGET_LINK_LIBRARIES(avogadro-app ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES} avogadro)

INSTALL(TARGETS avogadro-app DESTINATION bin)

add_subdirectory(tools)

