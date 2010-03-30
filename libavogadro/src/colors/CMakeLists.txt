include(${QT_USE_FILE})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)

set(DESTINATION_DIR ${Avogadro_PLUGIN_INSTALL_DIR}/colors)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

link_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL colors)
set(PLUGIN_TARGET colors)
add_custom_target(colors COMMENT "Meta target to build all Avogadro colors.")

### elementcolor
# This is built statically by the main libavogadro -- we do not build it again

### residuecolor
avogadro_plugin(residuecolor residuecolor.cpp residuecolorsettings.ui)

### indexcolor
avogadro_plugin(atomindexcolor atomindexcolor.cpp)

### chargecolor
avogadro_plugin(chargecolor chargecolor.cpp)

### distancecolor
avogadro_plugin(distancecolor distancecolor.cpp)

## custom color
avogadro_plugin(customcolor customcolor.cpp)

## smartscolor
avogadro_plugin(smartscolor smartscolor.cpp)
