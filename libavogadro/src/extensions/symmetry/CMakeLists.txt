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

# Fixes a compilation error involving position-independent code
if(CMAKE_COMPILER_IS_GNUCXX)
  set_property(TARGET msym PROPERTY COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif(CMAKE_COMPILER_IS_GNUCXX)

target_link_libraries(symmetryextension msym)
