include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

# Build your plugin using the default options
avogadro_plugin_nogl(packmolextension "packmolextension.cpp;packmoldialog.cpp;highlighter.cpp;structuresmodel.cpp" packmoldialog.ui)

