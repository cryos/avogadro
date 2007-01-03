project(avogadro) # the name of your project

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} .)

cmake_minimum_required(VERSION 2.4.0)

find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
find_package(OpenBabel REQUIRED) # find and setup OpenBabel
find_package(Avogadro REQUIRED) # find and setup Avogadro

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
