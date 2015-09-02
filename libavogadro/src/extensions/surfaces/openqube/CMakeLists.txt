find_package(Qt4 4.6 REQUIRED)
find_package(Eigen3)
if(NOT EIGEN3_FOUND)
  message(STATUS "Cannot find Eigen3, trying Eigen2")
  find_package(Eigen2 REQUIRED)
else()
  set (EIGEN2_SUPPORT_STAGE10_FULL_EIGEN2_API TRUE)
endif()

include_directories(${QT_INCLUDE_DIR} ${EIGEN2_INCLUDE_DIR})

# Headers for our public API
set(openqube_HDRS
  atom.h
  basisset.h
  basissetloader.h
  cube.h
  gamessukout.h
  gamessus.h
  orca.h
  gaussianset.h
  molecule.h
  openqubeabi.h
  slaterset.h
)

# Source files for our data.
set(openqube_SRCS
  atom.cpp
  basisset.cpp
  basissetloader.cpp
  cube.cpp
  gamessukout.cpp
  gamessus.cpp
  orca.cpp
  gaussianfchk.cpp
  gaussianset.cpp
  molden.cpp
  molecule.cpp
  mopacaux.cpp
  slaterset.cpp
)

qt4_wrap_cpp(openqubeMocSrcs basisset.h gaussianset.h slaterset.h)

add_library(OpenQube SHARED ${openqube_SRCS} ${openqubeMocSrcs})

set_target_properties(OpenQube PROPERTIES
  VERSION 0.1.0
  OUTPUT_NAME avogadro_OpenQube
  LINK_INTERFACE_LIBRARIES ""
  SOVERSION 0
  LABELS openqube)

target_link_libraries(OpenQube ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

add_executable(qube qube.cpp)
target_link_libraries(qube OpenQube ${QT_LIBRARIES})
install(TARGETS qube DESTINATION ${BIN_INSTALL_DIR})

install(TARGETS OpenQube DESTINATION ${LIB_INSTALL_DIR})
