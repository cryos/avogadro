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
  ${EIGEN_INCLUDE_DIR}
)

FILE(GLOB libavogadro_SRCS "*.cpp")
FILE(GLOB libavogadro_HDRS "*.h")

qt4_automoc(${libavogadro_SRCS})

SET(libavogadro_TS 
	${CMAKE_SOURCE_DIR}/i18n/avogadro_de.ts
	${CMAKE_SOURCE_DIR}/i18n/avogadro_fr.ts
) 
QT4_WRAP_TS(libavogadro_QM ${libavogadro_TS})

ADD_LIBRARY(avogadro-lib SHARED ${libavogadro_SRCS} ${libavogadro_QM})
SET_TARGET_PROPERTIES(avogadro-lib PROPERTIES OUTPUT_NAME avogadro)
set_target_properties(avogadro-lib PROPERTIES VERSION ${VERSION} SOVERSION 0 )

TARGET_LINK_LIBRARIES(avogadro-lib ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                   ${OPENGL_LIBRARIES}
    )

INSTALL(TARGETS avogadro-lib DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${libavogadro_HDRS} DESTINATION include/avogadro)
INSTALL(FILES ${libavogadro_QM} DESTINATION share/libavogadro/i18n)

add_subdirectory(engines)
add_subdirectory(tools)
