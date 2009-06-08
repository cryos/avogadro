set(QT_USE_QTOPENGL true)
set(QT_USE_QTNETWORK true)

if(WIN32)
  set(QT_USE_QTMAIN true)
endif(WIN32)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

link_directories(${OPENBABEL2_LIBRARY_DIRS})

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

# Should we build the update checking code?
option(ENABLE_UPDATE_CHECKER
  "Should the Avogadro update checking code be compiled?" ON)
if(ENABLE_UPDATE_CHECKER)
  add_definitions(-DENABLE_UPDATE_CHECKER)
else(ENABLE_UPDATE_CHECKER)
  list(REMOVE_ITEM avogadro_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/updatecheck.cpp")
  list(REMOVE_ITEM avogadro_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/updatedialog.cpp")
endif(ENABLE_UPDATE_CHECKER)

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

file(GLOB avogadro_TS "${Avogadro_BINARY_DIR}/i18n/avogadro*.ts")
QT4_WRAP_TS(avogadro_QM ${avogadro_TS})

# Some relative rpath handling for Linux
if(CMAKE_SYSTEM_NAME MATCHES Linux AND ENABLE_RPATH)
  # Set a relative rpath on Linux - just testing right now...
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib${LIB_SUFFIX}")
endif(CMAKE_SYSTEM_NAME MATCHES Linux AND ENABLE_RPATH)

add_executable(avogadro-app WIN32 MACOSX_BUNDLE
  ${avogadro_SRCS}
  ${avogadro_UIS_H}
  ${avogadro_RC_SRCS}
  ${avogadro_QM}
)

set_target_properties(avogadro-app PROPERTIES OUTPUT_NAME avogadro)

target_link_libraries(avogadro-app ${OPENBABEL2_LIBRARIES} ${QT_LIBRARIES}
    avogadro)

if(APPLE)
  set(MACOSX_BUNDLE_INFO_STRING "Avogadro - version ${Avogadro_VERSION_FULL}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${Avogadro_VERSION_FULL})
  # Change following line to point to actual icns file in bundle.
  set(MACOSX_BUNDLE_ICON_FILE "avogadro.icns")
  # means bundle is set to be net.sourceforge.Avogadro
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge")
  set(MACOSX_BUNDLE_BUNDLE_NAME "Avogadro")

  set(CMAKE_INSTALL_DESTINATION_ARGS
      BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}")
  # This also sets the bundle name correctly
  set_target_properties(avogadro-app
    PROPERTIES OUTPUT_NAME "${CMAKE_BUNDLE_NAME}")

  # Use relative paths for the executable to link to its libraries
  set_target_properties(avogadro-app PROPERTIES
    CMAKE_INSTALL_NAME_DIR "@executable_path/../lib")

  set(MACOSX_SRC_DIR ${avogadro_SOURCE_DIR}/src/mac)

  # Overload the Info.plist default
  set(CMAKE_MODULE_PATH ${avogadro_SOURCE_DIR}/src/mac ${CMAKE_MODULE_PATH})

  # Do not install the bundle into the prefix - just everything else
  set(Avogadro_INSTALL_DESTINATIONS_ARGS
    BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}")

  install(FILES "mac/avogadro.icns" DESTINATION Resources)
  install(FILES "mac/AvogadroDocument.icns" DESTINATION Resources)

# Install a .lproj directory for each translation
# We're assuming that there will never be an libavogadro translation
#  without a corresponding avogadro one.
# (Fairly safe assumption)

# This regex gives us translations with just two characters
#  (e.g., avogadro_fr.ts)
  string(REGEX MATCHALL "_[a-z]+\\.ts" langresult "${avogadro_TS}")
  string(REGEX REPLACE "_([^.]+)\\.ts" "\\1" languages "${langresult}")
# This one gives us languages for country codes (e.g., avogadro_zh_CN.ts)
  string(REGEX MATCHALL "_[a-z]+_[A-Z]+\\.ts" langresult "${avogadro_TS}")
  string(REGEX REPLACE "_([^_]+_[^.]+)\\.ts" "\\1" fulllangs "${langresult}")
  set (LANGUAGES "${languages};${fulllangs};en;en_US")

    foreach(lang ${LANGUAGES})
    set( MAC_LANG_DIR "${CMAKE_INSTALL_PREFIX}/Resources/${lang}.lproj" )
    install(CODE "EXECUTE_PROCESS(COMMAND
          mkdir ${MAC_LANG_DIR}
          ERROR_QUIET)")
    install(CODE "EXECUTE_PROCESS(COMMAND
          ${MACOSX_SRC_DIR}/lang.sh ${MACOSX_SRC_DIR} ${MAC_LANG_DIR} ${Avogadro_VERSION_FULL} ${lang}
          OUTPUT_QUIET)")
  endforeach(lang ${TRANSLATIONS})
endif(APPLE)

install(TARGETS avogadro-app RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  ${Avogadro_INSTALL_DESTINATIONS_ARGS})

install(FILES ${avogadro_QM} DESTINATION ${I18N_INSTALL_DIR})

if(APPLE AND ENABLE_AVO_PACKAGE)
  set(OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT
    "${CMAKE_BINARY_DIR}/Avogadro_OSX_MakeStandAloneBundle.cmake")
  set(OSX_MAKE_STANDALONE_BUNDLE_BASH_SCRIPT
    "${CMAKE_BINARY_DIR}/Avogadro_OSX_MakeStandAloneBundle.sh")
  # Configure the cmake file and the shell script
  configure_file("${MACOSX_SRC_DIR}/CompleteBundle.cmake.in"
    "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}" @ONLY IMMEDIATE)
  configure_file("${MACOSX_SRC_DIR}/CreateBundle.sh.in"
    "${OSX_MAKE_STANDALONE_BUNDLE_BASH_SCRIPT}" @ONLY IMMEDIATE)
  install(SCRIPT "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}")
endif(APPLE AND ENABLE_AVO_PACKAGE)

# Install a .desktop file and icon for Linux users
if (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD)
  install(FILES "avogadro.desktop" DESTINATION ${XDG_APPS_INSTALL_DIR})
  install(FILES "icons/avogadro-icon.png" DESTINATION ${XDG_ICON_INSTALL_DIR})
endif(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD)

#add_subdirectory(projectplugins)

