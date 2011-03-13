include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(spglib)

set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

set(crystallographyextension_SRCS
  avospglib.cpp
  cbpastedialog.cpp
  cbundo.cpp
  crystallographyextension.cpp
  ui/cbabstracteditor.cpp
  ui/cbcoordinateeditor.cpp
  ui/cbmatrixeditor.cpp
  ui/cbparametereditor.cpp
)

set(crystallographyextension_UIS
  cbpastedialog.ui
  ui/cbcoordinateeditor.ui
  ui/cbmatrixeditor.ui
  ui/cbparametereditor.ui
)

avogadro_plugin(crystallographyextension
  "${crystallographyextension_SRCS}"
  "${crystallographyextension_UIS}"
)

target_link_libraries(crystallographyextension spglib)
add_dependencies(crystallographyextension spglib)