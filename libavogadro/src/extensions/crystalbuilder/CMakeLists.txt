include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(spglib)

set(PLUGIN_LABEL extensions)
set(PLUGIN_TARGET extensions)

### Crystal builder
set(crystalbuilderextension_SRCS
  cbpastedialog.cpp
  cbundo.cpp
  crystalbuilderextension.cpp
  crystalbuilderdialog.cpp
  avospglib.cpp
)

set(crystalbuilderextension_UIS
  cbpastedialog.ui
  crystalbuilderdialog.ui
)

avogadro_plugin(crystalbuilderextension
  "${crystalbuilderextension_SRCS}"
  "${crystalbuilderextension_UIS}"
)

target_link_libraries(crystalbuilderextension spglib)
add_dependencies(crystalbuilderextension spglib)