project(AVOGADRO) # the name of your project

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

cmake_minimum_required(VERSION 2.4.0)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(libavogadro)
add_subdirectory(avogadro)
