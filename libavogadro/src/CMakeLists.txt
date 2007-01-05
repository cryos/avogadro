set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

set(libavogadro_SRCS
	glwidget.cpp
	color.cpp
	primitives.cpp
	cylinder.cpp
	sphere.cpp
)

set(libavogadro_HDRS
	glwidget.h
	color.h
	primitives.h
	engine.h
	cylinder.h
	sphere.h
)

include_directories(${OPENBABEL2_INCLUDE_DIR} ${EIGEN_INCLUDE_DIR})

qt4_automoc(${libavogadro_SRCS})

ADD_LIBRARY(avogadro-lib SHARED ${libavogadro_SRCS})
SET_TARGET_PROPERTIES(avogadro-lib PROPERTIES OUTPUT_NAME avogadro)

INSTALL(TARGETS avogadro-lib DESTINATION lib)
INSTALL(FILES ${libavogadro_HDRS} DESTINATION include/avogadro)

add_subdirectory(engines)
