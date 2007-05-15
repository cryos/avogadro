project(avogadro) # the name of your project

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

cmake_minimum_required(VERSION 2.4.3)

#set(QT_MIN_VERSION "4.3.0") # we need at least version 4.3 of Qt
find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
find_package(OpenGL REQUIRED) # find and setup OpenGL
find_package(OpenBabel2 REQUIRED) # find and setup OpenBabel
find_package(Eigen REQUIRED) # find and setup Eigen

if(NOT LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR lib)
endif(NOT LIB_INSTALL_DIR)

CONFIGURE_FILE(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(libavogadro)
add_subdirectory(avogadro)
