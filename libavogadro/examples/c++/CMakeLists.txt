######################################################################
# Example CMake project building a 3rd party extension against 
# installed Avogadro library.  
# See http://avogadro.openmolecules.net/ for more information.
######################################################################

# Project name - plugin name or something meaningful
project(TemplateExtension)

# Minimum version of CMake that can be used
cmake_minimum_required(VERSION 2.6)

# Find the Avogadro library if it is installed
find_package(Avogadro REQUIRED)
 
set(QT_MIN_VERSION "4.5.0")
find_package(Qt4 REQUIRED)

# Activate this line if you need direct use of OpenBabel
#find_package(OpenBabel2 REQUIRED)

# Set up the build environment
include(${Avogadro_USE_FILE})
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${OPENBABEL2_INCLUDE_DIR}
)
link_directories(${OPENBABEL2_LIBRARY_DIRS})

# Build your plugin using the default options
set (templateextension_SRCS templateextension.cpp)

avogadro_plugin(templateextension "${templateextension_SRCS}")
