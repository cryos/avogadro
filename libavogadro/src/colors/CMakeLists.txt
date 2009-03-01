set(QT_USE_QTOPENGL true)

include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${PLUGIN_INSTALL_DIR}/colors)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

### elementcolor
# This is built statically by the main libavogadro -- we do not build it again

### residuecolor
set(residuecolor_SRCS residuecolor.cpp)
qt4_wrap_ui(residuecolor_UIS residuecolorsettings.ui)
qt4_automoc(${residuecolor_SRCS})
add_library(residuecolor MODULE ${residuecolor_SRCS} ${residuecolor_UIS})
target_link_libraries(residuecolor ${LINK_LIBS})
install(TARGETS residuecolor DESTINATION ${DESTINATION_DIR})

### indexcolor
set(indexcolor_SRCS indexcolor.cpp)
qt4_automoc(${indexcolor_SRCS})
add_library(indexcolor MODULE ${indexcolor_SRCS})
target_link_libraries(indexcolor ${LINK_LIBS})
install(TARGETS indexcolor DESTINATION ${DESTINATION_DIR})

### chargecolor
set(chargecolor_SRCS chargecolor.cpp)
qt4_automoc(${chargecolor_SRCS})
add_library(chargecolor MODULE ${chargecolor_SRCS})
target_link_libraries(chargecolor ${LINK_LIBS})
install(TARGETS chargecolor DESTINATION ${DESTINATION_DIR})

### distancecolor
set(distancecolor_SRCS distancecolor.cpp)
qt4_automoc(${distancecolor_SRCS})
add_library(distancecolor MODULE ${distancecolor_SRCS})
target_link_libraries(distancecolor ${LINK_LIBS})
install(TARGETS distancecolor DESTINATION ${DESTINATION_DIR})
