include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${Avogadro_PLUGIN_INSTALL_DIR}/colors)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS "avogadro-lib")

### elementcolor
# This is built statically by the main libavogadro -- we do not build it again

### residuecolor
avogadro_plugin(residuecolor residuecolor.cpp residuecolorsettings.ui)

### indexcolor
avogadro_plugin(indexcolor indexcolor.cpp)

### chargecolor
avogadro_plugin(chargecolor chargecolor.cpp)

### distancecolor
avogadro_plugin(distancecolor distancecolor.cpp)

