project(avogadro)
set(VERSION "0.9.0")

if (WIN32)
  add_definitions( -DWIN32 )
  # add definitions for OB in WIN32
  add_definitions( -DUSING_OBDLL -DUSING_DYNAMIC_LIBS )
endif (WIN32)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
