
# Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
# Project managers: Eric Henon and Michael Krajecki
# Financial support: Region Champagne-Ardenne (Fr)

set( PLUGIN_NAME wmextension )

# wmextension plugin source files.
set( wmavoconst_include_dir ../ ) # .h (constants & revision & others)
#set( wmtool_include_dir ../tool/src ) # .h (wmtool plugin)

if( NOT WIN32 )
  set( OpenBabel2_include_DIR "/usr/include/openbabel-2.0/" )  
endif()


if( "${LINK_LIBS}" STREQUAL "avogadro" )
# In the Avogadro compilation process.
# A lot of attribut are initiated for openGL, openBabel ... 
# It is necessary that you (if it is does't already) :
# - add the project in : [avogadro_path]/libavogadro/src/extensions/[here_wmavo_directory]
# - in : [avogadro_path]/libavogadro/src/extensions/CMakeLists.txt
#   - add "add_subdirectory( wmavo/src/tool )"
#   - add "add_subdirectory( wmavo/src/ex )"

  # The Meta-Object Compiler, moc, is the program that handles Qt's C++ extensions.
  set( in_MOC_SRC src/wmextension.cpp )
  set( in_UIS_H "" )
  set( in_RC_SCRS "" )
  
  # Write the Avogadro revision in revision.h.
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/../revision.h.in ${CMAKE_CURRENT_SOURCE_DIR}/../revision.h )

  ## Set up the build environment (to know where are .h, ".dll" and some files)

  # Let to use .h directly : 
  # ex. : #include <wmavo_const.h> instead of <../../wmavo_const.h>
  include_directories( ${wmavoconst_include_dir} )

  # Generate moc, specify install directories ...
  avogadro_plugin( ${PLUGIN_NAME} "${in_MOC_SRC}" )
  
else()
# In a stand alone compilation process.
# Nothing is initiated ...

  project( ${PLUGIN_NAME} )
  cmake_minimum_required(VERSION 2.6)
  
  if( WIN32 )
    set( CMAKE_BUILD_TYPE "Debug" ) # Initiate by Me. Release or Debug
  endif()
  
  
  # Avogadro needs.
 
  if( NOT EXISTS "${Avogadro_DIR}" )
    if( WIN32 ) 
      if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
        set( Avogadro_DIR "C:/Program Files (x86)/Avogadro-1.0.3_debug/bin/avogadro" )
      else()
        set( Avogadro_DIR "C:/Program Files (x86)/Avogadro-1.0.3_release/bin/avogadro" )
      endif()
    endif()
  else()
    #message( STATUS "Avogadro_DIR is : " ${Avogadro_DIR} )
  endif()
  
  #message( STATUS "Avogadro directories is searching ..." )
  find_package( Avogadro 1.0.0 REQUIRED )
  message( STATUS "Avogadro directory: ${Avogadro_DIR}" )
  
  # OpenGL needs (Useless, "included" in Avogadro)
  #find_package(OpenGL REQUIRED) # find and setup OpenGL.
  #message( STATUS "OpenGL found : " ${OPENGL_LIBRARIES})

  # glut libraries and source files ("Replaced by" OpenGL needs)
  #set( glut_INCLUDE_DIR "C:/src/freeglut-2.6.0/include" ) # .h
  #set( glut_LIB_DIR "C:/src/freeglut-2.6.0/Release" ) # .lib
  #set( glut_lib_name freeglut ) # Name of the library to link.

  # Eigen needs.
  if( NOT EXISTS "${EIGEN2_INCLUDE_DIR}" )
    if( WIN32 ) 
      set( EIGEN2_INCLUDE_DIR "C:/src/eigen-2.0.15" ) # .h
      message( STATUS "EIGEN2_INCLUDE_DIR is by default : " ${EIGEN2_INCLUDE_DIR} )
    endif()
  else()
    message( STATUS "EIGEN2_INCLUDE_DIR is : " ${EIGEN2_INCLUDE_DIR} )
  endif()
  
  # OpenBabel needs.
  if( NOT EXISTS "${OPENBABEL2_INCLUDE_DIR}" )
    if( WIN32 )
      if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
        set( OPENBABEL2_INCLUDE_DIR "C:/src/openbabeld-2.3.0/windows-vc2008/install/include/openbabel-2.0" ) # .h
      else()
        set( OPENBABEL2_INCLUDE_DIR "C:/src/openbabel-2.3.0/windows-vc2008/install/include/openbabel-2.0" ) # .h
      endif()
    else()
      set( OPENBABEL2_INCLUDE_DIR "/usr/include/openbabel-2.0/" )
      
      if( NOT EXISTS "${OPENBABEL2_INCLUDE_DIR}" )
        set( OPENBABEL2_INCLUDE_DIR "/usr/local/include/openbabel-2.0/" )
      endif()
    endif()
  else()
    #message( STATUS "OPENBABEL2_INCLUDE_DIR is : " ${OPENBABEL2_INCLUDE_DIR} )
  endif()
  
  if( EXISTS "${OPENBABEL2_INCLUDE_DIR}" )
    message( STATUS "OPENBABEL2_INCLUDE_DIR is : " ${OPENBABEL2_INCLUDE_DIR} )
  else()
    message( STATUS "OPENBABEL2_INCLUDE_DIR NOT found : " ${OPENBABEL2_INCLUDE_DIR} )
  endif()
  
  # 
  set( LINK_LIBS avogadro )
  set( PLUGIN_LABEL extensions )
  set( PLUGIN_TARGET extensions )
  set( PLUGIN_TYPE extension )
  
    
  # The Meta-Object Compiler, moc, is the program that handles Qt's C++ extensions.
  set( in_MOC_H src/wmextension.h )
  set( in_UIS_H "" )
  set( in_RC_SCRS "" )
  
  # All source file (recursif search) (wmavo, fifo ...).
  # Add the forgotten files for the moc generation.
  file( GLOB_RECURSE source_files	src/* )
  
  # Write the Avogadro revision in revision.h.
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/../revision.h.in ${CMAKE_CURRENT_SOURCE_DIR}/../revision.h )

  # Add flags for Windows.
  if( WIN32 )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") # Add a flag in a CMake attribut.
    
    if( MSVC ) # For Visual Studio.
      add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
    endif()
  endif()

  
  ## Set up the build environment (to know where are .h, ".dll" and some files)

  # Let to use .h directly : 
  # ex. : #include <wmavo_const.h> instead of <../../wmavo_const.h>
  include_directories( ${wmavoconst_include_dir} )
  include_directories( ${OPENBABEL2_INCLUDE_DIR} )

  # Other files.
  include(${Avogadro_USE_FILE})

  ## BELOW, NO CHANGE ! 
  # (Except to add a target_link_libraries.)

  ## Génération des fichiers nécessaire pour la compilation 
  ## et éditions de liens avec Qt, Avogadro & Co.
  QT4_WRAP_CPP(out_MOC_H ${in_MOC_H})
  qt4_wrap_ui(out_UIS_H ${in_UIS_H})
  qt4_add_resources(out_RC_SRCS ${in_RC_SCRS})

  add_library( ${PLUGIN_NAME} MODULE ${source_files} ${out_UIS_H} ${out_MOC_H} ${out_RC_SRCS} )
  
  # Link the LIBRARY ! Not *.h !
  target_link_libraries( ${PLUGIN_NAME} avogadro )
  target_link_libraries( ${PLUGIN_NAME} ${OPENGL_LIBRARIES} )
  
  # Copy some files in the Avogadro directory.
  file( GLOB_RECURSE frag_files img/fragments/* )
  message( STATUS "fragments_resume directory:" "${BIN_INSTALL_DIR}/../share/avogadro/fragments_resume" )
  message( STATUS "fragments_resume files:" ${frag_files} )
  #install( FILES ${frag_files} DESTINATION "${BIN_INSTALL_DIR}/../share/avogadro/fragments/resume" )

endif()

