project(libavogadro)
set(VERSION 0.2.1)

# Generate SVN_REVISION variable
execute_process(COMMAND svnversion -n ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE SVN_REVISION)

if (WIN32)
  add_definitions( -DWIN32 )

  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif (WIN32)

if (ENABLE_THREADED_GL)
  add_definitions( -DENABLE_THREADED_GL )
endif(ENABLE_THREADED_GL)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
