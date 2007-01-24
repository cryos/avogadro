set(QT_USE_QTOPENGL TRUE)

include(${QT_USE_FILE})

include_directories(
	${OPENBABEL2_INCLUDE_DIR} 
	${EIGEN_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${libavogadro_SOURCE_DIR}/include
)

set(libavogadro_SRCS
	color.cpp
	primitives.cpp
	cylinder.cpp
	sphere.cpp
	glwidget.cpp
	moleculetreeview.cpp
)

set(libavogadro_HDRS
	color.h
	primitives.h
	engine.h
	cylinder.h
	sphere.h
	glwidget.h
	moleculetreeview.h
)

set(libavogadro_MOC_HDRS
	primitives.h
	glwidget.h
	moleculetreeview.h
	elementcombobox.h
)

#qt4_automoc(${libavogadro_SRCS})
qt4_wrap_cpp(libavogadro_MOC_SRCS ${libavogadro_MOC_HDRS})

ADD_LIBRARY(avogadro-lib SHARED ${libavogadro_SRCS} ${libavogadro_MOC_SRCS})
SET_TARGET_PROPERTIES(avogadro-lib PROPERTIES OUTPUT_NAME avogadro)

INSTALL(TARGETS avogadro-lib DESTINATION lib)
INSTALL(FILES ${libavogadro_HDRS} DESTINATION include/avogadro)

add_subdirectory(engines)
