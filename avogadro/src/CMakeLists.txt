set(QT_USE_QTOPENGL true)
set(QT_USE_QTNETWORK true)

if(WIN32)
  set(QT_USE_QTMAIN true)
endif()

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
endif()

include(${QT_USE_FILE})

option(QtTesting "Enable Qt Testing framework?" OFF)
if(QtTesting)
  find_package(QtTesting REQUIRED)
  include_directories(${QtTesting_INCLUDE_DIRS})
  link_directories(${QtTesting_LIBRARY_DIR})
  add_definitions(-DQTTESTING)
endif()

FILE(GLOB avogadro_SRCS "*.cpp")

# Should we build the update checking code? Currently causes segfaults with Qt
# 4.5.3 on both Arch and Gentoo 64 bit platforms (may be more)
option(ENABLE_UPDATE_CHECKER
  "Should the Avogadro update checking code be compiled?" OFF)
if(ENABLE_UPDATE_CHECKER)
  add_definitions(-DENABLE_UPDATE_CHECKER)
else()
  list(REMOVE_ITEM avogadro_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/updatecheck.cpp")
  list(REMOVE_ITEM avogadro_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/updatedialog.cpp")
endif()

FILE(GLOB projectDelegates_SRCS "projectdelegates/*.cpp")
set(avogadro_SRCS ${avogadro_SRCS} ${projectDelegates_SRCS})

if(WIN32)
  set(avogadro_SRCS ${avogadro_SRCS} windows/avogadro.rc)
endif()

file(GLOB avogadro_RCS "*.qrc")

file(GLOB avogadro_UIS "*.ui")

# designer has some flaws.
qt4_automoc(${avogadro_SRCS})
qt4_add_resources(avogadro_RC_SRCS ${avogadro_RCS})
qt4_wrap_ui(avogadro_UIS_H ${avogadro_UIS})

if(NOT WIN32)
  if(Linguist_FOUND)
    file(GLOB_RECURSE lang_PO  "${Avogadro_SOURCE_DIR}/i18n/avogadro/*.po")
    foreach(po ${lang_PO})
      string(REGEX REPLACE "(.*)/([^.]+)\\.po" "\\1/avogadro-\\2.po" po ${po})
      list(APPEND avogadro_PO ${po})
    endforeach()
    QT4_WRAP_PO(avogadro_QM ${avogadro_PO})
  endif()
else()
  # don't attempt to create QM files on windows -- copy them from a linux build!
  file(GLOB_RECURSE avogadro_QM "${Avogadro_SOURCE_DIR}/avogadro/src/avogadro*.qm")
endif()

# Some relative rpath handling for Linux
if(UNIX AND ENABLE_RPATH AND NOT APPLE)
  # Set a relative rpath on Linux - just testing right now...
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib${LIB_SUFFIX}")
endif()

add_executable(avogadro-app WIN32 MACOSX_BUNDLE
  ${avogadro_SRCS}
  ${avogadro_UIS_H}
  ${avogadro_RC_SRCS}
  ${avogadro_QM}
)

set_target_properties(avogadro-app PROPERTIES
  OUTPUT_NAME avogadro
  LABELS avogadro-app)
set_source_files_properties(${avogadro_SRCS}
  PROPERTIES LABELS avogadro)

target_link_libraries(avogadro-app
  ${QT_LIBRARIES}
  ${OPENBABEL2_LIBRARIES}
  avogadro)
if(QtTesting)
  target_link_libraries(avogadro-app QtTesting)
endif()
if(Q_WS_X11)
  target_link_libraries(avogadro-app ${X11_X11_LIB})
endif()

if(ENABLE_PYTHON)
  target_link_libraries(avogadro-app ${Boost_LIBRARIES})
endif()

if(ENABLE_UPDATE_CHECKER)
  target_link_libraries(avogadro-app ${QT_QTNETWORK_LIBRARY})
endif()

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

  # install .nib file
  set(QT_COCOA_NIB /Library/Frameworks/QtGui.framework/Versions/Current/Resources/qt_menu.nib)
  if(EXISTS ${QT_COCOA_NIB})
    install(DIRECTORY ${QT_COCOA_NIB}
      DESTINATION Resources
      USE_SOURCE_PERMISSIONS)
  endif()

# Install a .lproj directory for each translation
# We're assuming that there will never be an libavogadro translation
#  without a corresponding avogadro one.
# (Fairly safe assumption)

# This regex gives us translations with just two characters
#  (e.g., avogadro_fr.ts)
  string(REGEX MATCHALL "-[a-z]+\\.po" langresult "${avogadro_PO}")
  string(REGEX REPLACE "-([^.]+)\\.po" "\\1" languages "${langresult}")
# This one gives us languages for country codes (e.g., avogadro_zh_CN.ts)
  string(REGEX MATCHALL "-[a-z]+_[A-Z]+\\.po" langresult "${avogadro_PO}")
  string(REGEX REPLACE "-([^_]+_[^.]+)\\.po" "\\1" fulllangs "${langresult}")
  set (LANGUAGES "${languages};${fulllangs};en;en_US")

  foreach(lang ${LANGUAGES})
    set( MAC_LANG_DIR "${CMAKE_INSTALL_PREFIX}/Resources/${lang}.lproj" )
    install(CODE "EXECUTE_PROCESS(COMMAND
      mkdir ${MAC_LANG_DIR}
      ERROR_QUIET)")
    install(CODE "EXECUTE_PROCESS(COMMAND
      ${MACOSX_SRC_DIR}/lang.sh ${MACOSX_SRC_DIR} ${MAC_LANG_DIR} ${Avogadro_VERSION_FULL} ${lang}
      OUTPUT_QUIET)")
  endforeach()
endif()

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
endif()

# Install a .desktop file and icon for users of XDG-aware OSes
if(XDG_APPS_INSTALL_DIR AND XDG_ICON_INSTALL_DIR)
  install(FILES "avogadro.desktop" DESTINATION "${XDG_APPS_INSTALL_DIR}")
  install(FILES "icons/avogadro-icon.png" DESTINATION "${XDG_ICON_INSTALL_DIR}")
endif()
