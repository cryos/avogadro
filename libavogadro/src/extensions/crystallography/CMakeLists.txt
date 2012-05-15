include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LINK_LIBS AvoSpg avogadro)

set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

set(crystallographyextension_SRCS
  ceundo.cpp
  crystallographyextension.cpp
  crystalpastedialog.cpp
  ui/ceabstractdockwidget.cpp
  ui/ceabstracteditor.cpp
  ui/cecoordinateeditor.cpp
  ui/cematrixeditor.cpp
  ui/ceparametereditor.cpp
  ui/ceslabbuilder.cpp
  ui/cetranslatewidget.cpp
  ui/ceviewoptionswidget.cpp
)

set(crystallographyextension_UIS
  crystalpastedialog.ui
  ui/cecoordinateeditor.ui
  ui/cematrixeditor.ui
  ui/ceparametereditor.ui
  ui/ceslabbuilder.ui
  ui/cetranslatewidget.ui
  ui/ceviewoptionswidget.ui
)

avogadro_plugin(crystallographyextension
  "${crystallographyextension_SRCS}"
  "${crystallographyextension_UIS}"
)

