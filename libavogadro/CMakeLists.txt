project(avogadrolib) # the name of your project

find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
find_package(OpenBabel2 REQUIRED) # find and setup OpenBabel
find_package(Eigen REQUIRED) # find and setup OpenBabel
find_package(OpenGL REQUIRED) # find and setup OpenBabel

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
