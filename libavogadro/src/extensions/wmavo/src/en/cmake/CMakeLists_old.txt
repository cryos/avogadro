
project(WmEngine)
cmake_minimum_required(VERSION 2.6)

# Define the "identity" of the project.
set( plugin_type Engine )
set( plugin_name wmengine )

# Define some Qt element for the moc generator.
set( in_MOC_H src/wmengine.h )
set( in_UIS_H "" )
set( in_RC_SCRS "" )

if( WIN32 ) # Windows
# Need Windows path for a lot of elements.
  set( Avogadro_DIR "C:/src/avogadro-1.1.0/build" ) # .h
  set( EIGEN2_INCLUDE_DIR "C:/src/eigen-2.0.15" ) # .h
  
  set( OpenBabel2_include_DIR "C:/src/openbabel-2.3.0/include" ) # .h
  set( OpenBabel22_include_DIR "C:/src/openbabel-2.3.0/windows-vc2008/build/include" ) # .h
  
  set( glut_INCLUDE_DIR "C:/src/freeglut-2.6.0/include" ) # .h
  set( glut_LIB_DIR "C:/src/freeglut-2.6.0/Release" ) # .lib
  set( glut_lib_name freeglut ) # Name of the library to link.

  # Add a flag in CMake attribut.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  
else( WIN32 ) # Unix
# Need a little element for Unix.
  set( OpenBabel2_include_DIR "/usr/include/openbabel-2.0/" )
  
  # Add a flag in CMake attribut.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -lglut")
  
endif( WIN32 )

# Search files and library.
find_package(Avogadro 1.0.0 REQUIRED)

# Define all source files.
set( wmavoconst_include_dir ../ )
file( GLOB_RECURSE source_files	src/* )

## Set up the build environment
include(${Avogadro_USE_FILE})

if( WIN32 ) # Windows
  include_directories(${OpenBabel2_include_DIR})
  include_directories(${OpenBabel22_include_DIR})

  include_directories( ${glut_INCLUDE_DIR} )
  link_directories( ${glut_LIB_DIR} )
  
  # Add macro for thr preprocessor.
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS)
  
else( WIN32 ) # Unix
  include_directories(${OpenBabel2_include_DIR})
  
endif( WIN32 )

## Génération des fichiers nécessaire pour la compilation 
## et éditions de liens avec Qt, Avogadro & Co.
QT4_WRAP_CPP(out_MOC_H ${in_MOC_H})
qt4_wrap_ui(out_UIS_H ${in_UIS_H})
qt4_add_resources(out_RC_SRCS ${in_RC_SCRS})

add_library( ${plugin_name} MODULE ${source_files} ${out_UIS_H} ${out_MOC_H} ${out_RC_SRCS} )
target_link_libraries( ${plugin_name} avogadro )

if( WIN32 ) # Windows
  target_link_libraries( ${plugin_name} ${glut_lib_name} )
endif( WIN32 )

# ?
#install(TARGETS ${plugin_name} DESTINATION "${Avogadro_PLUGIN_DIR}/${plugin_type}")
#set_target_properties(${plugin_name} PROPERTIES OUTPUT_NAME ${plugin_name} PREFIX "")

