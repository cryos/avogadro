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

# Ensure the Avogadro include directory is always first
get_directory_property(tmp_include_dirs INCLUDE_DIRECTORIES)
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES
  "${libavogadro_BINARY_DIR}/include;${tmp_include_dirs}")
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OPENBABEL2_INCLUDE_DIR}
  ${EIGEN2_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

if(GLEW_FOUND)
  include_directories(${GLEW_INCLUDE_DIR})
endif(GLEW_FOUND)

include(${QT_USE_FILE})

FILE(GLOB avogadro_SRCS "*.cpp")

FILE(GLOB projectDelegates_SRCS "projectdelegates/*.cpp")
set(avogadro_SRCS ${avogadro_SRCS} ${projectDelegates_SRCS})
# Add GL2PS to the executable
set(avogadro_SRCS ${avogadro_SRCS} "../gl2ps/gl2ps.c")

if( WIN32 )
  set(avogadro_SRCS ${avogadro_SRCS} windows/avogadro.rc)
endif( WIN32)

file(GLOB avogadro_RCS "*.qrc")

file(GLOB avogadro_UIS "*.ui")

# designer has some flaws.
qt4_automoc(${avogadro_SRCS})
qt4_add_resources(avogadro_RC_SRCS ${avogadro_RCS})
qt4_wrap_ui(avogadro_UIS_H ${avogadro_UIS})

file(GLOB avogadro_TS "${CMAKE_SOURCE_DIR}/i18n/*.ts")

QT4_WRAP_TS(avogadro_QM ${avogadro_TS})

# Some relative rpath handling for Linux
if(CMAKE_SYSTEM_NAME MATCHES Linux)
  # Set a relative rpath on Linux - just testing right now...
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib${LIB_SUFFIX}")
endif(CMAKE_SYSTEM_NAME MATCHES Linux)

add_executable(avogadro-app WIN32 MACOSX_BUNDLE
  ${avogadro_SRCS}
  ${avogadro_UIS_H}
  ${avogadro_RC_SRCS}
  ${avogadro_QM}
)

set_target_properties(avogadro-app PROPERTIES OUTPUT_NAME avogadro)

target_link_libraries(avogadro-app ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
    avogadro-lib)

if(APPLE)
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
  EXECUTE_PROCESS(COMMAND mkdir -p ${MACOSX_BUILD_CONTENTS_DIR} )
  EXECUTE_PROCESS(COMMAND cp ${MACOSX_SRC_DIR}/avogadro.icns ${MACOSX_BUILD_CONTENTS_DIR} )
  EXECUTE_PROCESS(COMMAND cp ${MACOSX_SRC_DIR}/AvogadroDocument.icns ${MACOSX_BUILD_CONTENTS_DIR} )
  SET( TRANSLATIONS "ar;ca;cs;de;en;en_GB;es;fi;fr;it;pl;pt;pt_BR;ru;sv;uk;zh;zh_CN;zh_TW" )
	FOREACH(lang ${TRANSLATIONS})
    SET( MAC_LANG_DIR ${MACOSX_BUILD_CONTENTS_DIR}/${lang}.lproj )
	  EXECUTE_PROCESS(COMMAND
          mkdir ${MAC_LANG_DIR}
          ERROR_QUIET)
    EXECUTE_PROCESS(COMMAND
          ${MACOSX_SRC_DIR}/lang.sh ${MACOSX_SRC_DIR} ${MAC_LANG_DIR} ${VERSION} ${lang}
          OUTPUT_QUIET)
	ENDFOREACH(lang ${TRANSLATIONS})

  # Overload the Info.plist default
  SET( CMAKE_MODULE_PATH ${avogadro_SOURCE_DIR}/src/mac ${CMAKE_MODULE_PATH} )
  INSTALL(TARGETS avogadro-app DESTINATION /Applications)
  INSTALL(
          CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change ${libavogadro_BINARY_DIR}/src/libavogadro.0.dylib ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/libavogadro.0.dylib /Applications/avogadro.app/Contents/MacOS/avogadro OUTPUT_QUIET)")
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND mv /Applications/avogadro.app /Applications/Avogadro.app OUTPUT_QUIET)")
endif(APPLE)

if(NOT APPLE)
  install(TARGETS avogadro-app DESTINATION ${BIN_INSTALL_DIR})
endif(NOT APPLE)

install(FILES ${avogadro_QM} DESTINATION ${I18N_INSTALL_DIR})

# Install a .desktop file and icon for Linux users
if (CMAKE_SYSTEM_NAME MATCHES Linux)
  install(FILES "avogadro.desktop" DESTINATION ${XDG_APPS_INSTALL_DIR})
  install(FILES "icons/avogadro_icon.png" DESTINATION ${XDG_ICON_INSTALL_DIR})
endif(CMAKE_SYSTEM_NAME MATCHES Linux)

#add_subdirectory(projectplugins)
