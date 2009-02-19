project(libavogadro)
# Library versioning
# We probably want to adopt the libtool scheme once we hit 1.0.0
set(VERSION 0.9.1)
set(SOVERSION 0)

if (WIN32)
  add_definitions( -DWIN32 )
  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif (WIN32)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)

if(ENABLE_TESTS MATCHES ON)
  add_subdirectory(tests)
endif(ENABLE_TESTS MATCHES ON)
