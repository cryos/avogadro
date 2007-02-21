project(avogadro) # the name of your project

find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
find_package(OpenGL REQUIRED) # find and setup OpenGL
find_package(OpenBabel2 REQUIRED) # find and setup OpenBabel
find_package(Eigen REQUIRED) # find and setup Eigen
#find_package(Avogadro REQUIRED) # find and setup Avogadro

if (WIN32)
	add_definitions( -DWIN32 )
endif (WIN32)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)


# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
