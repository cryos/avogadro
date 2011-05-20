set(QT_USE_QTOPENGL true)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h)

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
link_directories(${OPENBABEL2_LIBRARY_DIRS})

# Only headers with exported symbols should actually be installed
set(libavogadro_HDRS
  animation.h
  atom.h
  bond.h
  camera.h
  color3f.h
  colorbutton.h
  color.h
  cube.h
  dockextension.h
  elementtranslator.h
  engine.h
  extension.h
  fragment.h
  glhit.h
  global.h
  glwidget.h
  idlist.h
  meshgenerator.h
  mesh.h
  moleculefile.h
  molecule.h
  navigate.h
  neighborlist.h
  painterdevice.h
  painter.h
  periodictableview.h
  plotaxis.h
  plotobject.h
  plotpoint.h
  plotwidget.h
  plugin.h
  pluginmanager.h
  primitive.h
  primitivelist.h
  protein.h
  residue.h
  toolgroup.h
  tool.h
  undosequence.h
  zmatrix.h
)

set(libavogadro_SRCS
  animation.cpp
  atom.cpp
  bond.cpp
  camera.cpp
  color.cpp
  colorbutton.cpp
  cube.cpp
  cylinder_p.cpp
  dockextension.cpp
  elementdetail_p.cpp
  elementitem_p.cpp
  elementtranslator.cpp
  engine.cpp
  extension.cpp
  fragment.cpp
  glhit.cpp
  global.cpp
  glpainter_p.cpp
  glwidget.cpp
  idlist.cpp
  mesh.cpp
  meshgenerator.cpp
  molecule.cpp
  moleculefile.cpp
  navigate.cpp
  neighborlist.cpp
  painter.cpp
  periodictablescene_p.cpp
  periodictableview.cpp
  plotaxis.cpp
  plotobject.cpp
  plotpoint.cpp
  plotwidget.cpp
  plugin.cpp
  pluginmanager.cpp
  primitive.cpp
  primitivelist.cpp
  protein.cpp
  readfilethread_p.cpp
  residue.cpp
  sphere_p.cpp
  textrenderer_p.cpp
  tool.cpp
  toolgroup.cpp
  undosequence.cpp
  zmatrix.cpp
)

set(libavogadro_MOC_HDRS
  animation.h
  atom.h
  bond.h
  color.h
  colorbutton.h
  cube.h
  dockextension.h
  elementdetail_p.h
  elementtranslator.h
  engine.h
  extension.h
  fragment.h
  glwidget.h
  mesh.h
  meshgenerator.h
  molecule.h
  moleculefile.h
  periodictablescene_p.h
  periodictableview.h
  plotwidget.h
  plugin.h
  pluginmanager.h
  primitive.h
  protein.h
  readfilethread_p.h
  residue.h
  tool.h
  toolgroup.h
  zmatrix.h
)

set(AVO_LINK_LIBRARIES
  ${OPENBABEL2_LIBRARIES}
  ${QT_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

if(GLEW_FOUND)
  include_directories(${GLEW_INCLUDE_DIR})
  list(APPEND AVO_LINK_LIBRARIES ${GLEW_LIBRARY})
endif()

# Library translations
if(NOT WIN32)
  if(Linguist_FOUND)
    file(GLOB_RECURSE lang_PO "${Avogadro_SOURCE_DIR}/i18n/libavogadro/*.po")
    foreach(po ${lang_PO})
      string(REGEX REPLACE "(.*)/([^.]+)\\.po" "\\1/libavogadro-\\2.po" po ${po})
      list(APPEND libavogadro_PO ${po})
    endforeach()
    QT4_WRAP_PO(libavogadro_QM ${libavogadro_PO})
  endif()
else()
  # don't attempt to create QM files on windows -- copy them from a linux build!
  file(GLOB_RECURSE libavogadro_QM
    "${Avogadro_SOURCE_DIR}/libavogadro/src/libavogadro*.qm")
endif()

# you have to add link_directories before you add the target
if(ENABLE_PYTHON AND ALL_PYTHON_FOUND)
  message(STATUS "Setting new boost python libraries")
  include_directories(
    ${Boost_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_PATH})
  qt4_add_resources(pythontool_RC_SRCS pythontool.qrc)
  list(APPEND AVO_LINK_LIBRARIES ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
  list(APPEND libavogadro_HDRS
    pythoninterpreter.h
    pythonscript.h
    pythonerror.h)
  list(APPEND libavogadro_MOC_HDRS
    pythonengine_p.h
    pythonerror.h
    pythonextension_p.h
    pythontool_p.h)
  list(APPEND libavogadro_SRCS
    pythoninterpreter.cpp
    pythonscript.cpp
    pythonerror.cpp
    pythontool_p.cpp
    pythonengine_p.cpp
    pythonextension_p.cpp)
endif()

qt4_wrap_cpp(libavogadro_MOC ${libavogadro_MOC_HDRS})

# Set up the include directory and symlink/copy all headers to it.
# Note: Windows lacks symlinks and so the files must be copied.
make_directory("${libavogadro_BINARY_DIR}/include/avogadro")
if(UNIX)
  set(header_cmd "create_symlink")
else()
  set(header_cmd "copy_if_different")
endif()
foreach(headerFile ${libavogadro_HDRS})
  set(from "${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}")
  set(to   "${libavogadro_BINARY_DIR}/include/avogadro/${headerFile}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E ${header_cmd} "${from}" "${to}")
endforeach()

add_library(avogadro SHARED
  ${libavogadro_SRCS}
  ${libavogadro_MOC}
  ${libavogadro_QM}
  ${pythontool_RC_SRCS})
set_target_properties(avogadro PROPERTIES
  VERSION ${Avogadro_VERSION_FULL}
  LINK_INTERFACE_LIBRARIES ""
  SOVERSION ${AVO_SOVERSION}
  LABELS avogadro)
install(FILES ${libavogadro_HDRS}
  DESTINATION "${INCLUDE_INSTALL_DIR}/avogadro")
install(TARGETS avogadro DESTINATION ${LIB_INSTALL_DIR} EXPORT AvogadroTargets)

# Add the engiens, tools and extensions directories
add_subdirectory(engines)
add_subdirectory(tools)
add_subdirectory(extensions)
add_subdirectory(colors)

set(static_plugin_instantiation "#include <QtCore/QCoreApplication>\n")
foreach(plugin ${plugins_built_static})
  set(static_plugin_instantiation "${static_plugin_instantiation}
Q_IMPORT_PLUGIN(${plugin})")
endforeach()
set(static_plugin_instantiation "${static_plugin_instantiation}
void initAvogadroResources()
{
")
foreach(plugin ${plugins_rcc_static})
  set(static_plugin_instantiation "${static_plugin_instantiation}
Q_INIT_RESOURCE(${plugin});")
endforeach()
set(static_plugin_instantiation "${static_plugin_instantiation}
}
")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/staticplugins.cpp.in
  "${static_plugin_instantiation}\n")
configure_file(${CMAKE_CURRENT_BINARY_DIR}/staticplugins.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/staticplugins.cpp)

target_link_libraries(avogadro ${AVO_LINK_LIBRARIES} ${plugins_built_static})
message(STATUS "Linking in static plugins: ${plugins_built_static}")
# Add the boost python module if the boost::python was found
if(ENABLE_PYTHON AND ALL_PYTHON_FOUND)
  add_subdirectory(python)
endif()

install(FILES ${libavogadro_QM} DESTINATION ${I18N_INSTALL_DIR})

if(NOT APPLE)
  install(EXPORT AvogadroTargets DESTINATION ${LIB_INSTALL_DIR}/avogadro)
endif()
