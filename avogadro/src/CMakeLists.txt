set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

set(avogadro_SRCS
	mainwindow.cpp
	main.cpp
	tool.cpp
)

include_directories(${OPENBABEL2_INCLUDE_DIR} ${EIGEN_INCLUDE_DIR} ${AVOGADRO_INCLUDE_DIR})

qt4_automoc(${avogadro_SRCS})

ADD_EXECUTABLE(avogadro ${avogadro_SRCS})

TARGET_LINK_LIBRARIES(avogadro ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES} ${AVOGADRO_LIBRARIES})

INSTALL(TARGETS avogadro DESTINATION bin)

#add_subdirectory(engines)
