
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LINK_LIBS avogadro)
set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

# Create a list of all source files
set( orcaextension_SRCS
     orcaextension.cpp
     orcainputdialog.cpp
     orcaanalysedialog.cpp
     orcaspectra.cpp
     orcadata.cpp
)

# Create a list of all ui files
set( orcaextension_UIS
     orcainputdialog.ui
     orcaanalysedialog.ui
     orcaspectra.ui
)

# Create the extension target orcaextension
avogadro_plugin_nogl(orcaextension "${orcaextension_SRCS}" "${orcaextension_UIS}")

