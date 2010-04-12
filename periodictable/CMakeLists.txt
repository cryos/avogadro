project(periodictable)
# Library versioning
# We probably want to adopt the libtool scheme once we hit 1.0.0
set(SOVERSION 0)

if (WIN32)
  add_definitions( -DWIN32 )
  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif (WIN32)

add_subdirectory(src)
