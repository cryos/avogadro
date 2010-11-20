# Dynamic Display Avogadro extension.
# See http://avogadro.openmolecules.net/ for more information.

# Project name
project(DynamicDisplayExtension)

# Minimum version of CMake that can be used
cmake_minimum_required(VERSION 2.6)

set(QT_MIN_VERSION "4.5.0")
find_package(Qt4 REQUIRED)
find_package(Avogadro REQUIRED)
include(${Avogadro_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set( dynamicdisplayextension_SRCS
     dynamicdisplayextension.cpp
     dynamicdisplaydialog.cpp
)

set( dynamicdisplayextension_UIS
     dynamicdisplaydialog.ui
)

avogadro_plugin( dynamicdisplayextension
                 "${dynamicdisplayextension_SRCS}"
		 "${dynamicdisplayextension_UIS}")
