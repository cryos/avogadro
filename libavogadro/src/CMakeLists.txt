set(QT_USE_QTOPENGL true)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include(${QT_USE_FILE})

include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${OPENBABEL2_INCLUDE_DIR}
  ${BOOST_PYTHON_INCLUDES}
  ${PYTHON_INCLUDE_PATH}
)

# I think this is necessary now in order to tell the linker where openbabel is
link_directories(${OPENBABEL2_LIBRARY_DIRS})

FILE(GLOB libavogadro_SRCS "*.cpp")
FILE(GLOB libavogadro_HDRS "*.h")

qt4_wrap_ui(libavogadro_UIS plugindialog.ui)
set(libavogadro_SRCS ${libavogadro_SRCS} ${libavogadro_UIS})

# Statically link the core engines and tools so that users have a working
# system even if no plugins can be found.
set(bsdyengine_SRCS engines/bsdyengine.cpp)
qt4_wrap_ui(bsdyengine_UIS engines/bsdysettingswidget.ui)
set(libavogadro_SRCS ${libavogadro_SRCS} ${bsdyengine_SRCS} ${bsdyengine_UIS})

set(elementcolor_SRCS colors/elementcolor.cpp)
set(libavogadro_SRCS ${libavogadro_SRCS} ${elementcolor_SRCS})


# If Boost Python isn't found remove the interpreter files
if(NOT BOOST_PYTHON_LIBS AND NOT Boost_PYTHON_FOUND)
	LIST(REMOVE_ITEM libavogadro_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterpreter.cpp)
	LIST(REMOVE_ITEM libavogadro_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterpreter.h)
endif(NOT BOOST_PYTHON_LIBS AND NOT Boost_PYTHON_FOUND)

qt4_automoc(${libavogadro_SRCS})

SET(libavogadro_TS
	${CMAKE_SOURCE_DIR}/i18n/avogadro_de.ts
	${CMAKE_SOURCE_DIR}/i18n/avogadro_fr.ts
	${CMAKE_SOURCE_DIR}/i18n/avogadro_en_GB.ts
)
QT4_WRAP_TS(libavogadro_QM ${libavogadro_TS})

# you have to add link_directories before you add the target
if(Boost_PYTHON_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	link_directories(${Boost_LIBRARY_DIRS})
endif(Boost_PYTHON_FOUND)

ADD_LIBRARY(avogadro-lib SHARED ${libavogadro_SRCS} ${libavogadro_QM})
SET_TARGET_PROPERTIES(avogadro-lib PROPERTIES OUTPUT_NAME avogadro)
set_target_properties(avogadro-lib PROPERTIES VERSION ${VERSION} SOVERSION 0 )

if(BOOST_PYTHON_FOUND)
	TARGET_LINK_LIBRARIES(avogadro-lib ${OPENBABEL2_LIBRARIES}
				${QT_LIBRARIES}
				${OPENGL_LIBRARIES}
				${BOOST_PYTHON_LIBS})
else(BOOST_PYTHON_FOUND)
	if(Boost_PYTHON_FOUND)
		message(STATUS "Setting new boost python libraries")
		TARGET_LINK_LIBRARIES(avogadro-lib ${OPENBABEL2_LIBRARIES}
			${QT_LIBRARIES}
			${OPENGL_LIBRARIES}
			${PYTHON_LIBRARIES}
			${Boost_LIBRARIES})
	else(Boost_PYTHON_FOUND)
		TARGET_LINK_LIBRARIES(avogadro-lib ${OPENBABEL2_LIBRARIES}
			${QT_LIBRARIES}
			${OPENGL_LIBRARIES})
	endif(Boost_PYTHON_FOUND)
endif(BOOST_PYTHON_FOUND)

INSTALL(TARGETS avogadro-lib DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${libavogadro_HDRS} DESTINATION include/avogadro)
INSTALL(FILES ${libavogadro_QM} DESTINATION share/libavogadro/i18n)

# Add the engiens, tools and extensions directories
add_subdirectory(engines)
add_subdirectory(tools)
add_subdirectory(extensions)
add_subdirectory(colors)
