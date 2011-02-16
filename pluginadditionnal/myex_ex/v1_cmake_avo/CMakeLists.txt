
# Project name - plugin name or something meaningful
project(MyExtension)

# Minimum version of CMake that can be used
cmake_minimum_required(VERSION 2.6)


# Find the Avogadro library if it is installed
find_package(Avogadro 0.9.4 REQUIRED)

set( OpenBabel_DIR "/usr/include/openbabel-2.0/" ) # Pas obligatoire pour tous les plugins.

# Set up the build environment
include(${Avogadro_USE_FILE})
include_directories(${OpenBabel_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})


# Build your plugin using the default options
avogadro_plugin(myextension myextension.cpp)

