
# Project name - plugin name or something meaningful
project(MyExtension)

# Minimum version of CMake that can be used
cmake_minimum_required(VERSION 2.6)

# Différentes variables propres au plugin.
set( plugin_type extension )
set( plugin_name myextension )
set( in_CPP myextension.cpp )
set( in_MOC_H myextension.h )
set( in_UIS_H "" )
set( in_RC_SCRS "" )
#set(Avogadro_DIR "C:/avogadro_compile/build") # Windows

# Find the Avogadro library if it is installed
find_package(Avogadro 0.9.4 REQUIRED)

set( OpenBabel_DIR "/usr/include/openbabel-2.0/" ) # Pas obligatoire pour tous les plugins.

# Set up the build environment
include(${Avogadro_USE_FILE})
include_directories(${OpenBabel_DIR})

### Génération des fichiers nécessaire pour la compilation 
### et éditions de liens avec Qt, Avogadro & Co.

QT4_WRAP_CPP(out_MOC_H ${in_MOC_H})
qt4_wrap_ui(out_UIS_H ${in_UIS_H})
qt4_add_resources(out_RC_SRCS ${in_RC_SCRS})

add_library(${plugin_name} MODULE ${in_CPP} ${out_UIS_H} ${out_MOC_H} ${out_RC_SRCS})
target_link_libraries(${plugin_name} avogadro)
install(TARGETS ${plugin_name} DESTINATION "${Avogadro_PLUGIN_DIR}/${plugin_type}")
set_target_properties(${plugin_name} PROPERTIES OUTPUT_NAME ${plugin_name} PREFIX "")

