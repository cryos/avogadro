# Hello World Avogadro extension.
# See http://avogadro.openmolecules.net/ for more information.

# Project name
project(ConformerPlotExtension)

# Minimum version of CMake that can be used
cmake_minimum_required(VERSION 2.6)

# Set up Qt
set(QT_MIN_VERSION "4.5.0")
find_package(Qt4 REQUIRED)

# Set up Avogadro
find_package(Avogadro REQUIRED)
include(${Avogadro_USE_FILE})

# Specify include directories
include_directories(
  # Needed to include Qt moc'd headers
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Create a list of all source files
set( conformerplotextension_SRCS
     conformerplotextension.cpp
     conformerplotdialog.cpp
)

# Create a list of all ui files
set( conformerplotextension_UIS
     conformerplotdialog.ui
)

# Create the extension target conformerplotextension
avogadro_plugin(conformerplotextension "${conformerplotextension_SRCS}" "${conformerplotextension_UIS}")

