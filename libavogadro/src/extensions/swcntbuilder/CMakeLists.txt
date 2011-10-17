include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create a list of all source files
set(swcntbuilderextension_SRCS
  ../insertcommand.cpp
  swcntbuilderextension.cpp
  swcntbuilderwidget.cpp
  avotubegen.cpp
)

# Create a list of all ui files
set(swcntbuilderextension_UIS
  swcntbuilderwidget.ui
)

# Create the extension target swcntbuilderextension
avogadro_plugin(swcntbuilderextension
 "${swcntbuilderextension_SRCS}"
 "${swcntbuilderextension_UIS}")

add_subdirectory(tubegen)

target_link_libraries(swcntbuilderextension tubegen)
