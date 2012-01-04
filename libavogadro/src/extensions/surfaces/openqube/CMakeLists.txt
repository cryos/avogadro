find_package(Qt4 4.6 REQUIRED)
find_package(Eigen2 REQUIRED)

include_directories(${QT_INCLUDE_DIR} ${EIGEN2_INCLUDE_DIR})

# Headers for our public API
set(openqube_HDRS
  atom.h
  basisset.h
  basissetloader.h
  cube.h
  gamessukout.h
  gamessus.h
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

target_link_libraries(OpenQube ${QT_QTCORE_LIBRARY})

install(TARGETS OpenQube DESTINATION ${LIB_INSTALL_DIR})
