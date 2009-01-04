set(QT_USE_QTOPENGL true)

if(WIN32)
  set(QT_USE_QTMAIN true)
endif(WIN32)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

link_directories(
  ${libavogadro_BINARY_DIR}/src
  ${OPENBABEL2_LIBRARY_DIRS}
)

include_directories(
  ${libavogadro_SOURCE_DIR}/include
  ${avogadro_SOURCE_DIR}/src
  ${avogadro_SOURCE_DIR}/include
  ${OPENBABEL2_INCLUDE_DIR}
  ${EIGEN2_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

include(${QT_USE_FILE})

FILE(GLOB avogadro_SRCS "*.cpp")

FILE(GLOB projectDelegates_SRCS "projectdelegates/*.cpp")
set(avogadro_SRCS ${avogadro_SRCS} ${projectDelegates_SRCS})
# Add GL2PS to the executable
set(avogadro_SRCS ${avogadro_SRCS} "../gl2ps/gl2ps.c")

if( WIN32 )
  set(avogadro_SRCS ${avogadro_SRCS} windows/avogadro.rc)
endif( WIN32)

FILE(GLOB avogadro_RCS "*.qrc")

FILE(GLOB avogadro_UIS "*.ui")

# designer has some flaws.
qt4_automoc(${avogadro_SRCS})
qt4_add_resources(avogadro_RC_SRCS ${avogadro_RCS})
qt4_wrap_ui(avogadro_UIS_H ${avogadro_UIS})

FILE(GLOB avogadro_TS "${CMAKE_SOURCE_DIR}/i18n/*.ts")

QT4_WRAP_TS(avogadro_QM ${avogadro_TS})

add_executable(avogadro-app WIN32 MACOSX_BUNDLE
  ${avogadro_SRCS}
  ${avogadro_UIS_H}
  ${avogadro_RC_SRCS}
  ${avogadro_QM}
)

SET_TARGET_PROPERTIES(avogadro-app PROPERTIES OUTPUT_NAME avogadro)

TARGET_LINK_LIBRARIES(avogadro-app ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
    avogadro-lib)

IF( APPLE )
  SET( MACOSX_BUNDLE_INFO_STRING "Avogadro - version ${VERSION}" )
  SET( MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION} )
  # Change following line to point to actual icns file in bundle.
  SET( MACOSX_BUNDLE_ICON_FILE "avogadro.icns" )
  # means bundle is set to be net.sourceforge.Avogadro
  SET( MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge" )
  SET( MACOSX_BUNDLE_BUNDLE_NAME "Avogadro" )

  # Need to copy the icon files and other resources
  SET( MACOSX_BUILD_CONTENTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/Avogadro.app/Contents/Resources )
	SET( MACOSX_SRC_DIR ${avogadro_SOURCE_DIR}/src/mac )
  EXEC_PROGRAM( "mkdir -p   ${MACOSX_BUILD_CONTENTS_DIR}" )
  EXEC_PROGRAM( "cp ${MACOSX_SRC_DIR}/avogadro.icns ${MACOSX_BUILD_CONTENTS_DIR}" )
  EXEC_PROGRAM("cp ${MACOSX_SRC_DIR}/AvogadroDocument.icns ${MACOSX_BUILD_CONTENTS_DIR}" )
  SET( TRANSLATIONS ar ca de en en_GB es fr it pl pt pt_BR ru zh_CN zh_TW )
	FOREACH(lang ${TRANSLATIONS})
	  EXEC_PROGRAM("mkdir ${MACOSX_BUILD_CONTENTS_DIR}/${lang}.lproj" )
	ENDFOREACH(lang ${TRANSLATIONS})

  # Overload the Info.plist default
  SET( CMAKE_MODULE_PATH ${avogadro_SOURCE_DIR}/src/mac ${CMAKE_MODULE_PATH} )
  INSTALL(TARGETS avogadro-app DESTINATION /Applications)
  INSTALL(
          CODE "EXEC_PROGRAM(\"install_name_tool -change ${libavogadro_BINARY_DIR}/src/libavogadro.0.dylib ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/libavogadro.0.dylib /Applications/avogadro.app/Contents/MacOS/avogadro\")")
  INSTALL(CODE "EXEC_PROGRAM(\"mv /Applications/avogadro.app /Applications/Avogadro.app\")")

ELSE( APPLE)
  INSTALL(TARGETS avogadro-app DESTINATION bin)
ENDIF( APPLE)

INSTALL(FILES ${avogadro_QM} DESTINATION share/avogadro/i18n)

#add_subdirectory(projectplugins)
