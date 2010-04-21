set(QT_USE_QTOPENGL true)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include(${QT_USE_FILE})

# Ensure the Avogadro include directory is always first
get_directory_property(tmp_include_dirs INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES
  "${libavogadro_BINARY_DIR}/include;${tmp_include_dirs}")
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${EIGEN2_INCLUDE_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
)

# I think this is necessary now in order to tell the linker where openbabel is
link_directories(${OPENBABEL2_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib)

set(AVO_LINK_LIBRARIES ${OPENBABEL2_LIBRARIES}
                       ${QT_QTCORE_LIBRARY}
                       )

# Only headers with exported symbols should actually be installed
set(libavogadrocore_HDRS
  atom.h
  bond.h
  cube.h
  fragment.h
  global.h
  idlist.h
  mesh.h
  meshgenerator.h
  molecule.h
  moleculefile.h
  neighborlist.h
  primitive.h
  primitivelist.h
  protein.h
  residue.h
  zmatrix.h
)

set(libavogadrocore_SRCS
  atom.cpp
  bond.cpp
  cube.cpp
  fragment.cpp
  global.cpp
  idlist.cpp
  mesh.cpp
  meshgenerator.cpp
  molecule.cpp
  moleculefile.cpp
  neighborlist.cpp
  primitive.cpp
  primitivelist.cpp
  protein.cpp
  residue.cpp
  zmatrix.cpp
)

set(libavogadrowidget_HDRS
  animation.h
  camera.h
  color3f.h
  color.h
  colorbutton.h
  dockextension.h
  engine.h
  extension.h
  glhit.h
  glwidget.h
  navigate.h
  painter.h
  painterdevice.h
  plugin.h
  pluginmanager.h
  tool.h
  toolgroup.h
  undosequence.h
)

set(libavogadrowidget_SRCS
  ../gl2ps/gl2ps.c
  animation.cpp
  camera.cpp
  color3f.cpp
  color.cpp
  colorbutton.cpp
  cylinder_p.cpp
  dockextension.cpp
  engine.cpp
  extension.cpp
  glhit.cpp
  glpainter_p.cpp
  glwidget.cpp
  navigate.cpp
  painter.cpp
  painterdevice.cpp
  plugin.cpp
  pluginmanager.cpp
  sphere_p.cpp
  textrenderer_p.cpp
  tool.cpp
  toolgroup.cpp
  undosequence.cpp
)

set(libavogadropython_HDRS
  pythoninterpreter.h
  pythonscript.h
  pythonerror.h
)

set(libavogadropython_SRCS
  pythoninterpreter.cpp
  pythonscript.cpp
  pythonerror.cpp
  pythontool_p.cpp
  pythonengine_p.cpp
  pythonextension_p.cpp
)

add_library(avogadrocore SHARED
  ${libavogadrocore_SRCS})
set_source_files_properties(${libavogadrocore_SRCS}
  PROPERTIES LABELS avogadrocore)
set_target_properties(avogadrocore PROPERTIES 
  VERSION ${Avogadro_VERSION_FULL}
  LINK_INTERFACE_LIBRARIES ""
  SOVERSION 1
  OUTPUT_NAME AvogadroCore
  LABELS avogadrocore)
target_link_libraries(avogadrocore
  ${AVO_LINK_LIBRARIES})

set(AVO_LINK_LIBRARIES ${OPENBABEL2_LIBRARIES}
                       ${QT_LIBRARIES}
                       ${OPENGL_LIBRARIES}
                       )

if(GLEW_FOUND)
  include_directories(${GLEW_INCLUDE_DIR})
  link_directories(${GLEW_LIBRARY_DIR})
  set(AVO_LINK_LIBRARIES ${AVO_LINK_LIBRARIES} ${GLEW_LIBRARY})
endif(GLEW_FOUND)

# Library translations
if(NOT WIN32)
  if(Linguist_FOUND)
    file(GLOB_RECURSE libavogadro_PO "${Avogadro_SOURCE_DIR}/i18n/libavogadro/libavogadro*.po")
    QT4_WRAP_PO(libavogadro_QM ${libavogadro_PO})
  endif(Linguist_FOUND)
else(NOT WIN32)
  # don't attempt to create QM files on windows -- copy them from a linux build!
  file(GLOB_RECURSE libavogadro_QM "${Avogadro_SOURCE_DIR}/libavogadro/src/libavogadro*.qm")
endif(NOT WIN32)

set(libavogadro_HDRS ${libavogadrocore_HDRS} ${libavogadrowidget_HDRS})
file(GLOB libavogadro_SRCS ${libavogadrowidget_SRCS})
#set(libavogadro_HDRS ${libavogadro_HDRS} "../gl2ps/gl2ps.h")

#set(libavogadro_SRCS ${libavogadro_SRCS} ${libavogadro_UIS})
# Also add in the qtiocompressor class
#set(libavogadro_SRCS ${libavogadro_SRCS} ../qtiocompressor/qtiocompressor.cpp)

# Statically link the core engines and tools so that users have a working
# system even if no plugins can be found.
set(bsdyengine_SRCS engines/bsdyengine.cpp)
qt4_wrap_ui(bsdyengine_UIS engines/bsdysettingswidget.ui)
set(libavogadro_SRCS ${libavogadro_SRCS} ${bsdyengine_SRCS} ${bsdyengine_UIS})

set(elementcolor_SRCS colors/elementcolor.cpp)
set(libavogadro_SRCS ${libavogadro_SRCS} ${elementcolor_SRCS})

# If python dependencies aren't found, remove the Avogadro Python source files
#if(NOT ENABLE_PYTHON OR NOT ALL_PYTHON_FOUND)
#  set(PYTHON_SRCS
#    ${CMAKE_CURRENT_SOURCE_DIR}/pythoninterpreter.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/pythonscript.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/pythonerror.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/pythontool_p.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/pythonengine_p.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/pythonextension_p.cpp)
#  foreach(P_ITEM ${PYTHON_SRCS})
#    list(REMOVE_ITEM libavogadro_SRCS "${P_ITEM}")
#  endforeach(P_ITEM ${PYTHON_SRCS})
#endif(NOT ENABLE_PYTHON OR NOT ALL_PYTHON_FOUND)

qt4_automoc(${libavogadro_SRCS})

# you have to add link_directories before you add the target
if(ENABLE_PYTHON AND ALL_PYTHON_FOUND)
  include_directories(
    ${Boost_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_PATH})
  qt4_add_resources(pythontool_RC_SRCS pythontool.qrc)

  message(STATUS "Setting new boost python libraries")
  set(AVO_LINK_LIBRARIES ${AVO_LINK_LIBRARIES} ${PYTHON_LIBRARIES}
                         ${Boost_LIBRARIES})
  set(libavogadro_HDRS ${libavogadro_HDRS} ${libavogadropython_HDRS})
  set(libavogadro_SRCS ${libavogadro_SRCS} ${libavogadropython_SRCS})
endif(ENABLE_PYTHON AND ALL_PYTHON_FOUND)

# Set up the include directory and symlink/copy all headers to it.
# Note: Windows lacks symlinks and so the files must be copied.
make_directory("${libavogadro_BINARY_DIR}/include/avogadro")
if(UNIX)
  set(header_cmd "create_symlink")
else(UNIX)
  set(header_cmd "copy_if_different")
endif(UNIX)
foreach(headerFile ${libavogadro_HDRS})
  set(from "${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}")
  set(to   "${libavogadro_BINARY_DIR}/include/avogadro/${headerFile}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E ${header_cmd} "${from}" "${to}")
endforeach(headerFile ${libavogadro_HDRS})

add_library(avogadro SHARED
  ${libavogadro_SRCS} ${libavogadro_QM} ${pythontool_RC_SRCS})
set_source_files_properties(${libavogadro_SRCS}
  PROPERTIES LABELS avogadro)
set_target_properties(avogadro PROPERTIES 
  VERSION ${Avogadro_VERSION_FULL}
  LINK_INTERFACE_LIBRARIES avogadrocore
  SOVERSION 1
  LABELS avogadro)
target_link_libraries(avogadro avogadrocore
  ${AVO_LINK_LIBRARIES})

install(TARGETS avogadrocore avogadro DESTINATION ${LIB_INSTALL_DIR} EXPORT AvogadroTargets)
install(FILES ${libavogadro_HDRS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro")
install(FILES ${libavogadrocore_HDRS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro/AvogadroCore")
install(FILES ${libavogadrowidget_HDRS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro/AvogadroWidget")
install(FILES ${libavogadro_QM} DESTINATION ${I18N_INSTALL_DIR})

# Add the engiens, tools and extensions directories
add_subdirectory(engines)
add_subdirectory(tools)
add_subdirectory(extensions)
add_subdirectory(colors)

# Add the boost python module if the boost::python was found
if(ENABLE_PYTHON AND ALL_PYTHON_FOUND)
  install(FILES ${libavogadropython_HDRS}
    DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro/AvogadroPython")
  add_subdirectory(python)
endif(ENABLE_PYTHON AND ALL_PYTHON_FOUND)

install(EXPORT AvogadroTargets DESTINATION ${LIB_INSTALL_DIR}/avogadro)

