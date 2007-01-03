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

include_directories(${OPENBABEL_INCLUDE_DIR})

# tell cmake to create .moc files for all files in the variable qtproject_SRCS that require such a file.
# note: this assumes that you use #include "header.moc" in your files
# also note that you don't actually require kde to use this command
qt4_automoc(${libavogadro_SRCS})


ADD_LIBRARY(avogadro SHARED ${libavogadro_SRCS})
# ADD_LIBRARY(avogadroStatic STATIC ${libavogadro_SRCS})
# ADD_LIBRARY(avogadroShared SHARED ${libavogadro_SRCS})
# SET_TARGET_PROPERTIES(avogadroStatic avogadroShared PROPERTIES OUTPUT_NAME avogadro)

# install(TARGETS avogadroShared DESTINATION lib)
# install(TARGETS avogadroShared DESTINATION lib)
INSTALL(TARGETS avogadro DESTINATION lib)
INSTALL(FILES ${libavogadro_HDRS} DESTINATION include/avogadro)

add_subdirectory(engines)
