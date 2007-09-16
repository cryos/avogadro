project(libavogadro)

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
