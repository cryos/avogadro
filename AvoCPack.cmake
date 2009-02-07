# Set up CPack with the correct version, external libraries etc

set (CPACK_PACKAGE_NAME "Avogadro")
set (CPACK_PACKAGE_VERSION_MAJOR 0)
set (CPACK_PACKAGE_VERSION_MINOR 9)
set (CPACK_PACKAGE_VERSION_PATCH 0)
set (CPACK_PACKAGE_INSTALL_DIRECTORY "Avogadro")
set (CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/avogadro/src/icons/avogadro.ico")
#set (CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/avogadro/src/icons\\avogadro.ico")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

set (CPACK_PACKAGE_EXECUTABLES "avogadro" "Avogadro")
set (CPACK_CREATE_DESKTOP_LINKS "avogadro")

if (WIN32)
  # Set the directories to defaults if not set
  if(NOT openbabel_DIR)
    set (openbabel_DIR "C:/src/openbabel-2-2-x/output/Release")
  endif(NOT openbabel_DIR)
  if(NOT openbabel_SRCDIR)
    set (openbabel_SRCDIR "C:/src/openbabel-2-2-x")
  endif(NOT openbabel_SRCDIR)
  if(NOT qt_BINDIR)
    set (qt_BINDIR "C:/qt/4.4.3/bin")
  endif(NOT qt_BINDIR)

  set(openbabel_DEPS
    "${openbabel_SRCDIR}/windows-vc2005/zlib1.dll"
    "${openbabel_SRCDIR}/windows-vc2005/libxml2.dll"
    "${openbabel_SRCDIR}/windows-vc2005/iconv.dll"
    "${openbabel_SRCDIR}/windows-vc2005/libinchi.dll")
  install(FILES ${openbabel_DEPS} DESTINATION bin)
  
  file(GLOB openbabel_FILES "${openbabel_DIR}/*.obf")
  set(openbabel_FILES ${openbabel_FILES} "${openbabel_DIR}/openbabel-2.dll")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.txt")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.par")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.prm")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.ff")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  
  set(qt_DEPS
    "${qt_BINDIR}/QtCore4.dll"
    "${qt_BINDIR}/QtGui4.dll"
    "${qt_BINDIR}/QtOpenGL4.dll")
  install(FILES ${qt_DEPS} DESTINATION bin)
endif (WIN32)

include(CPack)
include(InstallRequiredSystemLibraries)