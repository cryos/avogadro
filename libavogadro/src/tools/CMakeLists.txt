include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${PLUGIN_INSTALL_DIR}/tools)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

### drawtool
set(drawtool_SRCS 
  drawtool.cpp
  drawcommand.cpp
  directorytreemodel.cpp
  insertfragmentdialog.cpp)
avogadro_plugin(drawtool 
  "${drawtool_SRCS}"
  insertfragmentdialog.ui
  drawtool.qrc)

option(ENABLE_ZMATRIX_TOOL "Enable the z-matrix tool (experimental)" OFF)
if(ENABLE_ZMATRIX_TOOL)
  ### zmatrixtool
  set(zmatrixtool_SRCS zmatrixtool.cpp zmatrixdialog.cpp zmatrixmodel.cpp)
  avogadro_plugin(zmatrixtool
    "${zmatrixtool_SRCS}"
    zmatrixdialog.ui)
endif(ENABLE_ZMATRIX_TOOL)

### bondcentrictool
qt4_add_resources(bondcentrictool_RC_SRCS bondcentrictool.qrc)
set(bondcentrictool_SRCS bondcentrictool.cpp skeletontree.cpp
  quaternion.cpp ${bondcentrictool_RC_SRCS})
avogadro_plugin(bondcentrictool
  "${bondcentrictool_SRCS}")

### clickmeasuretool
qt4_add_resources(clickmeasuretool_RC_SRCS clickmeasuretool.qrc)
avogadro_plugin(clickmeasuretool
  "clickmeasuretool.cpp;${clickmeasuretool_RC_SRCS}")

### selectrotatetool
qt4_add_resources(selectrotatetool_RC_SRCS selectrotatetool.qrc)
avogadro_plugin(selectrotatetool
  "selectrotatetool.cpp;${selectrotatetool_RC_SRCS}")

### autoopttool
qt4_add_resources(autoopttool_RC_SRCS autoopttool.qrc)
avogadro_plugin(autoopttool
  "autoopttool.cpp;${autoopttool_RC_SRCS}")

### navigatetool
qt4_add_resources(navigatetool_RC_SRCS navigatetool.qrc)
avogadro_plugin(navigatetool
  "navigatetool.cpp;eyecandy.cpp;${navigatetool_RC_SRCS}")

### manipulatetool
qt4_add_resources(manipulatetool_RC_SRCS manipulatetool.qrc)
avogadro_plugin(manipulatetool
  "manipulatetool.cpp;eyecandy.cpp;${manipulatetool_RC_SRCS}")

### autorotatetool
qt4_add_resources(autorotatetool_RC_SRCS autorotatetool.qrc)
avogadro_plugin(autorotatetool
  "autorotatetool.cpp;${autorotatetool_RC_SRCS}")

### aligntool
qt4_add_resources(aligntool_RC_SRCS aligntool.qrc)
avogadro_plugin(aligntool "aligntool.cpp;${aligntool_RC_SRCS}")

if(ALL_PYTHON_FOUND)
  file(GLOB python_SCRIPTS "python/*.py")
  install(FILES ${python_SCRIPTS} DESTINATION share/libavogadro/toolScripts)
endif(ALL_PYTHON_FOUND)

