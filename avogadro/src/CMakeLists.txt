set(QT_USE_QTOPENGL TRUE)

if(WIN32)
	set(QT_USE_QTMAIN TRUE)
endif(WIN32)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

LINK_DIRECTORIES(${LINK_DIRECTORIES} ${libavogadro_BINARY_DIR}/src)

include_directories(
	${libavogadro_SOURCE_DIR}/include
	${avogadro_SOURCE_DIR}/src
	${avogadro_SOURCE_DIR}/include
	${OPENBABEL2_INCLUDE_DIR} 
	${EIGEN_INCLUDE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DQT_DEBUG)
include(${QT_USE_FILE})

set(avogadro_SRCS
    flowlayout.cpp
    flattabwidget.cpp
    enginelistview.cpp
    aboutdialog.cpp
    application.cpp
    mainwindow.cpp
    main.cpp
)

set(avogadro_RCS
    avogadro.qrc
)

set(avogadro_UIS
    mainwindow.ui
)

# designer has some flaws.
qt4_automoc(${avogadro_SRCS})
qt4_add_resources(avogadro_RC_SRCS ${avogadro_RCS})
qt4_wrap_ui(avogadro_UIS_H ${avogadro_UIS})

IF( APPLE )
  SET( MACOSX_BUNDLE_INFO_STRING "Avogadro - version 0.0.3" )
  SET( MACOSX_BUNDLE_BUNDLE_VERSION "0.0.3" )
  # Change following line to point to actual icns file in bundle.
  SET( MACOSX_BUNDLE_ICON_FILE "avogadro.icns" )
  SET( MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge" ) # <-- ???
  SET( MACOSX_BUNDLE_BUNDLE_NAME "Avogadro" )

  # Need to copy the icon file
  EXEC_PROGRAM( "mkdir -p 	${avogadro_BINARY_DIR}/src/Avogadro.app/Contents/Resources")
  EXEC_PROGRAM( "cp ${avogadro_SOURCE_DIR}/src/mac/avogadro.icns ${avogadro_BINARY_DIR}/src/Avogadro.app/Contents/Resources")

  # Overload the Info.plist default
  SET( CMAKE_MODULE_PATH ${avogadro_SOURCE_DIR}/src/mac ${CMAKE_MODULE_PATH} )
ENDIF( APPLE)

add_executable(avogadro-app WIN32 MACOSX_BUNDLE
	${avogadro_SRCS} 
	${avogadro_UIS_H} 
	${avogadro_RC_SRCS}	)

SET_TARGET_PROPERTIES(avogadro-app PROPERTIES OUTPUT_NAME avogadro)

TARGET_LINK_LIBRARIES(avogadro-app ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
    avogadro-lib)

INSTALL(TARGETS avogadro-app DESTINATION bin)

add_subdirectory(extensions)

