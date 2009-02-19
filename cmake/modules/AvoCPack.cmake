# Set up CPack with the correct version, external libraries etc

set (CPACK_PACKAGE_NAME "Avogadro")
set (CPACK_PACKAGE_VERSION_MAJOR 0)
set (CPACK_PACKAGE_VERSION_MINOR 9)
set (CPACK_PACKAGE_VERSION_PATCH 1)
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
  if(NOT python_DIR)
    set (python_DIR "C:/src/python-2.6.1") # already contains sip & numpy modules (see wiki)
  endif(NOT python_DIR)
  if(NOT boost_python_DIR)
    set (boost_python_DIR "C:/src/boost_1_38_0")
  endif(NOT boost_python_DIR)
  if(NOT pyqt_DIR)
    set (pyqt_DIR "C:/src/PyQt-win-gpl-4.4.4")
  endif(NOT pyqt_DIR)

  # openbabel
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

  # Qt
  set(qt_DEPS
    "${qt_BINDIR}/QtCore4.dll"
    "${qt_BINDIR}/QtGui4.dll"
    "${qt_BINDIR}/QtOpenGL4.dll")
  install(FILES ${qt_DEPS} DESTINATION bin)

  ##########
  # Python #
  ##########

  # python library
  set(python_DEPS
    "${python_DIR}/libs/python26.dll")
  install(FILES ${python_DEPS} DESTINATION bin)
  
  # lib/*: (includes all sip & numpy runtime files needed)
  file(GLOB python_lib_FILES "${python_DIR}/lib/*.py") 
  install(FILES ${python_lib_FILES} DESTINATION bin/lib)
  #file(GLOB_RECURSE python_lib_FILES "${python_DIR}/lib/site-packages/*.*") 
  #install(FILES ${python_lib_FILES} DESTINATION bin/lib)

  add_custom_command(TARGET python_lib_FILES POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${python_DIR}/lib" bin)

  # boost python
  set(boost_python_DEPS
    "${boost_python_DIR}/lib/boost_python-vc90-mt-1_38.dll")
  install(FILES ${boost_python_DEPS} DESTINATION bin)
  

  # PyQt4
  set(pyqt_DEPS
    "${pyqt_DIR}/__init__.py"
    "${pyqt_DIR}/Qt/Qt.pyd"
    "${pyqt_DIR}/QtCore/QtCore.pyd"
    "${pyqt_DIR}/QtGui/QtGui.pyd"
    "${pyqt_DIR}/QtOpenGL/QtOpenGL.pyd"
    "${pyqt_DIR}/QtCore/QtCore.pyd")
  install(FILES ${pyqt_DEPS} DESTINATION bin/lib/site-packages/PyQt4)
 
endif (WIN32)

include(CPack)
include(InstallRequiredSystemLibraries)
