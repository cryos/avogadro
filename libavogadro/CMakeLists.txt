project(libavogadro) # the name of your project

find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
find_package(OpenGL REQUIRED) # find and setup OpenGL
find_package(OpenBabel2 REQUIRED) # find and setup OpenBabel
find_package(Eigen REQUIRED) # find and setup Eigen

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
