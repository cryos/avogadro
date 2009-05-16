include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${Avogadro_PLUGIN_INSTALL_DIR}/tools)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro")

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
set(bondcentrictool_SRCS bondcentrictool.cpp skeletontree.cpp)
avogadro_plugin(bondcentrictool
  "${bondcentrictool_SRCS}"
  bondcentrictool.qrc)

### clickmeasuretool
avogadro_plugin(clickmeasuretool clickmeasuretool.cpp clickmeasuretool.qrc)

### selectrotatetool
avogadro_plugin(selectrotatetool selectrotatetool.cpp selectrotatetool.qrc)

### autoopttool
avogadro_plugin(autoopttool autoopttool.cpp autoopttool.qrc)

### navigatetool
avogadro_plugin(navigatetool
  "navigatetool.cpp;eyecandy.cpp" navigatetool.qrc)

### manipulatetool
avogadro_plugin(manipulatetool
  "manipulatetool.cpp;eyecandy.cpp" manipulatetool.qrc)

### autorotatetool
avogadro_plugin(autorotatetool autorotatetool.cpp autorotatetool.qrc)

### aligntool
avogadro_plugin(aligntool aligntool.cpp aligntool.qrc)

if(ALL_PYTHON_FOUND)
  file(GLOB python_SCRIPTS "python/*.py")
  install(FILES ${python_SCRIPTS} DESTINATION share/libavogadro/toolScripts)
endif(ALL_PYTHON_FOUND)

