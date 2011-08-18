include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(spglib)

set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

set(crystallographyextension_SRCS
  avospglib.cpp
  ceundo.cpp
  crystallographyextension.cpp
  crystalpastedialog.cpp
  ui/ceabstractdockwidget.cpp
  ui/ceabstracteditor.cpp
  ui/cecoordinateeditor.cpp
  ui/cematrixeditor.cpp
  ui/ceparametereditor.cpp
  ui/cetranslatewidget.cpp
  ui/ceviewoptions.cpp
)

set(crystallographyextension_UIS
  crystalpastedialog.ui
  ui/cecoordinateeditor.ui
  ui/cematrixeditor.ui
  ui/ceparametereditor.ui
  ui/cetranslatewidget.ui
  ui/ceviewoptions.ui
)

avogadro_plugin(crystallographyextension
  "${crystallographyextension_SRCS}"
  "${crystallographyextension_UIS}"
)

target_link_libraries(crystallographyextension spglib)
add_dependencies(crystallographyextension spglib)
