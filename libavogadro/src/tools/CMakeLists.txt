set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${LIB_INSTALL_DIR}/avogadro/tools)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

### drawtool
set(drawtool_SRCS drawtool.cpp  directorytreemodel.cpp
                                insertfragmentdialog.cpp
                                )
qt4_add_resources(drawtool_RC_SRCS drawtool.qrc)
qt4_automoc(${drawtool_SRCS})
qt4_wrap_ui(drawtool_UIS_H insertfragmentdialog.ui)
add_library(drawtool MODULE drawcommand.cpp
                    ${drawtool_SRCS}
                    ${drawtool_UIS_H}
                    ${drawtool_RC_SRCS})
target_link_libraries(drawtool ${LINK_LIBS})
install(TARGETS drawtool DESTINATION ${DESTINATION_DIR})

### bondcentrictool
set(bondcentrictool_SRCS bondcentrictool.cpp skeletontree.cpp)
qt4_add_resources(bondcentrictool_RC_SRCS bondcentrictool.qrc)
qt4_automoc(${bondcentrictool_SRCS})
add_library(bondcentrictool MODULE quaternion.cpp
    ${bondcentrictool_SRCS} ${bondcentrictool_RC_SRCS})
target_link_libraries(bondcentrictool ${LINK_LIBS})
install(TARGETS bondcentrictool DESTINATION ${DESTINATION_DIR})

### clickmeasuretool
set(clickmeasuretool_SRCS clickmeasuretool.cpp)
qt4_add_resources(clickmeasuretool_RC_SRCS clickmeasuretool.qrc)
qt4_automoc(${clickmeasuretool_SRCS})
add_library(clickmeasuretool MODULE
    ${clickmeasuretool_SRCS} ${clickmeasuretool_RC_SRCS} )
target_link_libraries(clickmeasuretool ${LINK_LIBS})
install(TARGETS clickmeasuretool DESTINATION ${DESTINATION_DIR})

### selectrotatetool
set(selectrotatetool_SRCS selectrotatetool.cpp)
qt4_add_resources(selectrotatetool_RC_SRCS selectrotatetool.qrc)
qt4_automoc(${selectrotatetool_SRCS})
add_library(selectrotatetool MODULE
    ${selectrotatetool_SRCS} ${selectrotatetool_RC_SRCS})
target_link_libraries(selectrotatetool ${LINK_LIBS})
install(TARGETS selectrotatetool DESTINATION ${DESTINATION_DIR})

### autoopttool
set(autoopttool_SRCS autoopttool.cpp)
qt4_add_resources(autoopttool_RC_SRCS autoopttool.qrc)
qt4_automoc(${autoopttool_SRCS})
add_library(autoopttool MODULE
    ${autoopttool_SRCS} ${autoopttool_RC_SRCS})
target_link_libraries(autoopttool ${LINK_LIBS})
install(TARGETS autoopttool DESTINATION ${DESTINATION_DIR})

### navigatetool
set(navigatetool_SRCS navigatetool.cpp)
qt4_add_resources(navigatetool_RC_SRCS navigatetool.qrc)
qt4_automoc(${navigatetool_SRCS})
add_library(navigatetool MODULE eyecandy.cpp
    ${navigatetool_SRCS} ${navigatetool_RC_SRCS})
target_link_libraries(navigatetool ${LINK_LIBS})
install(TARGETS navigatetool DESTINATION ${DESTINATION_DIR})

### manipulatetool
set(manipulatetool_SRCS manipulatetool.cpp)
qt4_add_resources(manipulatetool_RC_SRCS manipulatetool.qrc)
qt4_automoc(${manipulatetool_SRCS})
add_library(manipulatetool MODULE eyecandy.cpp
    ${manipulatetool_SRCS} ${manipulatetool_RC_SRCS})
target_link_libraries(manipulatetool ${LINK_LIBS})
install(TARGETS manipulatetool DESTINATION ${DESTINATION_DIR})

### autorotatetool
set(autorotatetool_SRCS autorotatetool.cpp)
qt4_add_resources(autorotatetool_RC_SRCS autorotatetool.qrc)
qt4_automoc(${autorotatetool_SRCS})
add_library(autorotatetool MODULE ${autorotatetool_SRCS}
    ${autorotatetool_RC_SRCS})
target_link_libraries(autorotatetool ${LINK_LIBS})
install(TARGETS autorotatetool DESTINATION ${DESTINATION_DIR})

### aligntool
set(aligntool_SRCS aligntool.cpp)
qt4_add_resources(aligntool_RC_SRCS aligntool.qrc)
qt4_automoc(${aligntool_SRCS})
add_library(aligntool MODULE ${aligntool_SRCS} ${aligntool_RC_SRCS})
target_link_libraries(aligntool ${LINK_LIBS})
install(TARGETS aligntool DESTINATION ${DESTINATION_DIR})
