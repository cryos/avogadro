find_package(Qt4 4.6.3 REQUIRED)
find_package(Eigen2 REQUIRED)

include_directories(${QT_INCLUDE_DIR} ${EIGEN2_INCLUDE_DIR})

# Source files for our data.
set(openqube_SRCS
  basissetloader.cpp
  cube.cpp
  gamessukout.cpp
  gamessus.cpp
  gaussianfchk.cpp
  gaussianset.cpp
  molden.cpp
  mopacaux.cpp
  slaterset.cpp
)

qt4_wrap_cpp(openqubeMocSrcs basisset.h gaussianset.h slaterset.h)

add_library(openqube SHARED ${openqube_SRCS} ${openqubeMocSrcs})

set_target_properties(openqube PROPERTIES
  VERSION 0.1.0
  LINK_INTERFACE_LIBRARIES ""
  SOVERSION 0
  LABELS openqube)

target_link_libraries(openqube ${QT_QTCORE_LIBRARY})
