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

file(GLOB libavogadro_SRCS "*.cpp")

# Only headers with exported symbols should actually be installed
set(libavogadro_HDRS
  atom.h
  bond.h
  camera.h
  color.h
  colorbutton.h
  cube.h
  elementtranslate.h
  engine.h
  extension.h
  filetreeitem.h
  fragment.h
  glgraphicsview.h
  glwidget.h
  global.h
  idlist.h
  line.h
  mesh.h
  meshgenerator.h
  molecule.h
  navigate.h
  neighborlist.h
  painter.h
  painterdevice.h
  periodictableview.h
  plugin.h
  pluginmanager.h
  point.h
  primitive.h
  primitiveitemmodel.h
  primitivelist.h
  residue.h
  tool.h
  toolgroup.h
  undosequence.h
)

qt4_wrap_ui(libavogadro_UIS pluginsettings.ui)
set(libavogadro_SRCS ${libavogadro_SRCS} ${libavogadro_UIS})

# Statically link the core engines and tools so that users have a working
# system even if no plugins can be found.
set(bsdyengine_SRCS engines/bsdyengine.cpp)
qt4_wrap_ui(bsdyengine_UIS engines/bsdysettingswidget.ui)
set(libavogadro_SRCS ${libavogadro_SRCS} ${bsdyengine_SRCS} ${bsdyengine_UIS})

set(elementcolor_SRCS colors/elementcolor.cpp)
set(libavogadro_SRCS ${libavogadro_SRCS} ${elementcolor_SRCS})

set(AVO_LINK_LIBRARIES ${OPENBABEL2_LIBRARIES}
                      ${QT_LIBRARIES}
                      ${OPENGL_LIBRARIES}
                      )

if(GLEW_FOUND)
  include_directories(${GLEW_INCLUDE_DIR})
  link_directories(${GLEW_LIBRARY_DIR})
  set(AVO_LINK_LIBRARIES ${AVO_LINK_LIBRARIES} ${GLEW_LIBRARY})
endif(GLEW_FOUND)

# If Boost Python isn't found remove the interpreter files
if(NOT ALL_PYTHON_FOUND)
  list(REMOVE_ITEM libavogadro_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterpreter.cpp)
  list(REMOVE_ITEM libavogadro_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.cpp)
  list(REMOVE_ITEM libavogadro_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/pythonerror.cpp)
endif(NOT ALL_PYTHON_FOUND)

qt4_automoc(${libavogadro_SRCS})

SET(libavogadro_TS
  ${CMAKE_SOURCE_DIR}/i18n/avogadro_de.ts
  ${CMAKE_SOURCE_DIR}/i18n/avogadro_fr.ts
  ${CMAKE_SOURCE_DIR}/i18n/avogadro_en_GB.ts
)
QT4_WRAP_TS(libavogadro_QM ${libavogadro_TS})

# you have to add link_directories before you add the target
if(ALL_PYTHON_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
  qt4_add_resources(pythontool_RC_SRCS pythontool.qrc)
endif(ALL_PYTHON_FOUND)

add_library(avogadro-lib SHARED ${libavogadro_SRCS} ${libavogadro_QM} ${pythontool_RC_SRCS})
set_target_properties(avogadro-lib PROPERTIES OUTPUT_NAME avogadro)
set_target_properties(avogadro-lib PROPERTIES VERSION ${VERSION} SOVERSION 0 )

if(ALL_PYTHON_FOUND)
  # The Python headers, only installed if Python is enabled
  set(python_HDRS
    pythoninterpreter.h
    pythonscript.h
    pythonerror.h
  )

  #message(STATUS "Setting new boost python libraries")
  set(AVO_LINK_LIBRARIES ${AVO_LINK_LIBRARIES} ${PYTHON_LIBRARIES}
                         ${Boost_LIBRARIES})
  set(libavogadro_HDRS ${libavogadro_HDRS} ${python_HDRS})
endif(ALL_PYTHON_FOUND)

target_link_libraries(avogadro-lib ${AVO_LINK_LIBRARIES})

install(TARGETS avogadro-lib DESTINATION ${LIB_INSTALL_DIR})
install(FILES ${libavogadro_HDRS} DESTINATION include/avogadro)
install(FILES ${libavogadro_QM} DESTINATION share/libavogadro/i18n)

# Add the engiens, tools and extensions directories
add_subdirectory(engines)
add_subdirectory(tools)
add_subdirectory(extensions)
add_subdirectory(colors)

# Add the boost python module if the boost::python was found
if(ALL_PYTHON_FOUND)
  add_subdirectory(python)
endif(ALL_PYTHON_FOUND)
