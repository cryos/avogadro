set(QT_USE_QTOPENGL TRUE)

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

set(libavogadro_SRCS
  camera.cpp
  color.cpp
  engine.cpp
  glwidget.cpp
  moleculetreeview.cpp
  primitive.cpp
  primitiveitemmodel.cpp
  primitivelist.cpp
  tool.cpp
  toolgroup.cpp
  undosequence.cpp
  painter.cpp
  textrenderer.cpp
  sphere.cpp
  cylinder.cpp
)

set(libavogadro_HDRS
  camera.h
  color.h
  engine.h
  glwidget.h
  global.h
  moleculetreeview.h
  primitive.h
  primitiveitemmodel.h
  primitivelist.h
  painter.h
  toolgroup.h
  undosequence.h
)

qt4_automoc(${libavogadro_SRCS})

#set(libavogadro_MOC_HDRS
  #primitive.h
  #glwidget.h
  #moleculetreeview.h
#)
#qt4_wrap_cpp(libavogadro_MOC_SRCS ${libavogadro_MOC_HDRS})

ADD_LIBRARY(avogadro-lib SHARED ${libavogadro_SRCS})
SET_TARGET_PROPERTIES(avogadro-lib PROPERTIES OUTPUT_NAME avogadro)
set_target_properties(avogadro-lib PROPERTIES VERSION 0.0.1 SOVERSION 0 )

TARGET_LINK_LIBRARIES(avogadro-lib ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
                                   ${OPENGL_LIBRARIES}                                   )

INSTALL(TARGETS avogadro-lib DESTINATION ${LIB_INSTALL_DIR})
INSTALL(FILES ${libavogadro_HDRS} DESTINATION include/avogadro)

add_subdirectory(engines)
add_subdirectory(tools)
