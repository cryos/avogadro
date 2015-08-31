include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Create a list of all source files
set(symmetryextension_SRCS
  symmetryextension.cpp
)

# Create a list of all ui files
set(symmetryextension_UIS
  symmetrydialog.ui
)

# Create the extension target swcntbuilderextension
avogadro_plugin(symmetryextension
 "${symmetryextension_SRCS}"
 "${symmetryextension_UIS}")

add_subdirectory(libmsym)

set_target_properties(msym PROPERTIES POSITION_INDEPENDENT_CODE "TRUE")
target_link_libraries(symmetryextension msym)
