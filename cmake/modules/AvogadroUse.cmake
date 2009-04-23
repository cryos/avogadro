# The use file for Avogadro, distributed along with Avogadro
include(CMakeImportBuildSettings)
cmake_import_build_settings(${Avogadro_BUILD_SETTINGS_FILE})

# Set up the include directories and link directories
include_directories(${Avogadro_INCLUDE_DIRS})
link_directories(${Avogadro_LIBRARY_DIRS})

# Set up the deps needed to compile an Avogadro plugin
find_package(Qt4 REQUIRED)
set(QT_USE_QTOPENGL true)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)

# Add the Avogadro modules directory to the CMake module path
set(CMAKE_MODULE_PATH ${Avogadro_PLUGIN_DIR}/cmake ${CMAKE_MODULE_PATH})
find_package(Eigen2 REQUIRED)
include_directories(${EIGEN2_INCLUDE_DIR})
if(Avogadro_ENABLE_GLSL)
  find_package(GLEW)
  if(GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIR})
    link_directories(${GLEW_LIBRARY_DIR})
    add_definitions(-DENABLE_GLSL)
  endif(GLEW_FOUND)
endif(Avogadro_ENABLE_GLSL)

# Use this function to add a new plugin. It also uses the global variables
# LINK_LIBS to determine what libraries the plugin should link to and
# DESTINATION_DIR to determine where the plugin will be installed.
# Optional ARGV2 - UI list
# Optional ARGV3 - RC files
function(avogadro_plugin plugin_name src_list)
  qt4_automoc(${src_list})
  if(NOT "${ARGV2}" STREQUAL "")
    # Process the UI file for this plugin
    qt4_wrap_ui(plugin_UIS_H ${ARGV2})
  endif(NOT "${ARGV2}" STREQUAL "")
  if(NOT "${ARGV3}" STREQUAL "")
    # Process the RC file and add it to the plugin
    qt4_add_resources(plugin_RC_SRCS ${ARGV3})
  endif(NOT "${ARGV3}" STREQUAL "")
  add_library(${plugin_name} MODULE ${src_list} ${plugin_UIS_H}
              ${plugin_RC_SRCS})
  target_link_libraries(${plugin_name} avogadro)
  install(TARGETS ${plugin_name} DESTINATION "${Avogadro_PLUGIN_DIR}/contrib")
  set_target_properties(${plugin_name} PROPERTIES
                        OUTPUT_NAME ${plugin_name}
                        PREFIX "")
endfunction(avogadro_plugin)

