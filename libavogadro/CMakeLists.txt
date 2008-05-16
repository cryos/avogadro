project(libavogadro)
# Library versioning
# We probably want to adopt the libtool scheme once we hit 1.0.0
set(VERSION 0.8.0)
set(SOVERSION 0)

# Generate SVN_REVISION variable
execute_process(COMMAND svnversion -n ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE SVN_REVISION)

if (WIN32)
  add_definitions( -DWIN32 )

  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif (WIN32)

if(ENABLE_THREADED_GL)
  add_definitions( -DENABLE_THREADED_GL )
  set(THREADED_GL true)
else(ENABLE_THREADED_GL)
  set(THREADED_GL false)
endif(ENABLE_THREADED_GL)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
