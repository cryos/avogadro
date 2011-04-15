# Set up CPack with the correct version, external libraries etc

set (CPACK_PACKAGE_NAME "Avogadro")
set (CPACK_PACKAGE_VERSION_MAJOR ${Avogadro_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${Avogadro_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${Avogadro_VERSION_PATCH})
set (CPACK_PACKAGE_VERSION ${Avogadro_VERSION_MAJOR}.${Avogadro_VERSION_MINOR}.${Avogadro_VERSION_PATCH})
set (CPACK_PACKAGE_INSTALL_DIRECTORY "Avogadro")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

set (CPACK_PACKAGE_EXECUTABLES "avogadro" "Avogadro")
set (CPACK_CREATE_DESKTOP_LINKS "avogadro")

if(WIN32)
  option(ENABLE_DEPRECATED_INSTALL_RULES "Should deprecated, Windows specific, install rules be enabled?" OFF)
endif()
if (WIN32 AND ENABLE_DEPRECATED_INSTALL_RULES)
  # Set the directories to defaults if not set

  ##############################################
  # Zlib                                       #
  ##############################################
  find_file(zlib_DLL "zlib1.dll" PATHS
      "C:/src/zlib-1.2.3/projects/visualc6/Win32_DLL_Release"
      ${zlib_DIR}
  )
  install(FILES ${zlib_DLL} DESTINATION bin)

  ##############################################
  # libxml2                                    #
  ##############################################
  find_path(libxml2_DIR "libxml-2.0.pc.in" PATHS
      "C:/src/libxml2"
      "C:/src/libxml2-2.7.3"
      "C:/src/libxml2-2-7-3"
  )
  find_file(libxml2_DLL "libxml2.dll" PATHS
      "${libxml2_DIR}/win32/bin.msvc"
      "${libxml2_DIR}/bin"
      "${libxml2_DIR}/lib"
  )
  install(FILES ${libxml2_DLL} DESTINATION bin)

  ##############################################
  # OpenBabel                                  #
  ##############################################
  find_path(openbabel_SRCDIR "openbabel-2.0.pc.in" PATHS
      "C:/src/openbabel"
      "C:/src/openbabel-2-2-1"
      "C:/src/openbabel-2-2-x"
      "C:/src/openbabel-2.2.1"
      "C:/src/openbabel-2.2.x"
  )

  # Data files needed by OpenBabel
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.txt")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.par")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.prm")
  install(FILES ${openbabel_FILES} DESTINATION bin)
  file(GLOB openbabel_FILES "${openbabel_SRCDIR}/data/*.ff")
  install(FILES ${openbabel_FILES} DESTINATION bin)

  find_path(openbabel_BINDIR "openbabel-2.dll" PATHS
      "${openbabel_SRCDIR}/output/Release"
      "${openbabel_SRCDIR}/build/src/Release"
      "${openbabel_SRCDIR}/src/Release"
      "${openbabel_SRCDIR}/Release"
      "${openbabel_SRCDIR}"
  )
  set(openbabel_DLLs
      "${openbabel_BINDIR}/openbabel-2.dll"
      "${openbabel_BINDIR}/inchi.dll")
  install(FILES ${openbabel_DLLs} DESTINATION bin)

  file(GLOB openbabel_FORMATS "${openbabel_BINDIR}/*.obf")
  install(FILES ${openbabel_FORMATS} DESTINATION bin)

  ##############################################
  # Qt                                         #
  ##############################################
  find_path(qt_BINDIR "QtCore4.dll" PATHS
      "C:/src/qt-4.4.3/bin"
      "C:/src/qt-4.4.4/bin"
      "C:/src/qt-4.4.5/bin"
      "C:/src/qt-4.5.0/bin"
      "C:/src/qt-4.5.1/bin"
  )
  set(qt_DEPS
    "${qt_BINDIR}/QtCore4.dll"
    "${qt_BINDIR}/QtGui4.dll"
    "${qt_BINDIR}/QtOpenGL4.dll"
    "${qt_BINDIR}/QtNetwork4.dll")
  install(FILES ${qt_DEPS} DESTINATION bin)

  ##############################################
  # GLSL shaders (Optional)                    #
  ##############################################
  if(ENABLE_GLSL AND GLEW_FOUND)
    find_file(glew_DLL "glew32.dll" PATHS
        "C:/src/glew/bin"
    )
    install(FILES ${glew_DLL} DESTINATION bin)
  endif()

  ##############################################
  # Python (Optional)                          #
  ##############################################
  if(ENABLE_PYTHON AND ALL_PYTHON_FOUND)
    # Python support - optionally enabled and installed

    #
    # python library
    #
    find_path(python_DIR "pyconfig.h.in" PATHS
        "C:/src/Python-2.6.2"
        "C:/src/Python-2.6.1"
    )
    find_file(python_DLL "python26.dll" PATHS
        "${python_DIR}/Libs"
        "${python_DIR}/DLLs"
        "${python_DIR}/bin"
        "${python_DIR}/PCbuild"
    )
    install(FILES ${python_DLL} DESTINATION bin)

    #
    # boost python
    #
    find_path(boost_DIR "boost.png" PATHS
        "C:/src/boost_1_38_0"
        "C:/src/boost_1_37_0"
        "C:/src/boost_1_36_0"
    )
    find_file(boost_python_DLL "boost_python-vc90-mt-1_38.dll" PATHS
        "${boost_DIR}/lib/"
        "${boost_DIR}/bin/"
    )
    install(FILES ${boost_python_DLL} DESTINATION bin)

    # lib/*: (includes all sip & numpy runtime files needed)
    file(GLOB python_lib_FILES "${python_DIR}/lib/*.py")
    install(FILES ${python_lib_FILES} DESTINATION bin/lib)
    # lib/encodings/*.py
    file(GLOB python_lib_encodings_FILES "${python_DIR}/lib/encodings/*.py")
    install(FILES ${python_lib_encodings_FILES} DESTINATION bin/lib/encodings)
    #
    # sip
    #
    set(python_lib_sip_FILES
        "${python_DIR}/lib/site-packages/sip.pyd"
        "${python_DIR}/lib/site-packages/sipconfig.py"
    )
    install(FILES ${python_lib_sip_FILES} DESTINATION bin/lib/site-packages)
    #
    # numpy
    #
    install(DIRECTORY ${python_DIR}/lib/site-packages/numpy DESTINATION bin/lib/site-packages)
    #
    # PyQt4
    #
    find_path(pyqt_DIR "pyqtconfig.py.in" PATHS
        "C:/src/PyQt-win-gpl-4.4.4"
    )
    set(pyqt_DEPS
      "${pyqt_DIR}/__init__.py"
      "${pyqt_DIR}/Qt/Qt.pyd"
      "${pyqt_DIR}/QtCore/QtCore.pyd"
      "${pyqt_DIR}/QtGui/QtGui.pyd"
      "${pyqt_DIR}/QtOpenGL/QtOpenGL.pyd"
      "${pyqt_DIR}/QtCore/QtCore.pyd")
    install(FILES ${pyqt_DEPS} DESTINATION bin/lib/site-packages/PyQt4)
    #
    # Avogadro python module
    #
    install(FILES ${Avogadro_BINARY_DIR}/lib/_Avogadro.pyd DESTINATION bin/lib/site-packages)
    install(FILES ${Avogadro_SOURCE_DIR}/libavogadro/src/python/Avogadro.py DESTINATION bin/lib/site-packages)
    #
    # Avogadro plugin scripts
    #
    file(GLOB toolScripts "${Avogadro_SOURCE_DIR}/libavogadro/src/tools/python/*.py")
    install(FILES ${toolScripts} DESTINATION bin/toolScripts)
    file(GLOB engineScripts "${Avogadro_SOURCE_DIR}/libavogadro/src/engines/python/*.py")
    install(FILES ${engineScripts} DESTINATION bin/engineScripts)
    file(GLOB extensionScripts "${Avogadro_SOURCE_DIR}/libavogadro/src/extensions/python/*.py")
    install(FILES ${extensionScripts} DESTINATION bin/extensionScripts)

  endif()

endif()

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "ppc;i386")
endif(APPLE)

configure_file("${CMAKE_MODULE_PATH}/AvoCPackOptions.cmake.in"
  "${CMAKE_BINARY_DIR}/AvoCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_BINARY_DIR}/AvoCPackOptions.cmake")

include(CPack)
include(InstallRequiredSystemLibraries)
